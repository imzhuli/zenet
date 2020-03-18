#include <ze/convention.h>
#include <ze/chrono.h>
#include <event2/dns.h>
#include <event2/event.h>
#include <event2/thread.h>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
using namespace std;

static void dns_callback(int  errcode,  struct  evutil_addrinfo *addr, void *usr_ptr)
{
	if (errcode) {
		assert(!addr);
		cout << "callback: errorcode=" << evutil_gai_strerror(errcode) << endl;
	}
	else {
		evutil_freeaddrinfo(addr);
		size_t count = 1;
		while((addr = addr->ai_next)) {
			++count;
		}
		cout << "callback: address count=" << count << endl;
	}
}


int main(int, char **)
{
	const char * host = "www.qq.com";

	evthread_use_pthreads();
	event_base * event_base = nullptr;
	evdns_base * evdns_base = nullptr;

	event_base = event_base_new();
	evdns_base = evdns_base_new(event_base, EVDNS_BASE_INITIALIZE_NAMESERVERS | EVDNS_BASE_DISABLE_WHEN_INACTIVE);

	struct evutil_addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = EVUTIL_AI_CANONNAME;
	auto request = evdns_getaddrinfo(evdns_base, host, nullptr, &hints, dns_callback, nullptr);
	cout << "Request: " << (void*)request << endl;

	evdns_getaddrinfo_cancel(request);
	event_base_loop(event_base, EVLOOP_NONBLOCK);
	evdns_base_free(evdns_base, 0);

	ze::Timer t;
	while(t.elapsed() < 1s) {
		event_base_loop(event_base, EVLOOP_ONCE);
	}
	event_base_free(event_base);

}