#pragma once
#include "./__ENV__.h"

namespace ze
{

	class ZEConnection;
	class ZEConnectionListener;
	class ZENetworkManager;

	class ZEConnectionEventListener : Interface
	{
	public:
		ZE_API virtual ZE_FORCE_INLINE bool onConnectionEstablished(ZEConnection * pConnection) { return true; }
		ZE_API virtual ZE_FORCE_INLINE bool onConnectionOutputReady(ZEConnection * pConnection) { return true; }
		ZE_API virtual bool onConnectionInputReady(ZEConnection * pConnection) ;

		ZE_API virtual ZE_FORCE_INLINE void onConnectionActiveClose(ZEConnection * pConnection) {};
		ZE_API virtual ZE_FORCE_INLINE void onConnectionPassiveClose(ZEConnection * pConnection) {};
	};

	class ZEConnection
	: public IResource
	{
		friend class ZEConnectionListener;
	public:
		ZE_API bool init(const void * pParam) override;
		ZE_API void clean() override;

	public:
		tag::option<const char *>                 opRemoteHost = nullptr;
		tag::option<uint_fast16_t>                oxRemotePort = 0;
		tag::handle<ZEConnectionEventListener *>  hConnectionEventListener = nullptr;
		tag::handle<Variant>                      hConnectionEventListenerContext {};
		tag::handle<ZENetworkManager*>            hNetworkManager = nullptr;

		union {
			sockaddr_in                       cRemoteAddress;
			sockaddr_in6                      cRemoteAddress_6 {};
		};
		union {
			sockaddr_in                       cLocalAddress;
			sockaddr_in6                      cLocalAddress_6 {};
		};
		tag::config<evutil_socket_t> cAcceptedSocket = -1; /* EVUTIL_INVALID_SOCKET is prefered, but it is not defined on all platforms */

	public:
		ZE_FORCE_INLINE bool isReady() const { return _xState != UNKNOWN; }
		ZE_FORCE_INLINE bool isOpen() const { return _xState >= OPEN && _xState < CLOSED; }
		ZE_FORCE_INLINE bool isClosed() const { assert(_xState); return _xState == CLOSED; }

		ZE_FORCE_INLINE void resetState() { assert(!isOpen()); _xState = UNKNOWN; }
		ZE_API bool read(void * buffer, tag::inout<size_t> length);
		ZE_API bool write(const void * data, size_t length);
		ZE_API bool write(const ArrayView<DataView> & writeList);
		ZE_API bool write(const DataView & dv) { return write(dv.data(), dv.size()); }

	private:
		static void event_cb(struct bufferevent *bev, short what, void *ctx);
		static void write_cb(struct bufferevent *bev, void *ctx);
		static void read_cb(struct bufferevent *bev, void *ctx);
		ZE_API void close();

	private:
		enum State {
			UNKNOWN = 0,
			OPEN,
			CONNECTING,
			CONNECTED,
			CLOSED,
		};

		evutil_socket_t _xSocket = -1;
		bufferevent * _pBufferEvent = nullptr;
		State _xState = UNKNOWN;
	};

}
