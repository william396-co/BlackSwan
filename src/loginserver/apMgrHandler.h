#pragma once

#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <array>

#include "share/utils/singleton.h"
#include "share/utils/xtime.h"

#include "authInfo.h"

constexpr auto WORK_THREAD_COUNT = 4;

class APMgrHandler : public Singleton<APMgrHandler>
{
	using ThreadArr = std::array<std::thread, WORK_THREAD_COUNT>;
	using MutexArr = std::array<std::mutex, WORK_THREAD_COUNT>;
	using MessageArr = std::array<APMsgList, WORK_THREAD_COUNT>;

	friend class Singleton<APMgrHandler>;
	APMgrHandler() = default;
public:
	~APMgrHandler() = default;

	bool start();
	void stop();
	void pushMsg(APMsg msg);

	inline bool isRunning()const { return running_.load(std::memory_order_acquire); }
	inline void startRunning() { running_.store(true, std::memory_order_release); }
	inline void stopRunning() { running_.store(false, std::memory_order_release); }

private:
	void run(int idx);
	void onProcApMsg(APMsg const& msg);
	void checkLoginTokent(APMsg const& msg);
private:
	ThreadArr work_arr_;
	MutexArr mtx_arr_;
	MessageArr message_arr_;
	uint32_t process_cnt_{};
	std::atomic_bool running_{};
};

#define g_apmgrhandler  APMgrHandler::InstancePtr()