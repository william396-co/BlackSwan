#pragma once 

#include <string>
#include <memory>
#include <thread>
#include <chrono>

#include <boost/asio.hpp>
#include <boost/asio/corotinue.hpp>


#include "chatSession.h"

class ChatConnector 
{
public:	
	explicit ChatConnector()

private:
	ChatSessionPtr session_ptr_;
	boost::asio::io_context& io_;
};