
#include "wxpusher/inc/callback.hpp"
#include "wxpusher/inc/data.hpp"

#include "ewarn/inc/server.hpp"

#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

void wxpusher_callback(struct evhttp_request *req, void *arg) {
    // // 解析POST请求数据
    // struct evkeyvalq params;
    // evhttp_parse_query_str((const char *)evhttp_request_get_uri(req),
    // &params); const char *action = evhttp_find_header(&params, "action");

    // 获取请求正文
    struct evbuffer *input_buffer = evhttp_request_get_input_buffer(req);
    size_t input_len = evbuffer_get_length(input_buffer);
    char *input_data = new char[input_len];
    evbuffer_remove(input_buffer, input_data, input_len);

    json j = json::parse(input_data);
    std::string action = j["action"].get_ref<std::string &>();

    if (action == "app_subscribe") { // 用户订阅
        api_user_attention_callback(req, arg);
    }
}