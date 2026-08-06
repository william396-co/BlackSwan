#pragma once

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

#include <chrono>
#include <functional>

#include "servicePool.h"
#include "socket.h"
#include "wrapperService.h"

using boost::asio::ip::tcp;

using AsyncConnectCallback= std::function<void(SharedSocketPtr)>;
using AsyncConnectFailedCallback = std::function<void()>;

class AsioTcpConnector{
public:

	AsioTcpConnector(IoServicePoolPtr ioservicePoolPtr)
		: ioServicePoolPtr_{ ioservicePoolPtr }
	{
	}

	void asyncConnect(tcp::endpoint ep, std::chrono::nanoseconds timeout,
		AsyncConnectCallback callback,
		AsyncConnectFailedCallback failedCallback)
	{
		wrapperAsyncConnect(ioServicePoolPtr_->pickIoServiceThread(),
			{ ep }, timeout, callback, failedCallback);

	}

	void asyncConnect(IoServiceThreadPtr ioserviceThreadPtr,
		tcp::endpoint ep, std::chrono::nanoseconds timeout,
		AsyncConnectCallback callback,
		AsyncConnectFailedCallback failedCallback) 
	{
		wrapperAsyncConnect(ioserviceThreadPtr, { ep }, timeout, callback, failedCallback);
	}
private:
	void wrapperAsyncConnect(IoServiceThreadPtr ioserviceThreadPtr,
		std::vector<tcp::endpoint> eps, 
		std::chrono::nanoseconds timeout,
		AsyncConnectCallback callback,
		AsyncConnectFailedCallback failedCallback) 
	{

		auto sharedSocket = std::make_shared<SharedSocket>(tcp::socket(ioserviceThreadPtr->io_context()), ioserviceThreadPtr->io_context());

		auto timer = ioserviceThreadPtr->wrapperIoService().runAfter(
			timeout,[=]() {
				sharedSocket->socket().close();
				failedCallback();
			});

		boost::asio::async_connect(sharedSocket->socket(),
			eps, [=](boost::system::error_code const& error, tcp::endpoint) {
				timer->cancel();
				if (!error) {
					callback(sharedSocket);
				}
				else {
					failedCallback();
				}
			});
	}

private:
	IoServicePoolPtr ioServicePoolPtr_;
};