#pragma once

// include this first to avoid windows MIN/MAX issue
#include <ze/convention.h>
#include <cassert>

#ifndef EVENT__HAVE_GETADDRINFO
	#define EVENT__HAVE_GETADDRINFO
#endif
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <event2/thread.h>
#include <event2/util.h>
#include <event2/dns.h>
#include <event2/http.h>
#include <event2/http_struct.h>
