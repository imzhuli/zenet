#pragma once
#include "./ZEDns.h"
#include <ze/convention.h>
#include <ze/dlist.h>
#include <ze/ext/memory_pool.h>
#include <event2/thread.h>
#include <event2/event.h>
#include <event2/dns.h>

namespace ze
{

	class ZENetworkManager final
	: public IResource
	{
	public:
		static void setupEnv();

		bool init(const void * pParam) override;
		void clean() override;

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

		void loopOnce();
		void infinateLoop();
		void deferBreak();

		// dns request:
		void addNameServer(const char * ipstr);
		AsyncContext * asyncResolve(const char * hostname, ZEDnsResultListener * listener);
		void cancelResolve(AsyncContext * && pContext);

	private:
		static void dns_callback(int  errcode,  struct  evutil_addrinfo *addr, void *usr_ptr);
	};

}
