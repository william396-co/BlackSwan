#include "runningFlag.h"
#include <atomic>

std::atomic<bool> g_stop_flag_{};

bool isGameRunning()
{
	return false == g_stop_flag_.load(std::memory_order_acquire);
}

void disableGameRunning()
{
	g_stop_flag_.store(true, std::memory_order_release);
}

void enableGameRunning()
{
	g_stop_flag_.store(false, std::memory_order_release);
}