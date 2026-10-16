#include "crashinspector.h"

#include <string>
#include <DbgHelp.h>
#include <time.h>
#include <atomic>
#undef UNICODE
#include <windows.h>
#include "signal.h"
#include <spdlog/spdlog.h>
#include <algorithm>

extern char version[];

static ExceptionCallBack _exception_callback = nullptr;
static _invalid_parameter_handler _invalid_parameter_handler_old = nullptr;
//static PEXCEPTION_POINTERS ExceptionInfo


#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // must be 0x1000
	char* szName; // pointer to name (in user addr space)
	DWORD dwThreadID; // thread ID (-1=caller thread)
	DWORD dwFlags; // reserved for future use, must be zero
} THREADNAME_INFO;
#pragma pack(pop)

using std::string;
DumpMode _DumpMode = DumpMode(dump_mode_simple | dump_mode_full);

bool _Installed = false;
bool _AutoDumpException = false;
std::atomic<int> _ExceptionCount = 0;

static bool IsPathDelimiter(const std::string& path, int index) {

	if ((index >= 0) && (index < (int)path.size())) {
		char symbol = path[index];
		return (symbol == '\\') || (symbol == '/');;
	}
	return false;
}

static std::string ExcludePathDelimiter(const std::string& path) {
	std::string s = path;
	if (IsPathDelimiter(path, (int)path.size() - 1))
		s.resize(s.size() - 1);
	return s;

}

