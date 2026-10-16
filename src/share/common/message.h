#pragma once

#include <unordered_map>
#include <functional>
#include <utility>
#include <google/protobuf/message_lite.h>
#include "utils/singleton.h"

template<typename MessageHandler>
struct MessageData
{
	MessageData() = default;
	MessageData(::google::protobuf::MessageLite* pMsg, MessageHandler handler)
		:pMessage_{ pMsg }, handler_{ std::move(handler) }
	{
	}

	MessageData(MessageData&& other)noexcept
		:pMessage_{ std::move(other.pMessage_) }
		, handler_{ std::move(other.handler_) } {
		other.pMessage_ = {};
		other.handler_ = {};
	}
	MessageData& operator=(MessageData&& other)noexcept
	{
		if (this == &other) return *this;
		pMessage_ = std::move(other.pMessage_);
		handler_ = std::move(other.handler_);
		other.pMessage_ = {};
		other.handler_ = {};
		return *this;
	}
	~MessageData() {
		delete pMessage_;
		pMessage_ = {};
	}
	::google::protobuf::MessageLite* pMessage_{};
	MessageHandler handler_{};
};

#if 0
template<typename MsgID, typename MessageHandler>
using MessageMap = std::unordered_map<MsgID, MessageData<MessageHandler>>;
#endif
