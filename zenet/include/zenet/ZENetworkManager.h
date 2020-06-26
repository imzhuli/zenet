#pragma once
#include "./__ENV__.h"
#include "./ZEDns.h"
#include <ze/dlist.h>
#include <ze/ext/memory_pool.h>

namespace ze
{

	class ZENetworkManager final
	: public IResource
	{
	public:
		struct EnvGuard {
			ZE_API EnvGuard();
			ZE_API ~EnvGuard();
		private:
			EnvGuard(EnvGuard&&) = delete; // no copy && no move
		};

		ZE_API ZENetworkManager();
		ZE_API ~ZENetworkManager();
		ZE_API bool init(const void * pParam) override;
		ZE_API void clean() override;

	public:
		ze::tag::config<uintptr_t> cMaxDnsRequestCount     = 256;
		ze::tag::config<bool>      cEnablePredefinedDnsServer = true;

	private:
		class AsyncContext final : public ze::DListNode {
			friend class ZENetworkManager;
		private:
			char                                        xDomainName[ZE_MAX_DOMAIN_NAME_LENGTH + 1];
			tag::handle<ZEDnsResultListener *>          hDnsResultListener;
			tag::handle<evdns_getaddrinfo_request *>    hRequest;
			tag::handle<MemoryPool<AsyncContext> *>     hRecyclePool;
		};
		event_base *                _pEventBase            = nullptr;
		evdns_base *                _pDnsBase              = nullptr;
		MemoryPool<AsyncContext>    _xDnsRequestPool;
		DList<AsyncContext>         _xDnsRequestList;

	public:
		ZE_FORCE_INLINE bool isReady() const { return _pEventBase; }
		ZE_FORCE_INLINE event_base * eventBase() const { return _pEventBase; }
		ZE_FORCE_INLINE evdns_base * dnsBase() const { return _pDnsBase; }

		ZE_API void loopOnce();
		ZE_API void infinateLoop();
		ZE_API void deferBreak();

		// dns request:
		ZE_API void addNameServer(const char * ipstr);
		ZE_API AsyncContext * asyncResolve(const char * hostname, ZEDnsResultListener * listener);
		ZE_API void cancelResolve(AsyncContext * && pContext);

	private:
		static void dns_callback(int  errcode,  struct  evutil_addrinfo *addr, void *usr_ptr);
	};

}
