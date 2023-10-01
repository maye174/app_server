//
// Create by github:Maye174
// 2023/9/25
//

#include "event2/event.h"
#include "server/inc/start.hpp"

#include <loguru.hpp>

#ifdef _WIN32
#include <winsock2.h>
#endif

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    // #ifndef DEBUG
    //     loguru::g_stderr_verbosity = loguru::Verbosity_ERROR;
    // #else
    loguru::g_stderr_verbosity = loguru::Verbosity_INFO;
    // #endif

    loguru::add_file("app.log", loguru::Append, loguru::Verbosity_MAX);

    struct event_base *base = event_base_new();
    if (!base) {
        LOG_F(ERROR, "Failed to create event base");
        return 1;
    }

    struct evhttp *http = evhttp_new(base);
    if (!http) {
        LOG_F(ERROR, "Failed to create HTTP server");
        return 1;
    }

    register_callback(base, http);
    auto ev_list = register_timer(base);

    if (evhttp_bind_socket(http, "0.0.0.0", 3000) != 0) {
        LOG_F(ERROR, "Failed to bind to port 3000");
        return 1;
    }

    event_base_dispatch(base);

    for (auto &d : ev_list) {
        event_free(d->ev);
        free(d);
    }
    evhttp_free(http);
    event_base_free(base);

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}