
#include "inc/start.hpp"
#include "ewarn/inc/server.hpp"
#include "wxpusher/inc/callback.hpp"

void register_callback(struct evhttp *http) {
    evhttp_set_cb(http, "ewarn/api/creata_qrcode", ewarn_api_create_qrcode,
                  nullptr);
    evhttp_set_cb(http, "/api/wxpusher_callback", wxpusher_callback, nullptr);
}