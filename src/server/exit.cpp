
#include "server/inc/exit.hpp"
#include "event2/event.h"

#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>
#include <fmt/core.h>
#include <nlohmann/json.hpp>
#include <openssl/sha.h>

using json = nlohmann::json;

static std::string gen_key() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(100000, 999999);

    int random_number = dis(gen);

    std::stringstream ss;
    ss << random_number;

    std::string input = ss.str();

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input.c_str(), input.size());
    SHA256_Final(hash, &sha256);

    std::stringstream sha256_ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sha256_ss << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(hash[i]);
    }

    return sha256_ss.str();
}

std::string key;

void api_exit_gen(struct evhttp_request *req, void *arg) {
    key = gen_key();
    fmt::println(key);
}

void api_exit_verify(struct evhttp_request *req, void *arg) {
    struct evbuffer *buf = evbuffer_new();
    if (!buf) {
        fmt::print("(fn)api_exit: Failed to create response buffer\n");
        evhttp_send_error(req, 500, "Internal Server Error");
        return;
    }

    // 获取请求正文
    struct evbuffer *input_buffer = evhttp_request_get_input_buffer(req);
    size_t input_len = evbuffer_get_length(input_buffer);
    char *input_data = new char[input_len];
    evbuffer_remove(input_buffer, input_data, input_len);

    std::cerr << "(fn)api_exit: " << input_data << std::endl;

    json j;

    try {
        j = json::parse(input_data);
    } catch (const json::parse_error &e) {
        fmt::println("(fn)api_exit: 错误 解析JSON数据时发生异常", e.what());
    } catch (const json::type_error &e) {
        fmt::println("(fn)api_exit: 错误 JSON数据类型错误", e.what());
    } catch (const std::exception &e) {
        fmt::println("(fn)api_exit: 错误 发生未知异常", e.what());
    }

    if (!j.contains("key")) {
        fmt::print("(fn)api_exit: 空内容\n 400\n");
        evhttp_send_error(req, 400, "Bad Request");
        evbuffer_free(buf);
        return;
    }

    std::string key1 = j["key"].get<std::string>();

    if (key != key1) {
        fmt::println("(fn)api_exit: key错误");
        evhttp_send_error(req, 400, "Bad Request");
        evbuffer_free(buf);
        return;
    }

    key = gen_key();

    evhttp_send_reply(req, 200, "OK", buf);
    evbuffer_free(buf);
    delete[] input_data;

    event_base_loopexit((struct event_base *)arg, NULL);
}
