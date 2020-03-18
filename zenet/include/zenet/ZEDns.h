#pragma once
#include <ze/convention.h>
#include <event2/event.h>

namespace ze
{
	constexpr size_t ZE_MAX_DOMAIN_NAME_LENGTH = 128;
	constexpr size_t ZE_MAX_RESOLVE_ADDRESS_NUMBER = 16;

	struct ZEDomainInfo
	{
		sockaddr_in xAddresses[ZE_MAX_RESOLVE_ADDRESS_NUMBER];
		size_t xAddressNumber = 0;
	};

	struct ZEDnsResultListener : ze::Interface
	{
		virtual void onDnsResult(const char * hostname, const ZEDomainInfo * domainInfo, const char * errstr) = 0;
	};

	// static assertions
	static_assert(std::is_trivially_copyable_v<ZEDomainInfo>);
}
