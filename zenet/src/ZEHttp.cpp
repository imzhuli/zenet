#include <zenet/ZEHttp.h>
#include <ze/convention.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <charconv>

namespace ze
{

	void ZEHttpClient::httpRequestCallback(struct evhttp_request *req, void *arg)
	{
		auto pClient = static_cast<ZEHttpClient*>(arg);
		auto pListener = pClient->hHttpListener;
		auto pResponseBody = &pClient->xRepondBody;

		if (nullptr == req) { // on error
			pResponseBody->reset();
			pListener->onHttpError(pClient);
			return;
		}

		size_t len = evbuffer_get_length(req->input_buffer);
		if (!pResponseBody->resize(len)) {
			pListener->onHttpError(pClient);
			return;
		}
		evbuffer_remove(req->input_buffer, pResponseBody->data(), pResponseBody->size());
		pListener->onHttpRespondData(pClient);
	}

	ZE_API bool ZEHttpClient::init(const void * pParam)
	{
		assert(hNetworkManager);
		assert(hHttpListener);
		assert(pConnection == nullptr);
		assert(opUri);
		assert(opHost && oxPort);

		pConnection = evhttp_connection_base_bufferevent_new(
			hNetworkManager->eventBase(), hNetworkManager->dnsBase(),  nullptr, opHost, oxPort);
		auto pRequest = evhttp_request_new(&httpRequestCallback, this);
		if (!pConnection || !pRequest) {
			goto LABEL_INIT_ERROR;
		}
		if (opLocalAddress) {
			evhttp_connection_set_local_address(pConnection, opLocalAddress);
		}
		if (oxLocalPort) {
			evhttp_connection_set_local_port(pConnection, oxLocalPort);
		}
		evhttp_connection_set_flags(pConnection, EVHTTP_CON_REUSE_CONNECTED_ADDR | EVHTTP_CON_LINGERING_CLOSE);
		evhttp_connection_set_timeout(pConnection, oxTimeoutBySecond ? oxTimeoutBySecond : 1);
		evhttp_connection_set_retries(pConnection, 0); // forbid auto-retry
		evhttp_connection_set_max_body_size(pConnection, static_cast<ev_ssize_t>(cMaxRespondSize));

		if(evhttp_add_header(pRequest->output_headers, "Host", opHost)) {
			goto LABEL_INIT_ERROR;
		};
		for (auto & exHeader: oxExtraHeaders) {
			if (evhttp_add_header(pRequest->output_headers, exHeader.hKey, exHeader.hValue)) {
				goto LABEL_INIT_ERROR;
			}
		}
		if (oxRequestBody.size())
		{
			char buffer[128]; // even on base of 2, buffer is big enough for any 64-bit integer
			ZE_UNUSED auto [p, ec] = std::to_chars(buffer, buffer + sizeof(buffer), oxRequestBody.size());
			assert(ec == std::errc());
			assert(p < buffer + sizeof(buffer) && *p == '\0');
			if (evhttp_add_header(pRequest->output_headers, "Content-Length", buffer)
			|| (opContentType && evhttp_add_header(pRequest->output_headers, "Content-Type", opContentType))
			|| evbuffer_add(pRequest->output_buffer, oxRequestBody, oxRequestBody.size())) {
				goto LABEL_INIT_ERROR;
			}
		}
		if (evhttp_make_request(pConnection, steal(pRequest), static_cast<evhttp_cmd_type>(oxMethod), opUri)) {
			goto LABEL_INIT_ERROR;
		}
		return true;

		LABEL_INIT_ERROR:
		if (pRequest) {
			evhttp_request_free(steal(pRequest));
		}
		if (pConnection) {
			evhttp_connection_free(steal(pConnection));
		}
		return false;
	}

	ZE_API void ZEHttpClient::clean()
	{
		xMime.reset();
		xRepondBody.reset();
		evhttp_connection_free(steal(pConnection));
	}

}
