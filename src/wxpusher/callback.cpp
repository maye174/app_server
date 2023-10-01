
#include "wxpusher/inc/callback.hpp"
#include "wxpusher/inc/data.hpp"

#include "ewarn/inc/server.hpp"

#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>
#include <loguru.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void wxpusher_callback(struct evhttp_request *req, void *arg) {

    // 获取请求正文
    struct evbuffer *input_buffer = evhttp_request_get_input_buffer(req);
    size_t input_len = evbuffer_get_length(input_buffer);
    char *input_data = (char *)malloc(sizeof(char) * (input_len + 1));
    evbuffer_remove(input_buffer, input_data, input_len);
    input_data[input_len] = '\0';

    json j;

    try {
        j = json::parse(input_data);
    } catch (const json::parse_error &e) {
        LOG_F(ERROR, "解析JSON数据时发生异常 - %s", e.what());
    } catch (const json::type_error &e) {
        LOG_F(ERROR, "JSON数据类型错误 - %s", e.what());
    } catch (const std::exception &e) {
        LOG_F(ERROR, "发生未知异常 - %s", e.what());
    }

    if (!j.contains("action")) {
        LOG_F(ERROR, "未找到action\n 400");
        evhttp_send_error(req, 400, "Bad Request");
        return;
    }

    std::string action = j["action"].get_ref<std::string &>();

    if (action == "app_subscribe") { // 用户订阅
        api_user_attention_callback(req, arg);
    }

    free(input_data);
}