
#include "ewarn/inc/server.hpp"
#include "server/inc/util.hpp"
#include "wxpusher/inc/qrcode.hpp"

#include <string>
#include <vector>

#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>
#include <loguru.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void ewarn_api_create_qrcode(struct evhttp_request *req, void *arg) {
    struct evbuffer *buf = evbuffer_new();
    if (!buf) {
        LOG_F(ERROR, "ewarn_api_create_qrcode: Failed to create response "
                     "buffer");
        evhttp_send_error(req, 500, "Internal Server Error");
        return;
    }

    // 获取请求正文
    struct evbuffer *input_buffer = evhttp_request_get_input_buffer(req);
    size_t input_len = evbuffer_get_length(input_buffer);
    char *input_data = (char *)malloc(sizeof(char) * (input_len + 1));
    evbuffer_remove(input_buffer, input_data, input_len);
    input_data[input_len] = '\0';

    LOG_F(INFO, "%s", input_data);

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

    if (!j.contains("building_number") || !j.contains("room_number")) {
        LOG_F(ERROR, "未找到building_number 或者 "
                     "room_number\n 400");
        evhttp_send_error(req, 400, "Bad Request");
        evbuffer_free(buf);
        return;
    }

    std::string building_number = j["building_number"].get<std::string>();
    std::string room_number = j["room_number"].get<std::string>();

    std::string extra =
        "{\"b\":\"" + building_number + "\",\"r\":\"" + room_number + "\"}";

    std::string app_token = get_someting_from_env("WXPUSHER_APP_EW_TOKEN");
    std::string qrcode_url = create_qrcode(app_token, extra, 1800);

    std::string response_body = "{\"url\":\"" + qrcode_url + "\"}";

    LOG_F(INFO, "%s", response_body.c_str());

    evbuffer_add_printf(buf, response_body.c_str());
    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type",
                      "application/json");
    evhttp_send_reply(req, 200, "OK", buf);
    evbuffer_free(buf);
    free(input_data);
}

extern std::string load_builds_json();
extern std::string load_rooms_json(const std::string &buildid);

void ewarn_api_get_epay_json(struct evhttp_request *req, void *arg) {
    struct evbuffer *buf = evbuffer_new();
    if (!buf) {
        LOG_F(ERROR, "ewarn_api_create_qrcode: Failed to create response "
                     "buffer");
        evhttp_send_error(req, 500, "Internal Server Error");
        return;
    }

    // 获取请求正文
    struct evbuffer *input_buffer = evhttp_request_get_input_buffer(req);
    size_t input_len = evbuffer_get_length(input_buffer);
    char *input_data = (char *)malloc(sizeof(char) * (input_len + 1));
    evbuffer_remove(input_buffer, input_data, input_len);
    input_data[input_len] = '\0';

    LOG_F(INFO, "%s", input_data);

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

    if (!j.contains("is_build")) {
        LOG_F(ERROR, "未找到is_build\n 400");
        evhttp_send_error(req, 400, "Bad Request");
        evbuffer_free(buf);
        return;
    }

    int is_build = j["is_build"].get<int>();

    std::string response_body;

    if (is_build) {
        response_body = load_builds_json();
    } else {
        response_body = load_rooms_json(j["buildid"].get_ref<std::string &>());
    }

    LOG_F(INFO, "%s", response_body.c_str());

    if (response_body.empty()) {
        evhttp_send_error(req, 400, "Bad Request");
        evbuffer_free(buf);
        return;
    }

    evbuffer_add_printf(buf, response_body.c_str());
    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type",
                      "application/json");
    evhttp_send_reply(req, 200, "OK", buf);
    evbuffer_free(buf);
    free(input_data);
}