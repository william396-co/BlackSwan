#include "service.h"

#include <google/protobuf/stubs/common.h>
#include "share/log/log.h"

int main() 
{
	// Start Log function
	auto log = std::make_unique<CLog>("Client");

	ClientService service;
	if (!service.start()) {
		LOG_ERROR("start service failed");
		return 0;
	}
	service.run();
	service.stop();

	// elegant shutdown protobuf
	google::protobuf::ShutdownProtobufLibrary();

	return 0;
}
