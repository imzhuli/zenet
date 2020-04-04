#include <zenet/ZENetworkManager.h>
#include <thread>
#include <mutex>

namespace ze
{
	// prerequisites:
	static_assert(std::has_unique_object_representations_v<sockaddr>);
	static_assert(std::has_unique_object_representations_v<sockaddr_in>);
	static_assert(std::has_unique_object_representations_v<sockaddr_in6>);
	static_assert(std::is_same_v<decltype(sockaddr::sa_family), decltype(sockaddr_in::sin_family)>);
	static_assert(std::is_same_v<decltype(sockaddr_in::sin_family), decltype(sockaddr_in6::sin6_family)>);
	static_assert(offsetof(sockaddr, sa_family) == offsetof(sockaddr_in, sin_family));
	static_assert(offsetof(sockaddr, sa_family) == offsetof(sockaddr_in6, sin6_family));
	static_assert(sizeof(sockaddr) <= sizeof(sockaddr_in));
	static_assert(sizeof(sockaddr_in) <= sizeof(sockaddr_in6));

	static bool sxEnvReady = false;
	static std::once_flag sgNetworkInitFlag;

	static const auto scPredefinedDnsServer = carray {
		type<const char *>,
		"223.5.5.5",
		"8.8.8.8",
	};

	static evutil_addrinfo scHints = {
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_STREAM,
		.ai_protocol = IPPROTO_TCP,
	};

	ZE_UNUSED static void EventLogNone(int severity, const char* msg)
	{}

	void ZENetworkManager::setupEnv()
	{
		std::call_once(sgNetworkInitFlag, [] {

#ifdef NDEBUG
			event_enable_debug_logging(EVENT_DBG_NONE);
			event_set_log_callback(EventLogNone);
#endif

#ifdef WIN32
		WSADATA wsa_data;
		WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif

#if defined(EVTHREAD_USE_WINDOWS_THREADS_IMPLEMENTED)
			expect(!evthread_use_windows_threads());
#elif defined(EVTHREAD_USE_PTHREADS_IMPLEMENTED)
			expect(!evthread_use_pthreads());
#endif
			sxEnvReady = true;
		});
	}

	bool ZENetworkManager::init(const void * pParam)
	{
		assert(sxEnvReady);

		auto pConfig = event_config_new();
		if (!pConfig) {
			return false;
		}
		claim(0 ==  event_config_set_flag(pConfig, EVENT_BASE_FLAG_EPOLL_USE_CHANGELIST));
		claim(0 ==  event_config_set_flag(pConfig, EVENT_BASE_FLAG_STARTUP_IOCP));
		_pEventBase = event_base_new_with_config(pConfig);
		event_config_free(steal(pConfig));

		if (!_pEventBase) {
			return false;
		}
		_pDnsBase = evdns_base_new(_pEventBase, EVDNS_BASE_INITIALIZE_NAMESERVERS | EVDNS_BASE_DISABLE_WHEN_INACTIVE);
		if (!_pDnsBase) {
			event_base_free(steal(_pEventBase));
			return false;
		}

	#ifdef EVDNS_BASE_CONFIG_WINDOWS_NAMESERVERS_IMPLEMENTED
		evdns_base_config_windows_nameservers(_pDnsBase);
	#endif
		if (cEnablePredefinedDnsServer) {
			for(auto dnsIp : scPredefinedDnsServer)	{
				evdns_base_nameserver_ip_add(_pDnsBase, dnsIp);
			}
		}

		_xDnsRequestPool.cInitSize = cMaxDnsRequestCount;
		_xDnsRequestPool.cMaxPoolSize = cMaxDnsRequestCount;
		if (!_xDnsRequestPool.init(nullptr)) {
			evdns_base_free(steal(_pDnsBase), 0);
			event_base_free(steal(_pEventBase));
			return false;
		}

		return true;
	}

