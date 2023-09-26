
#include "ewarn/inc/server.hpp"
#include "server/inc/util.hpp"
#include "wxpusher/inc/qrcode.hpp"

#include <iostream>
#include <string>
#include <vector>

#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>
#include <fmt/core.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void ewarn_api_create_qrcode(struct evhttp_request *req, void *arg) {
    struct evbuffer *buf = evbuffer_new();
    if (!buf) {
        fmt::print(
            "ewarn_api_create_qrcode: Failed to create response buffer\n");
        evhttp_send_error(req, 500, "Internal Server Error");
        return;
    }

    // 获取请求正文
    struct evbuffer *input_buffer = evhttp_request_get_input_buffer(req);
    size_t input_len = evbuffer_get_length(input_buffer);
    char *input_data = new char[input_len];
    evbuffer_remove(input_buffer, input_data, input_len);

    std::cerr << "(fn)ewarn_api_create_qrcode: " << input_data << std::endl;

    json j;

    try {
        j = json::parse(input_data);
    } catch (const json::parse_error &e) {
        fmt::println("错误:解析JSON数据时发生异常 - ", e.what());
    } catch (const json::type_error &e) {
        fmt::println("错误:JSON数据类型错误 - ", e.what());
    } catch (const std::exception &e) {
        fmt::println("错误：发生未知异常 - ", e.what());
    }

    if (!j.contains("building_number") || !j.contains("room_number")) {
        fmt::print("ewarn_api_create_qrcode: 未找到building_number 或者 "
                   "room_number\n 400\n");
        evhttp_send_error(req, 400, "Bad Request");
        evbuffer_free(buf);
        return;
    }

    std::string building_number = j["building_number"].get<std::string>();
    std::string room_number = j["room_number"].get<std::string>();

    std::string extra = "{b:" + building_number + ",r:" + room_number + "}";

    std::string app_token = get_someting_from_env("WXPUSHER_APP_EW_TOKEN");
    std::string qrcode_url = create_qrcode(app_token, extra, 1800);

    std::string response_body = "{\"url\":\"" + qrcode_url + "\"}";

    std::cerr << response_body << std::endl;

    evbuffer_add_printf(buf, response_body.c_str());
    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type",
                      "application/json");
    evhttp_send_reply(req, 200, "OK", buf);
    evbuffer_free(buf);
    delete[] input_data;
}
