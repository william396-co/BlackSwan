#include "apMgrHandler.h"
#include "share/log/log.h"
#include "utils/runningFlag.h"
#include "config.h"
#include "constdefs.h"
#include "apMgr.h"
#include "service.h"

#include <chrono>

void APMgrHandler::pushMsg(APMsg msg)
{
	auto idx = (process_cnt_++) % work_arr_.size();
	std::lock_guard lk(mtx_arr_[idx]);
	message_arr_[idx].emplace_back(std::move(msg));
}


bool APMgrHandler::start() 
{
	if (!isRunning()) {
		startRunning();
		running_.store(true, std::memory_order_release);
		for(size_t i =0; i != work_arr_.size();++i){
			work_arr_[i] = std::thread(&APMgrHandler::run, this, i);
		}
		return true;
	}
	return false;
}

void APMgrHandler::stop() 
{
	stopRunning();

	for (size_t i = 0; i != message_arr_.size();++i) {
		std::lock_guard lk(mtx_arr_[i]);
		message_arr_[i].clear();
	}

	for (auto& it : work_arr_) {
		if (it.joinable()) {
			it.join();
		}
	}
}

void APMgrHandler::run(int idx) 
{
	// sub working-thread
	while (isGameRunning() && isRunning())
	{
		APMsgList cur_message_list;
		{
			std::lock_guard lck(mtx_arr_[idx]);
			cur_message_list.swap(message_arr_[idx]);
		}
		for (auto const& msg : cur_message_list) {
			onProcApMsg(msg);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds{ 2 });
	}
}

void APMgrHandler::onProcApMsg(APMsg const& msg)
{
	if (!msg.getAuth()) {
		LOG_ERROR("process PT msg without authInfo, operation:{} result:{}", msg.getOperation(), msg.getResult());
		return;
	}

	switch (msg.getOperation()) {
	case OPERATION::regist:
		break;
	case OPERATION::authen_ap:
		checkLoginTokent(msg);
		break;
	default:
		LOG_WARN("Unkown Authen operation Type:{}", msg.getOperation());
		break;
	}
}

void APMgrHandler::checkLoginTokent(APMsg const& msg)
{
	if (g_Config->getLoginUrl().empty()) {
		LOG_INFO("Process AT Msg LoginUrl is empty");
		return;
	}

	auto pAuth = msg.getAuth();
	auto const& strAuthenID = msg.getAuth()->getVal(gl_csAuthenID);
	auto const& strToken = msg.getAuth()->getVal(gl_csReservePwd);
	auto const& apType = msg.getAuth()->getAPType();
	auto const& reserve = msg.getAuth()->getReserve();
	auto const& areaGroup = msg.getAuth()->getAreaGroup();
	auto const& strIP = msg.getAuth()->getVal(gl_csClientIP);
	auto const& strInviteCode = msg.getAuth()->getVal(gl_csInviteCode);

	char szWebCheck[2048];
	snprintf(szWebCheck, sizeof(szWebCheck) - 1, "%s?sdk_id=%d&account=%s&token=%s&serverid=%u&invite=%s&reserve=%u&client_ip=%s", g_Config->getLoginUrl().c_str(), apType, strAuthenID.c_str(), strToken.c_str(), areaGroup, strInviteCode.c_str(), reserve, strIP.c_str());

	LOG_DEBUG("send [{}]", szWebCheck);
		
	// CURL
#if 0
	CURL* curl = curl_easy_init();
	if (curl)
	{
		auto now = GetCurrMilli();

		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);        //屏蔽其它信号
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		//curl_easy_setopt( curl, CURLOPT_VERBOSE, 1 ); //调试debug
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0)");
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);//5秒超时
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);//5秒超时
		curl_easy_setopt(curl, CURLOPT_URL, szWebCheck);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		string strResult;
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &strResult);
		CURLcode res = curl_easy_perform(curl);
		if (res == CURLE_RECV_ERROR || res == CURLE_SEND_ERROR)
		{
			LOG_INFO("[{}] http Check Token recv error", res);
			res = curl_easy_perform(curl);
		}
		if (res == CURLE_OPERATION_TIMEDOUT)
		{
			res = curl_easy_perform(curl);
		}

		LOG_INFO("web verify cost: {}ms", (GetCurrMilli() - now));

		if (res != CURLE_OK)
		{
			LOG_INFO("[{}] http Check Token false[{}]", res, szWebCheck);
			gpAPMgr->OperateResult(stMsg.p_session, stMsg.operation, time_out, time_out);
			curl_easy_cleanup(curl);
			return;
		}
		curl_easy_cleanup(curl);

		LOG_DEBUG(" AP result [{}]", strResult.c_str());

		//判断是不是合法的json包
		Json::Value valueCode, values;
		Json::Reader reader;
		if (!reader.parse(strResult, values, false))
		{
			g_apmgr->OperateResult(stMsg.p_session, stMsg.operation, failed, failed);
			LOG_DEBUG(" AP result error[{}]", strResult.c_str());
			return;
		}

		if (!values.isObject())
		{
			g_apmgr->OperateResult(stMsg.p_session, stMsg.operation, failed, failed);
			LOG_DEBUG(" AP result error[{}]", strResult.c_str());
			return;
		}

		valueCode = values["code"];
		if (valueCode.empty())
		{
			g_apmgr->OperateResult(stMsg.p_session, stMsg.operation, failed, failed);
			LOG_DEBUG(" AP result error[{}]", strResult.c_str());
			return;
		}

		if (valueCode.asInt() == 0)
		{
			LOG_DEBUG(" AP result success");

			Json::Value valueData;
			valueData = values["data"];
			if (!valueData.empty())
			{
				stMsg.p_session->PutValue(gl_csData, valueData.toStyledString().c_str());
			}

			g_apmgr->OperateResult(stMsg.p_session, stMsg.operation, success);
		}
		else if (valueCode.asInt() == 3)
		{
			LOG_DEBUG(" AP result success");
			g_apmgr->OperateResult(stMsg.p_session, stMsg.operation, not_open, 3);
		}
		else
		{
			LOG_DEBUG("AP result false {}", valueCode.asInt());
			g_apmgr->OperateResult(stMsg.p_session, stMsg.operation, miss_pwd, valueCode.asInt());
		}
	}
	else
	{
		g_apmgr->OperateResult(stMsg.p_session, stMsg.operation, sys_error, sys_error);
	}
#endif
}
