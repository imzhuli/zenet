#include <zenet/ZEConnectionListener.h>
#include <zenet/ZENetworkManager.h>
#include <zenet/ZEConnection.h>

namespace ze
{

	void ZEConnectionListener::acceptEventCallback(struct evconnlistener *, evutil_socket_t clientSock, struct sockaddr * clientAddress, int socklen, void * ctx)
	{
		assert(socklen == sizeof(sockaddr_in)); // ipv4
		ZEConnectionListener * pConnectionListener = static_cast<ZEConnectionListener *>(ctx);
		if (ZEConnection * pConnection = pConnectionListener->makeConnection(std::move(clientSock), *(sockaddr_in*)clientAddress)) {
			auto pListener = pConnection->hConnectionEventListener;
			if (!pListener->onConnectionEstablished(pConnection)) {
				pConnection->close();
				pListener->onConnectionActiveClose(pConnection);
			}
		}
	}

	ZE_API bool ZEConnectionListener::init(const void * pParam)
	{
		assert(hNetworkManager);
		assert(oxBindPort);

		sockaddr_in bindAddr; // ipv4
		memset(&bindAddr, 0, sizeof(bindAddr));

		bindAddr.sin_family = AF_INET;
		bindAddr.sin_port = htons(oxBindPort);
		if (opBindIp) {
			claim(1 == evutil_inet_pton(AF_INET, opBindIp, &bindAddr.sin_addr));
		}
		_pConnListener = evconnlistener_new_bind(hNetworkManager->eventBase(), acceptEventCallback, this,
			LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
			(sockaddr*)&bindAddr, sizeof(bindAddr));

		return _pConnListener != nullptr;
	}

	ZE_API void ZEConnectionListener::clean()
	{
		evconnlistener_free(steal(_pConnListener));
	}

}
