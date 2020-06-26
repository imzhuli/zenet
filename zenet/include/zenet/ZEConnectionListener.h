#pragma once
#include "./__ENV__.h"
#include "./ZEConnection.h"
#include "./ZENetworkManager.h"

namespace ze
{

	class ZEConnectionListener
	: public IResource
	{
	public:
		tag::option<const char *>       opBindIp = "0.0.0.0";
		tag::option<uint16_t>           oxBindPort = 0;
		tag::handle<ZENetworkManager *> hNetworkManager = nullptr;

	private:
		struct ::evconnlistener  * _pConnListener = nullptr;

	public:
		ZE_API bool init(const void * pParam) override;
		ZE_API void clean() override;

		/**
		 * Function: makeConnection
		 * Purpose: create proper instance of ZEConnecion type, and initialize it.
		 * Note: !!!
		 *   . the caller does not initialize the returned reference of instance,
		 *   typically, the caller uses return value for logging only.
		 *   . this function is designed to take the ownership of clientSock,
		 *   so it's its duty to close socket propperly on any failure
		 */
		ZE_API virtual ZEConnection * makeConnection(evutil_socket_t && clientSock, const sockaddr_in & clientAddress) = 0;

	private:
		static void acceptEventCallback(struct evconnlistener *, evutil_socket_t clientSock, struct sockaddr * clientAddress, int socklen, void * ctx);
	};

}
