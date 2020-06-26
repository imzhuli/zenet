#pragma once
#include "./__ENV__.h"
#include "./ZEConnection.h"
#include "./ZENetworkManager.h"

namespace ze
{
	class ZEHttpClientListener;
	class ZEHttpClient;

	class ZEHttpClientListener : Interface
	{
	public:
		ZE_API virtual ZE_FORCE_INLINE void onHttpRespondData(ZEHttpClient* pClient) {};
		ZE_API virtual ZE_FORCE_INLINE void onHttpError(ZEHttpClient* pClient) {};
	protected:
		ZE_API ZEHttpClientListener() = default;
		ZE_API ZEHttpClientListener(ZEHttpClientListener &&) = delete;
	};

	class ZEHttpClient final
	: public IResource
	{
	public:
		static constexpr const char * CONTENT_TYPE_JSON = "application/json";
		static constexpr const char * CONTENT_TYPE_TEXT = "text/plain";
		static constexpr const char * CONTENT_TYPE_HTML = "text/html";

		enum struct Method : uint_fast16_t
		{
			GET    = EVHTTP_REQ_GET,
			POST   = EVHTTP_REQ_POST,
			PUT    = EVHTTP_REQ_PUT,
		};

		struct HeaderView
		{
			tag::handle<const char *> hKey;
			tag::handle<const char *> hValue;
		};

		tag::handle<ZENetworkManager*>           hNetworkManager          = nullptr;
		tag::handle<ZEHttpClientListener *>      hHttpListener            = nullptr;
		tag::handle<Variant>                     hHttpListenerContext     = {};
		tag::config<ssize32_t>                   cMaxRespondSize          = std::numeric_limits<ssize32_t>::max() / 2;

		tag::option<const char *>                opLocalAddress           = nullptr;
		tag::option<uint_fast16_t>               oxLocalPort              = 0;
		tag::option<uint_fast32_t>               oxTimeoutBySecond        = 5;

		tag::option<const char *>                opHost                   = nullptr;
		tag::option<uint_fast16_t>               oxPort                   = 0;
		tag::option<const char *>                opContentType            = nullptr;
		tag::option<const char *>                opUri                    = "/";
		tag::option<Method>                      oxMethod                 = Method::GET;
		tag::option<ArrayView<HeaderView>>       oxExtraHeaders;
		tag::option<DataView>                    oxRequestBody;

	private:
		DynamicBuffer<32> xMime;
		DynamicBuffer<512> xRepondBody;
		evhttp_connection * pConnection = nullptr;

	private:
		static void httpRequestCallback(struct evhttp_request *req, void *arg);

	public:
		ZE_API bool init(const void * pParam) override;
		ZE_API void clean() override;

		ZE_FORCE_INLINE std::string_view respondMime() const { return xMime.view().sv(); }
		ZE_FORCE_INLINE DataView respondBody() const { return xRepondBody.view(); }

	};


}