static bool ExistDirectory(const std::string& path) {

	WIN32_FIND_DATA findData;
	findData.dwFileAttributes = 0;
	HANDLE hFind = FindFirstFile(path.c_str(), &findData);
	if (hFind != INVALID_HANDLE_VALUE) {
		FindClose(hFind);
		return (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}
	return false;
}

static bool CreateDir(std::string& dir) {
	return CreateDirectory(dir.c_str(), nullptr) == TRUE;
}

static std::string ExtractFilePath(const std::string path)
{
	size_t posWinDelimiter = path.find_last_of('\\');
	size_t posLinuxDelimiter = path.find_last_of('/');
	size_t posDriverDelimiter = path.find_last_of(':');
	size_t pos = {};
	if (posWinDelimiter != string::npos) {
		pos = posWinDelimiter;
	}

	if (posLinuxDelimiter != string::npos) {
		pos = std::max(posLinuxDelimiter, pos);
	}

	if (posDriverDelimiter != string::npos) {
		pos = std::max(posDriverDelimiter, pos);
	}

	if (pos == -1)
		return path;
	else
		return path.substr(0, pos + 1);
}

static bool ForceDirectory(const std::string& path) {
	std::string dir = path;
	dir = ExcludePathDelimiter(dir);

	if (ExistDirectory(dir))
		return true;

	std::string parentPath = ExtractFilePath(dir);
	if ((dir.size() < 3) || (parentPath == dir))
		return CreateDir(dir);
	else
		return ForceDirectory(parentPath) && CreateDir(dir);

}

static string IncludePathDelimiter(const std::string& path) {
	if (!IsPathDelimiter(path, (int)path.size() - 1)) {
		return path + '\\';
	}
	return path;
}

static string ExpandFilename(const std::string& filename) {
	char buff[4096];
	std::string result;
	int count = GetCurrentDirectory(sizeof(buff), buff);
	if (count > 0)
		result = IncludePathDelimiter(buff) + filename;
	else
		result = filename;
	return result;
}

string GenDumpFileName(const string& path, time_t dumpTime, const string& fileSuffix = "mini")
{
	TCHAR buffer[MAX_PATH];
	string moduleName;
	int size = GetModuleFileName(0, buffer, MAX_PATH * 2 - 2);
	if (size > 0)
	{
		moduleName = buffer;
		auto lastPathDelimeterPos = moduleName.rfind("\\");
		if (lastPathDelimeterPos > 0)
			moduleName = moduleName.substr(lastPathDelimeterPos);
	}
	else
		moduleName = "unknown";

	tm tmLocal = {};
	errno_t errNo = localtime_s(&tmLocal, &dumpTime);

	sprintf_s(buffer, "%s\\%s_%04u%02u%02u_%02u%02u%02u_%u_%u_%s_%s.dmp.log",
		path.c_str(),
		moduleName.c_str(),
		tmLocal.tm_year + 1900,
		tmLocal.tm_mon + 1,
		tmLocal.tm_mday,
		tmLocal.tm_hour,
		tmLocal.tm_min,
		tmLocal.tm_sec,
		GetCurrentThreadId(),
		GetTickCount(),
		version,
		fileSuffix.c_str()
	);
	return buffer;
}

void WriteDumpFile(string filename, struct _EXCEPTION_POINTERS* ExceptionInfo, _MINIDUMP_TYPE dumpType)
{
	HANDLE hFile = ::CreateFile(
		filename.c_str(),
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (INVALID_HANDLE_VALUE != hFile)
	{
		PMINIDUMP_EXCEPTION_INFORMATION pInfo = nullptr;
		MINIDUMP_EXCEPTION_INFORMATION info;
		info.ThreadId = ::GetCurrentThreadId();
		info.ExceptionPointers = ExceptionInfo;
		info.ClientPointers = FALSE;
		pInfo = &info;

		::MiniDumpWriteDump(
			::GetCurrentProcess(),
			::GetCurrentProcessId(),
			hFile,
			dumpType,
			pInfo,
			NULL,
			NULL);
		::CloseHandle(hFile);
	}
}


void SaveExpceptionDump(const DumpExceptionInfo& info, _EXCEPTION_POINTERS* ExceptionInfo)
{
	spdlog::default_logger()->info("dump begin, exception count: {}", _ExceptionCount);
	spdlog::default_logger()->flush();

	// Dump文件默认在CrashDump目录
	string DumpPath = "CrashDumps";
	// 如果是Hangdump类型, 则更改目录为HangDump
	if (info.dumpReason == dump_reason_hang)
		DumpPath = "HangDumps";
	ForceDirectory(DumpPath);

	// 获取dump的模式
	DumpMode mode = info.dumpMode;
	time_t curTime = time(nullptr);

	if (mode & dump_mode_mini) {
		string simpleFilename = GenDumpFileName(DumpPath, curTime, "mini");
		_MINIDUMP_TYPE dumpType = (_MINIDUMP_TYPE)(MiniDumpWithIndirectlyReferencedMemory);
		WriteDumpFile(simpleFilename,
			ExceptionInfo,
			dumpType
		);
	};


	if (mode & dump_mode_simple) {
		string simpleFilename = GenDumpFileName(DumpPath, curTime, "simple");
		_MINIDUMP_TYPE dumpType = (_MINIDUMP_TYPE)(MiniDumpWithDataSegs /*| MiniDumpWithPrivateWriteCopyMemory*/ | MiniDumpWithPrivateReadWriteMemory /*| MiniDumpIgnoreInaccessibleMemory*/);
		WriteDumpFile(simpleFilename,
			ExceptionInfo,
			dumpType
		);
	};

	if (mode & dump_mode_full) {
		string fullFileName = GenDumpFileName(DumpPath, curTime, "full");
		_MINIDUMP_TYPE dumpType = (_MINIDUMP_TYPE)(MiniDumpWithIndirectlyReferencedMemory  /*| MiniDumpIgnoreInaccessibleMemory*/);
		WriteDumpFile(fullFileName,
			ExceptionInfo,
			dumpType
		);
	};
#ifdef _DEBUG
	exit(-1);
#endif
}


LONG WINAPI UnhandledExceptionHandler(_In_ struct _EXCEPTION_POINTERS* ExceptionInfo)
{
	_ExceptionCount++;
	// 构建Info数据, 引导SaveExceptionDump正确的生成dump文件
	DumpExceptionInfo info;
	info.dumpMode = _DumpMode;
	info.dumpReason = dump_reason_catched;
	SaveExpceptionDump(info, ExceptionInfo);
	return EXCEPTION_EXECUTE_HANDLER;
}

void RaiseDumpException(DumpMode dumpMode,const char* message)
{
	// 主动创建一个Exception, 原因是由于主线程长期挂起无效应导致的
	DumpExceptionInfo exceptionInfo;
	exceptionInfo.dumpMode = dumpMode;
	exceptionInfo.dumpReason = dump_reason_force;
	exceptionInfo.exceptionMessage = message;
	DWORD paramCount = sizeof(exceptionInfo) / sizeof(void*);
	RaiseException(DumpExceptionCode, 0, paramCount, (ULONG_PTR*)&exceptionInfo);
}

extern void RaiseHangDumpException(DumpMode dumpMode)
{
	DumpExceptionInfo exceptionInfo;
	exceptionInfo.dumpMode = dumpMode;
	exceptionInfo.dumpReason = dump_reason_hang;
	exceptionInfo.exceptionMessage = {};
	DWORD paramCount = sizeof(exceptionInfo) / sizeof(void*);
	RaiseException(DumpExceptionCode, 0, paramCount, (ULONG_PTR*)&exceptionInfo);
}

void SetThreadNameByThreadID(DWORD thread_id, char* thread_name)
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.dwThreadID = thread_id;
	info.dwFlags = 0;
	info.szName = thread_name;
	DWORD paramCount = sizeof(info) / sizeof(void*);
	__try
	{
		RaiseException(SetThreadNameExcetpionCode, 0, paramCount, (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
	{

	}
}

extern void SetThreadName(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	char thread_name[8192];
	vsnprintf(thread_name, sizeof(thread_name), format, ap);
	va_end(ap);
	SetThreadNameByThreadID(-1, thread_name);
}

void OnDumpExceptionCallback(DumpExceptionInfo* info) {
	if (_exception_callback) {
		_exception_callback(info);
	}
}

LONG CALLBACK VectoredHandler(PEXCEPTION_POINTERS ExceptionInfo) {

	// 过滤 c++ , 线程名称, 和outputDebugString带来的异常
	if (ExceptionInfo) {
		DWORD exceptionCode = ExceptionInfo->ExceptionRecord->ExceptionCode;


		/* 忽略throw和设置线程名称的2个异常代码 */
		bool ignoreException = (exceptionCode == CPPExceptionCode)
			|| (exceptionCode == SetThreadNameExcetpionCode)
			|| (exceptionCode == DBG_PRINTEXCEPTION_C);


		if (ignoreException)
			return EXCEPTION_EXECUTE_HANDLER;

		_ExceptionCount++;

		// 新增强制dump
		if (exceptionCode == DumpExceptionCode) {
			DumpExceptionInfo* info = (DumpExceptionInfo*)ExceptionInfo->ExceptionRecord->ExceptionInformation;
			SaveExpceptionDump(*info, ExceptionInfo);
			// 回调异常函数
			OnDumpExceptionCallback(info);
		}
		else {
			// 构建Info数据, 引导SaveExceptionDump正确的生成dump文件
			DumpExceptionInfo info;
#ifdef _DEBUG
			info.dumpMode = dump_mode_full;
#else
			info.dumpMode = dump_mode_mini;
#endif
			info.dumpReason = dump_reason_catched;
			SaveExpceptionDump(info, ExceptionInfo);
		}
	}
	return EXCEPTION_EXECUTE_HANDLER;
}



int CrashInspector::GetExceptionCount()
{
	return _ExceptionCount;
}

void CrashInspector::ResetExceptionCount()
{
	_ExceptionCount = 0;
}


void __cdecl _invalid_parameter_exception_handler(
	wchar_t const* expression,
	wchar_t const* function_name,
	wchar_t const* file_name,
	unsigned int line_number,
	uintptr_t reserved
)
{
	RaiseDumpException(dump_mode_full, "halt exception");
}

void __cdecl _purecall_exception_handler(void)
{
	RaiseDumpException(dump_mode_full, "pure call halt exception");
}

void __CRTDECL _terminate_handler()
{
	RaiseDumpException(dump_mode_full, "terminated");
}

void __CRTDECL _unexpected_handler()
{
	RaiseDumpException(dump_mode_full, "unexpected");
}

void _OnSigAbort(int s)
{
	printf("s %u", s);
	RaiseDumpException(dump_mode_full, "terminated");
}


void CrashInspector::SetExceptionCallback(ExceptionCallBack callback)
{
	_exception_callback = callback;
}

void CrashInspector::Install()
{
	if (!_Installed) {
		_Installed = true;
		SetUnhandledExceptionFilter(UnhandledExceptionHandler);
	}
}

void CrashInspector::Uninstall()
{
	if (_Installed) {
		_Installed = false;
		SetUnhandledExceptionFilter(nullptr);
	}
}

void CrashInspector::SetDumpMode(DumpMode dmpMode)
{
	_DumpMode = dmpMode;
}

void CrashInspector::EnableAutoDump()
{
	if (!_AutoDumpException)
	{
		_AutoDumpException = true;
		AddVectoredExceptionHandler(1, VectoredHandler);
		_invalid_parameter_handler_old = _get_invalid_parameter_handler();
		_set_invalid_parameter_handler(_invalid_parameter_exception_handler);
		//_set_purecall_handler(_purecall_exception_handler);	
		//set_unexpected(_unexpected_handler);
		set_terminate(_terminate_handler);
		signal(SIGABRT, _OnSigAbort);
	}
}

