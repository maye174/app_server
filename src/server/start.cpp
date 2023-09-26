
#include "server/inc/start.hpp"
#include "event2/http.h"
#include "ewarn/inc/server.hpp"
#include "server/inc/exit.hpp"
#include "wxpusher/inc/callback.hpp"

void register_callback(struct event_base *base, struct evhttp *http) {
    evhttp_set_cb(http, "/api/exit", api_exit, base);
    evhttp_set_cb(http, "/api/ewarn/creata_qrcode", ewarn_api_create_qrcode,
                  nullptr);
    evhttp_set_cb(http, "/api/wxpusher_callback", wxpusher_callback, nullptr);
}