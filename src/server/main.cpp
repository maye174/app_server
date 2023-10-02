//
// Create by github:Maye174
// 2023/9/25
//

#include "event2/event.h"
#include "event2/rpc.h"
#include "server/inc/start.hpp"

#include <loguru.hpp>
#include <thread>

#ifdef _WIN32
#include <winsock2.h>
#endif

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

#ifdef DEBUG
    loguru::g_stderr_verbosity = loguru::Verbosity_MAX;
#else
    loguru::g_stderr_verbosity = loguru::Verbosity_OFF;
#endif

    loguru::add_file("error.log", loguru::Append, loguru::Verbosity_ERROR);
    loguru::add_file("info.log", loguru::Append, loguru::Verbosity_MAX);

    struct event_base *base = event_base_new();
    if (!base) {
        LOG_F(ERROR, "Failed to create event base");
        return 1;
    }

    struct event_base *base_timer = event_base_new();
    if (!base_timer) {
        LOG_F(ERROR, "Failed to create event base_timer");
        return 1;
    }

    struct evhttp *http = evhttp_new(base);
    if (!http) {
        LOG_F(ERROR, "Failed to create HTTP server");
        return 1;
    }

    register_callback(base, http);

    // 注册定时器 第二个线程执行
    auto ev_list = register_timer(base_timer);

    if (evhttp_bind_socket(http, "0.0.0.0", 3000) != 0) {
        LOG_F(ERROR, "Failed to bind to port 3000");
        return 1;
    }

    std::thread timer_thread(event_base_dispatch, base_timer);
    event_base_dispatch(base);

    if (timer_thread.joinable())
        timer_thread.join();

    for (auto &d : ev_list) {
        event_free(d->ev);
        free(d);
    }
    evhttp_free(http);
    event_base_free(base);
    event_base_free(base_timer);

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}