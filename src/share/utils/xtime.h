#pragma once

#include <chrono>

using namespace std::chrono;
using namespace std::chrono_literals;

// seconds
inline time_t time()
{
	return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}

// millisecond
inline time_t now()
{
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

// microsecond
inline time_t current()
{	
	return duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
}
