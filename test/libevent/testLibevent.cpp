#include <ze/convention.h>
#include <ze/chrono.h>
#include <ze/ext/string.h>
#include <zenet/ZENetworkManager.h>
#include <zenet/ZEConnection.h>
#include <zenet/ZEDns.h>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>
using namespace std;
using namespace ze;

static bool testSuccessful = false;

struct ZEDnsResultPrinter
	: ZEDnsResultListener
{
	void onDnsResult(const char* hostname, const ZEDomainInfo* domainInfo, const char* errstr)
	{
		cout << "hostname: " << hostname << endl;
		if (!domainInfo) {
			cout << "error: " << errstr << endl;
			return;
		}
		for (size_t i = 0; i < domainInfo->xAddressNumber; ++i)
		{
			auto& addr = domainInfo->xAddresses[i];
			if (addr.addr.sa_family == AF_INET) {
				auto a4 = reinterpret_cast<const uint8_t *>(&addr.addr4.sin_addr);
				cout << "IPv4: "
					<< (int)a4[0] << '.'
					<< (int)a4[1] << '.'
					<< (int)a4[2] << '.'
					<< (int)a4[3]
					<< endl;
			}
		}
		testSuccessful = true;
	}
};

ZE_UNUSED static const char* host = "www.baidu.com";

int main(int, char **)
{
	ZENetworkManager::EnvGuard nmEnvGuard;

	ZENetworkManager nm;
	ZEDnsResultPrinter dnsPrinter;

	nm.init(nullptr);
	nm.asyncResolve(host, &dnsPrinter);

	ze::Timer t;
	while(t.elapsed() < 5s && !testSuccessful) {
		nm.loopOnce();
	}
	nm.clean();
	std::this_thread::sleep_for(1s);
}