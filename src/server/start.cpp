
#include "server/inc/start.hpp"
#include "event2/http.h"
#include "ewarn/inc/server.hpp"
#include "server/inc/exit.hpp"
#include "wxpusher/inc/callback.hpp"

#include <iostream>

static void timer_flush(evutil_socket_t fd, short events, void *arg) {

    std::cout << std::flush;

    struct event *ev = (struct event *)arg;
    struct timeval tv;
    evutil_timerclear(&tv);
    tv.tv_sec = *(int *)arg;
    event_add(ev, &tv);
}

static struct event *register_timer_helper(struct event_base *base,
                                           void (*fn)(long long, short, void *),
                                           int time) {
    struct event *ev = event_new(base, -1, 0, fn, &time);
    struct timeval tv;
    evutil_timerclear(&tv);
    tv.tv_sec = time;
    event_add(ev, &tv);

    return ev;
}

std::vector<struct event *> register_timer(struct event_base *base,
                                           struct evhttp *http) {
    //
    std::vector<struct event *> ret;
    ret.emplace_back(register_timer_helper(base, timer_flush, 20));

    return ret;
}

void register_callback(struct event_base *base, struct evhttp *http) {
    evhttp_set_cb(http, "/api/exit/gen", api_exit_gen, nullptr);
    evhttp_set_cb(http, "/api/exit/verify", api_exit_verify, base);
    evhttp_set_cb(http, "/api/ewarn/creata_qrcode", ewarn_api_create_qrcode,
                  nullptr);
    evhttp_set_cb(http, "/api/wxpusher_callback", wxpusher_callback, nullptr);
}