	void ZENetworkManager::clean()
	{
		// cancel requests:
		do {
			for (auto & request : _xDnsRequestList) {
				evdns_getaddrinfo_cancel(request.hRequest);
			}
		} while(false);
		event_base_loop(_pEventBase, EVLOOP_NONBLOCK);
		assert(_xDnsRequestList.empty());

		_xDnsRequestPool.clean();
		evdns_base_free(steal(_pDnsBase), 0);
		event_base_free(steal(_pEventBase));
	}

	// DNS Session:
	void ZENetworkManager::dns_callback(int  errcode,  struct  evutil_addrinfo *addr, void *usr_ptr)
	{
		auto pRequest = static_cast<AsyncContext *>(usr_ptr);
		if (errcode) {
			assert(!addr);
			if (errcode != EVUTIL_EAI_CANCEL) {
				pRequest->hDnsResultListener->onDnsResult(pRequest->xDomainName, nullptr, evutil_gai_strerror(errcode));
			}
		}
		else {
			ZEDomainInfo result;
			auto iterator = addr;
			size_t count = 0;
			while(iterator && count < ZE_MAX_RESOLVE_ADDRESS_NUMBER) {
				if(iterator->ai_family == AF_INET && iterator->ai_addrlen == sizeof(sockaddr_in)) { // only process af_inet address
					result.xAddresses[count].addr4 = reinterpret_cast<sockaddr_in&>(*iterator->ai_addr);
					++count;
				}
				if (iterator->ai_family == AF_INET6 && iterator->ai_addrlen == sizeof(sockaddr_in6)) { // only process af_inet address
					result.xAddresses[count].addr6 = reinterpret_cast<sockaddr_in6&>(*iterator->ai_addr);
					++count;
				}
				iterator = iterator->ai_next;
			}
			result.xAddressNumber = count;
			pRequest->hDnsResultListener->onDnsResult(pRequest->xDomainName, &result,  evutil_gai_strerror(errcode));
			evutil_freeaddrinfo(addr);
		}
		auto pool = pRequest->hRecyclePool;
		pool->dealloc(pRequest);
	}

	void ZENetworkManager::addNameServer(const char * ipstr)
	{
		assert(_pDnsBase);
		evdns_base_nameserver_ip_add(_pDnsBase, ipstr);
	}

	ZENetworkManager::AsyncContext * ZENetworkManager::asyncResolve(const char * hostname, ZEDnsResultListener * listener)
	{
		size_t hostnameLength = strlen(hostname);
		if (hostnameLength > ZE_MAX_DOMAIN_NAME_LENGTH) {
			listener->onDnsResult(hostname, nullptr, "Too long hostname");
			return nullptr;
		}
		AsyncContext * pContext = _xDnsRequestPool.alloc();
		if (!pContext) {
			listener->onDnsResult(hostname, nullptr, "Max request reached");
			return nullptr;
		}
		memcpy(pContext->xDomainName, hostname, hostnameLength);
		pContext->xDomainName[hostnameLength] = '\0';
		pContext->hRecyclePool = &_xDnsRequestPool;
		pContext->hDnsResultListener = listener;
		auto request = evdns_getaddrinfo(_pDnsBase, hostname, nullptr, &scHints, &dns_callback, pContext);
		if (request) {
			pContext->hRequest = request;
			_xDnsRequestList.addTail(*pContext);
			return pContext;
		}
		return nullptr;
	}

	void ZENetworkManager::cancelResolve(AsyncContext * && pContext)
	{
		assert(pContext);
		#ifndef NDEBUG
			bool found = false;
			for(auto & req : _xDnsRequestList) {
				if (&req == pContext) {
					found = true;
					break;
				}
			}
			assert(found);
			assert(pContext->hRequest);
		#endif
		// *pContext will be recycled in callback (defered to next networkmanager loop):
		evdns_getaddrinfo_cancel(pContext->hRequest);
	}

	void ZENetworkManager::loopOnce()
	{
		 event_base_loop(_pEventBase, EVLOOP_NONBLOCK);
	}

	void ZENetworkManager::infinateLoop()
	{
		event_base_loop(_pEventBase, EVLOOP_NO_EXIT_ON_EMPTY);
	}

	void ZENetworkManager::deferBreak()
	{
		event_base_loopbreak(_pEventBase);
	}

}
