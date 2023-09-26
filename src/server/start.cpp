
#include "server/inc/start.hpp"
#include "event2/http.h"
#include "ewarn/inc/server.hpp"
#include "server/inc/exit.hpp"
#include "wxpusher/inc/callback.hpp"

#include <iostream>

static void timer_flush(evutil_socket_t fd, short events, void *arg) {

    std::cout << std::flush;

    auto d = (timer_data *)arg;
    event_add(d->ev, &d->interval);
}

static timer_data *register_timer_helper(struct event_base *base,
                                         void (*fn)(long long, short, void *),
                                         int id, int time) {
    timer_data *data = (timer_data *)malloc(sizeof(timer_data));
    data->ev = event_new(base, -1, 0, fn, data);

    evutil_timerclear(&data->interval);
    data->id = id;
    data->interval.tv_sec = time;

    event_add(data->ev, &data->interval);

    return data;
}

std::vector<timer_data *> register_timer(struct event_base *base) {

    int id = 0;
    std::vector<timer_data *> ret;
    ret.emplace_back(register_timer_helper(base, timer_flush, id++, 20));

    return ret;
}

void register_callback(struct event_base *base, struct evhttp *http) {
    evhttp_set_cb(http, "/api/exit/gen", api_exit_gen, nullptr);
    evhttp_set_cb(http, "/api/exit/verify", api_exit_verify, base);
    evhttp_set_cb(http, "/api/ewarn/creata_qrcode", ewarn_api_create_qrcode,
                  nullptr);
    evhttp_set_cb(http, "/api/wxpusher_callback", wxpusher_callback, nullptr);
}