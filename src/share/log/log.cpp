#include "log.h"

#include "../utils/xtime.h"

// Compatibility helper required by the bundled daily rotating sink.
namespace spdlog::sinks {
inline std::time_t GetCurrSecond()
{
	return xtime::time();
}
}


#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/async.h>
#include <spdlog/spdlog.h>
//#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/daily_rotating_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <ctime>
#include <iomanip>
#include <chrono>
#include <sstream>

CLog::CLog(std::string const&appName)
{
	Init_Logger(appName);
}

CLog::~CLog()
{
	spdlog::default_logger()->flush();
}
std::string time_to_string(time_t tp)
{
	// transform local time
	std::tm tm = *std::localtime(&tp); // Windows use localtime_s
	std::ostringstream oss;// formating output
	oss << std::put_time(&tm, "%Y-%m-%d_%H_%M_%S");
	return oss.str();
}

void CLog::Init_Logger(std::string const&appName)
{
	// init thread pool (queue size 8192,work thread 1)
	spdlog::init_thread_pool(8192, 1);

	//auto dateTime = time_to_string(GetCurrSecond());
	//std::string base_name = fmt::format("./logs/{}_{}", appName, dateTime);

	constexpr auto single_file_size = 1024 * 1024 * 20;// 20MB
	constexpr auto max_file_num = 1000;
	// define file sizek（20MB x 1000 files）
	auto file_sink = std::make_shared<spdlog::sinks::daily_rotating_sink_mt>(
		appName, single_file_size, max_file_num
	);
	file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s %# %!] | %v");

	#define SHOW_CONSOLE
#ifdef SHOW_CONSOLE
	auto console_sink = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
	console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
#endif

	// create async logger
	auto m_logger = std::make_shared<spdlog::async_logger>(
		"async_logger",
		spdlog::sinks_init_list{ file_sink
#ifdef SHOW_CONSOLE
		, console_sink
#endif 
		},
		spdlog::thread_pool(),
		spdlog::async_overflow_policy::block
	);

#ifdef _DEBUG
	m_logger->set_level(level::debug);	// set log level
#else
	m_logger->set_level(level::info);   // set log level
#endif

	m_logger->flush_on(level::err);     // imidately flush log level(Err)
	spdlog::flush_every(std::chrono::seconds{ 1 });
	spdlog::set_default_logger(m_logger);

	spdlog::set_error_handler([](const std::string& msg) {
		std::cerr << "my err handler: " << msg << std::endl;
		});
}
