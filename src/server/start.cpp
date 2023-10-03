
#include "server/inc/start.hpp"
#include "event2/http.h"
#include "ewarn/inc/server.hpp"
#include "ewarn/inc/task.hpp"
#include "server/inc/exit.hpp"
#include "wxpusher/inc/callback.hpp"

#include <atomic>

#include <loguru.hpp>

extern std::atomic<bool> exit_flag;

static void check_exit_flag(evutil_socket_t fd, short events, void *arg) {
    auto d = (timer_data *)arg;
    LOG_F(INFO, "id: %d", d->id);

    if (exit_flag) {
        LOG_F(INFO, "exit_flag is true, timer exit");
        event_base_loopbreak(d->base_timer);
        return;
    }

    event_add(d->ev, &d->interval);
}

static timer_data *register_timer_helper(struct event_base *base_timer,
                                         void (*fn)(evutil_socket_t fd,
                                                    short events, void *arg),
                                         int id, int time) {
    timer_data *data = (timer_data *)malloc(sizeof(timer_data));
    data->base_timer = base_timer;
    data->ev = event_new(base_timer, -1, 0, fn, data);

    evutil_timerclear(&data->interval);
    data->id = id;
    data->interval.tv_sec = time;

    event_add(data->ev, &data->interval);

    return data;
}

std::vector<timer_data *> register_timer(struct event_base *base_timer) {

    int id = 0;
    std::vector<timer_data *> ret;
    ret.emplace_back(
        register_timer_helper(base_timer, check_exit_flag, id++, 20));
    ret.emplace_back(
        register_timer_helper(base_timer, ew_timer_task, id++, 86400));

    return ret;
}

void register_callback(struct event_base *base, struct evhttp *http) {
    evhttp_set_cb(http, "/api/exit/gen", exit_api_gen, nullptr);
    evhttp_set_cb(http, "/api/exit/verify", exit_api_verify, base);
    evhttp_set_cb(http, "/api/ewarn/creata_qrcode", ewarn_api_create_qrcode,
                  nullptr);
    evhttp_set_cb(http, "/api/ewarn/get_epay_json", ewarn_api_get_epay_json,
                  nullptr);
    evhttp_set_cb(http, "/api/wxpusher_callback", wxpusher_callback, nullptr);
}