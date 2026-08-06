#pragma once

#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <deque>
#include <string>
#include <functional>
#include <cassert>

#include <boost/asio.hpp>

#include "socket.h"
#include "ox_buffer.h"

using boost::asio::ip::tcp;

class AsioTcpSession : public std::enable_shared_from_this<AsioTcpSession> {
	using DataCB = std::function<size_t(const char*, size_t)>;
public:

	AsioTcpSession(SharedSocketPtr socketptr, size_t maxRecvBufSize, DataCB cb)
		:maxRecvBufSize_{ maxRecvBufSize },
		socket_{socketptr },
		dataCb_{ cb }
	{

		socket_->socket().non_blocking();
		boost::asio::ip::tcp::no_delay option(true);
		socket_->socket().set_option(option);
		growRecvBuffer();

		//startRecv();	
	}
	void send(std::shared_ptr<std::string> msg) {
		{
			std::lock_guard lk(sendMtx_);
			pendingSendMsg.push_back({ 0,std::move(msg) });
		}
		trySend();
	}
	void send(std::string msg) {
		send(std::make_shared<std::string>(std::move(msg)));
	}
	const SharedSocketPtr& socket()const {
		return socket_;
	}
	void startRecv() {
		std::call_once(recvInitOnceFlag_, [this]() {
			doRecv();
			});
	}
private:
	void doRecv() {

		socket_->socket().async_read_some(
			boost::asio::buffer(ox_buffer_getwriteptr(recvBuffers_.get()),
				ox_buffer_getwritevalidcount(recvBuffers_.get())),
			[self = shared_from_this(),this](boost::system::error_code const&error,size_t byteTransferred) {
				onRecvCompleted(error, byteTransferred);
			});
	}
	void onRecvCompleted(boost::system::error_code const& error, size_t bytesTransferred) {

		if (error)return;

		ox_buffer_addwritepos(recvBuffers_.get(), bytesTransferred);
		if (ox_buffer_getreadvalidcount(recvBuffers_.get()) == ox_buffer_getsize(recvBuffers_.get())) {
			growRecvBuffer();
		}

		if (dataCb_) {

			auto const proclen = dataCb_(ox_buffer_getreader(recvBuffers_.get()),
				ox_buffer_getreadvalidcount(recvBuffers_.get()));
				assert(proclen <= ox_buffer_getreadvalidcount(recvBuffers_.get()) );
			if (proclen <= ox_buffer_getreadvalidcount(recvBuffers_.get())) {
				ox_buffer_addreadpos(recvBuffers_.get(), proclen);
			}

		}
		if (ox_buffer_getwritevalidcount(recvBuffers_.get()) == 0
			|| ox_buffer_getreadvalidcount(recvBuffers_.get()) == 0) {
			ox_buffer_adjustto_head(recvBuffers_.get());
		}

		doRecv();
	}
	void trySend() {
		std::lock_guard lk(sendMtx_);
		if (bSending_ || pendingSendMsg.empty()) {
			return;
		}

		buffers.resize(pendingSendMsg.size());
		for (size_t i = 0; i != pendingSendMsg.size();++i) {
			auto& msg = pendingSendMsg[i];
			buffers[i] = boost::asio::const_buffer(msg.msg->c_str() + msg.sendPos,
				msg.msg->size() - msg.sendPos);
		}

		socket_->socket().async_send(
			buffers,
			[self = shared_from_this(), this](boost::system::error_code const& error, size_t byteTransferred) {
				onSendCompleted(error, byteTransferred);
			}
		);
		bSending_ = true;
	}
	void onSendCompleted(boost::system::error_code const& error, size_t byteTransferred) {

		{
			std::lock_guard lk(sendMtx_);
			bSending_ = false;
			if (error) {
				return;
			}
			adjustSendBuffer(byteTransferred);
		}
		trySend();
	}
	
	void adjustSendBuffer(size_t bytesTransferred) {
		while (bytesTransferred) {

			auto& frontMsg = pendingSendMsg.front();
			const auto len = std::min<size_t>(bytesTransferred, frontMsg.msg->size() - frontMsg.sendPos);
			frontMsg.sendPos += len;
			bytesTransferred -= len;
			if (frontMsg.sendPos == frontMsg.msg->size()) {
				pendingSendMsg.pop_front();
			}
		}
	}
	void growRecvBuffer() {
		
		if (!recvBuffers_) {
			recvBuffers_.reset(ox_buffer_new(std::min<size_t>(16 * 1024, maxRecvBufSize_)));
		}
		else {
			const auto newSize = ox_buffer_getsize(recvBuffers_.get()) + 1024;
			if (newSize > maxRecvBufSize_) {
				return;
			}

			std::unique_ptr<struct buffer_s, BufferDeleter> newBuffer(ox_buffer_new(newSize));
			ox_buffer_write(newBuffer.get(), ox_buffer_getreadptr(recvBuffers_.get()),
				ox_buffer_getreadvalidcount(recvBuffers_.get()));
			recvBuffers_ = std::move(newBuffer);
		}
	}

private:
	const size_t maxRecvBufSize_{};
	const SharedSocketPtr socket_{};

	bool bSending_{};
	std::mutex sendMtx_;

	struct PendingMsg {
		size_t sendPos;
		std::shared_ptr<std::string> msg;
	};
	std::deque<PendingMsg> pendingSendMsg;
	std::vector<boost::asio::const_buffer> buffers;

	std::once_flag recvInitOnceFlag_;
	DataCB dataCb_;

	struct BufferDeleter {
		void operator()(struct buffer_s* ptr)const {
			ox_buffer_delete(ptr);
		}
	};
	std::unique_ptr<struct buffer_s, BufferDeleter> recvBuffers_;
};

using AsioTcpSessionPtr = std::shared_ptr<AsioTcpSession>;