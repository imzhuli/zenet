#include <ze/convention.h>
#include <ze/chrono.h>
#include <zenet/ZENetworkManager.h>
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
			cout << "IP: " << inet_ntoa(domainInfo->xAddresses[i].sin_addr) << endl;
		}
		testSuccessful = true;
	}
};

static const char* host = "www.baidu.com";

int main(int, char **)
{
	ZENetworkManager::setupEnv();

	ZENetworkManager nm;
	ZEDnsResultPrinter dnsPrinter;

	ResGuard guard{ nm };
	nm.asyncResolve(host, &dnsPrinter);

	ze::Timer t;
	while(t.elapsed() < 5s && !testSuccessful) {
		nm.loopOnce();
	}
	std::this_thread::sleep_for(1s);
}