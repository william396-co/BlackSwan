#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <string>
#include <iostream>

#ifndef SPDLOG_USE_STD_FORMAT
#define SPDLOG_USE_STD_FORMAT
#endif

class CLog
{
public:
	// now allow copy and move
	CLog(CLog const&) = delete;
	CLog& operator=(CLog const&) = delete;
	CLog(CLog&&) = delete;
	CLog& operator=(CLog&&) = delete;

public:
	// constructor and destructor
	explicit CLog(std::string const& appName);
	~CLog();

private:
	void Init_Logger(std::string const&appName);
private:
	std::shared_ptr<spdlog::async_logger> m_logger;
};

using namespace spdlog;

// exception log(Critical) show [file_name, line, func]
#define SPDLOG_EX(level, ...) \
    spdlog::log(spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, level, fmt::format(__VA_ARGS__))

// normal log, just show function name
#define SPDLOG(level, ...) \
    spdlog::log(spdlog::source_loc{"",0,__FUNCTION__}, level,fmt::format(__VA_ARGS__))

#ifdef _DEBUG

#define LOG_TRACE(...) SPDLOG(spdlog::level::trace, __VA_ARGS__)
#define LOG_DEBUG(...) SPDLOG(spdlog::level::debug, __VA_ARGS__)

#else

#define LOG_TRACE(...)
#define LOG_DEBUG(...)

#endif

#define LOG_INFO(...)  SPDLOG(spdlog::level::info, __VA_ARGS__)
#define LOG_WARN(...)  SPDLOG(spdlog::level::warn, __VA_ARGS__)
#define LOG_ERROR(...) SPDLOG_EX(spdlog::level::err, __VA_ARGS__)
#define LOG_CRITICAL(...) SPDLOG_EX(spdlog::level::critical, __VA_ARGS__)