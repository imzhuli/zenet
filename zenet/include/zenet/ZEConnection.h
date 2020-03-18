#pragma once
#include <ze/convention.h>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/util.h>
#include <cassert>

namespace ze
{

	class ZEConnection;
	class ZEConnectionListener;
	class ZENetworkManager;

	class ZEConnectionEventListener : Interface
	{
	public:
		virtual bool onConnectionEstablished(ZEConnection * pConnection) { return true; }
		virtual bool onConnectionOutputReady(ZEConnection * pConnection) { return true; }
		virtual bool onConnectionInputReady(ZEConnection * pConnection) ;

		virtual void onConnectionActiveClose(ZEConnection * pConnection) {};
		virtual void onConnectionPassiveClose(ZEConnection * pConnection) {};
	};

	class ZEConnection
	: public IResource
	{
		friend class ZEConnectionListener;
	public:
		bool init(const void * pParam) override;
		void clean() override;

	public:
		tag::option<const char *>                 opRemoteHost = nullptr;
		tag::option<uint_fast16_t>                oxRemotePort = 0;
		tag::handle<ZEConnectionEventListener *>  hConnectionEventListener = nullptr;
		tag::handle<Variable>                     hConnectionEventListenerContext {};
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

		static_assert(std::has_unique_object_representations_v<sockaddr_in>);
		static_assert(std::has_unique_object_representations_v<sockaddr_in6>);
		static_assert(std::is_same_v<decltype(sockaddr_in::sin_family), decltype(sockaddr_in6::sin6_family)>);
		static_assert(offsetof(sockaddr_in, sin_family) == offsetof(sockaddr_in6, sin6_family));
		static_assert(sizeof(sockaddr_in) <= sizeof(sockaddr_in6));
	public:
		ZE_FORCE_INLINE bool isReady() const { return _xState != UNKNOWN; }
		ZE_FORCE_INLINE bool isOpen() const { return _xState >= OPEN && _xState < CLOSED; }
		ZE_FORCE_INLINE bool isClosed() const { assert(_xState); return _xState == CLOSED; }

		ZE_FORCE_INLINE void resetState() { assert(!isOpen()); _xState = UNKNOWN; }
		bool read(void * buffer, tag::inout<size_t> length);
		bool write(const void * data, size_t length);
		bool write(const ArrayView<DataView> & writeList);
		bool write(const DataView & dv) { return write(dv.data(), dv.size()); }

	private:
		static void event_cb(struct bufferevent *bev, short what, void *ctx);
		static void write_cb(struct bufferevent *bev, void *ctx);
		static void read_cb(struct bufferevent *bev, void *ctx);
		void close();

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
