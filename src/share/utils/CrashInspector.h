#pragma once
#include <windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "dbghelp.lib")

#include <string>

enum DumpMode {
	dump_mode_mini = 1,
	dump_mode_simple = 2,
	dump_mode_full = 4
};

enum DumpReason {
	// 强制产生的dump
	dump_reason_force,
	// 检测到主线程挂起时产生的dump
	dump_reason_hang,
	// 由系统捕捉到异常
	dump_reason_catched
};

// 自定义DumpExceptionInfo, 用于产生异常Dump文件
struct DumpExceptionInfo
{
	DumpMode dumpMode;
	DumpReason dumpReason;  // dump 产生的原因
	std::string  exceptionMessage;
};

// 回调方法函数的声明
typedef void(*ExceptionCallBack)(DumpExceptionInfo*);

const DWORD DumpExceptionCode = 0xE06D7364;
const DWORD CPPExceptionCode = 0xE06D7363;
const DWORD SetThreadNameExcetpionCode = 0x406D1388;


class CrashInspector
{
public:
	static void Install();
	static void Uninstall();
	static void SetDumpMode(DumpMode dmpMode);
	static void EnableAutoDump();
	static int GetExceptionCount();
	static void ResetExceptionCount();
	static void SetExceptionCallback(ExceptionCallBack callback);
};


extern void RaiseDumpException(DumpMode dumpMode,const char* message);
extern void RaiseHangDumpException(DumpMode dumpMode);

extern void SetThreadNameByThreadID(DWORD thread_id,const char* thread_name);
extern void SetThreadName(const char* fmt, ...);