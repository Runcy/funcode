
#include <iostream>
#include "Logger.h"

int main()
{
	CLogger::InitLogger( E_LOG_INFO, "MyLog", 0);
	while(true)
	{
		LOGGER(E_LOG_INFO) << "listen port: ,forward info:" << forwardPort;
		LOGGER(E_LOG_FATAL) << "invalid port config file";
		LOGGER(E_LOG_DEBUG) << "removeItemEvent pTCPNetworkItemClient is null";
	}
	
	return 0;
}

