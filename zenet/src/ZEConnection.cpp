#include <zenet/ZEConnection.h>
#include <zenet/ZENetworkManager.h>
#include <ze/ext/string.h>
#include <event2/util.h>
#include <event2/buffer.h>

namespace ze
{

	static evutil_socket_t createSocket()
	{
		evutil_socket_t sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (-1 == sock) {
			return -1;
		}
		evutil_make_socket_nonblocking(sock);
		evutil_make_listen_socket_reuseable(sock);
		evutil_make_listen_socket_reuseable_port(sock);
		/*
		uint32_t on = 1;
		setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on));
		*/
		return sock;
	}

	ZE_API bool ZEConnectionEventListener::onConnectionInputReady(ZEConnection * pConnection)
	{
		ubyte data[1024];
		while(pConnection->read(data, xref(sizeof data)))
		{}
		return true;
	}

	void ZEConnection::event_cb(struct bufferevent *bev, short what, void *ctx)
	{
		auto connection = static_cast<ZEConnection *>(ctx);
		if(what & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
			connection->close();
			connection->hConnectionEventListener->onConnectionPassiveClose(connection);
			return;
		}
		if (what &  BEV_EVENT_CONNECTED) {
			connection->_xState = CONNECTED;
			auto pListener = connection->hConnectionEventListener;
			if (!pListener->onConnectionEstablished(connection)) {
				connection->close();
				pListener->onConnectionActiveClose(connection);
			}
			return ;
		}
	}

	void ZEConnection::write_cb(struct bufferevent *bev, void *ctx)
	{
		auto connection = static_cast<ZEConnection *>(ctx);
		auto pListener = connection->hConnectionEventListener;
		if (!pListener->onConnectionOutputReady(connection)) {
			connection->close();
			pListener->onConnectionActiveClose(connection);
		}
	}

	void ZEConnection::read_cb(struct bufferevent *bev, void *ctx)
	{
		auto connection = static_cast<ZEConnection *>(ctx);
		auto pListener = connection->hConnectionEventListener;
		if (!pListener->onConnectionInputReady(connection)) {
			connection->close();
			pListener->onConnectionActiveClose(connection);
		}
	}

	ZE_API bool ZEConnection::init(const void * pParam)
	{
		assert(hConnectionEventListener);
		assert(hNetworkManager);
		assert(_xState == UNKNOWN && "invalid connection state, if you want to reuse a connection, call resetState() before init(const void * pParam)");
		if (cAcceptedSocket == -1) {
			assert(cRemoteAddress.sin_family != PF_UNSPEC || (opRemoteHost && oxRemotePort));
			if (-1 == (_xSocket = createSocket())) {
				_xState = UNKNOWN;
				return false;
			}
			_xState = OPEN;
		}
		else {
			_xSocket = cAcceptedSocket;
			_xState = CONNECTED;
		}
		if (_xSocket == -1) {
			return false;
		}
		_pBufferEvent = bufferevent_socket_new(
			hNetworkManager->eventBase(),
			_xSocket,
			0
		);
		if (!_pBufferEvent) {
			evutil_closesocket(steal(_xSocket, -1));
			_xState = UNKNOWN;
			return false;
		}
		if (_xState == OPEN) {
			if (((cLocalAddress.sin_addr.s_addr || cLocalAddress.sin_port) && -1 == bind(_xSocket, (sockaddr*)&cLocalAddress, sizeof(cLocalAddress)))
			|| (cRemoteAddress.sin_family == AF_INET && 0 != bufferevent_socket_connect(_pBufferEvent, (sockaddr*)&cRemoteAddress, sizeof(cRemoteAddress)))
			|| (cRemoteAddress.sin_family == AF_INET6 && 0 != bufferevent_socket_connect(_pBufferEvent, (sockaddr*)&cRemoteAddress_6, sizeof(cRemoteAddress_6)))
			|| (opRemoteHost && 0 != bufferevent_socket_connect_hostname(_pBufferEvent, hNetworkManager->dnsBase(), AF_UNSPEC, opRemoteHost, oxRemotePort))) {
				bufferevent_free(steal(_pBufferEvent));
				evutil_closesocket(steal(_xSocket, -1));
				_xState = UNKNOWN;
				return false;
			}
			_xState = CONNECTING;
		}
		bufferevent_setcb(_pBufferEvent, read_cb, write_cb, event_cb, this);
		bufferevent_enable(_pBufferEvent, EV_READ | EV_ET);
		return true;
	}

	ZE_API void ZEConnection::clean()
	{
		if (isOpen()) {
			close();
		}
		_xState = UNKNOWN;
	}

	// callbacks:
	ZE_API bool ZEConnection::write(const void * data, size_t length)
	{
		return 0 == bufferevent_write(_pBufferEvent, data, length);
	}

	ZE_API bool ZEConnection::write(const ArrayView<DataView> & writeList)
	{
		static_assert(std::is_standard_layout_v<DataView> && std::is_standard_layout_v<evbuffer_iovec>);
		static_assert(std::alignment_of_v<DataView> == std::alignment_of_v<evbuffer_iovec>);
		static_assert(sizeof(DataView) == sizeof(evbuffer_iovec));

		static_assert(std::is_same_v<std::remove_cv_t<std::remove_pointer_t<const void * /* decltype(DataView::pData) */>>,
		              std::remove_cv_t<std::remove_pointer_t<decltype(evbuffer_iovec::iov_base)>>>);
		static_assert(std::is_same_v<std::remove_cv_t<size_t /* decltype(DataView::xSize) */>,
		              std::remove_cv_t<decltype(evbuffer_iovec::iov_len)>>);
		static_assert(OFFSET_OF_PDATA_IN_DATAVIEW() == offsetof(evbuffer_iovec, iov_base));
		static_assert(OFFSET_OF_XSIZE_IN_DATAVIEW() == offsetof(evbuffer_iovec, iov_len));

		return evbuffer_add_iovec(bufferevent_get_output(_pBufferEvent), reinterpret_cast<evbuffer_iovec *>(writeList.begin()), (int)writeList.size());
	}

	ZE_API bool ZEConnection::read(void * buffer, tag::inout<size_t> length)
	{
		return (length = bufferevent_read(_pBufferEvent, buffer, length)) > 0;
	}

	ZE_API void ZEConnection::close()
	{
		assert(isOpen());
		bufferevent_free(steal(_pBufferEvent));
		evutil_closesocket(steal(_xSocket, -1));
		_xState = CLOSED;
	}

}
