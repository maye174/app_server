//
// Create by github:Maye174
// 2023/9/25
//

#include "wxpusher/inc/qrcode.hpp"
#include "fmt/core.h"

#include <curl/curl.h>
#include <iostream>
#include <nlohmann/json.hpp>

static size_t write_callback(void *contents, size_t size, size_t nmemb,
                             void *userp) {
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

std::string create_qrcode(const std::string &app_token,
                          const std::string &extra, int valid_time) {
    CURL *curl;
    CURLcode res;
    std::string response;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type:application/json");

        std::string data = "{"
                           "\"appToken\":\"" +
                           app_token +
                           "\","
                           "\"extra\":\"" +
                           extra +
                           "\","
                           "\"validTime\":" +
                           std::to_string(valid_time) + "}";

        curl_easy_setopt(curl, CURLOPT_URL,
                         "https://wxpusher.zjiecode.com/api/fun/create/qrcode");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: "
                      << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    curl_global_cleanup();

    std::cerr << "(fn)create_qrcode \nqrcode_data:" << response << std::endl;

    // 解析返回的JSON数据以获取二维码图片的URL
    nlohmann::json j;
    try {
        j = nlohmann::json::parse(response);
    } catch (const nlohmann::json::parse_error &e) {
        fmt::println("错误:解析JSON数据时发生异常 - ", e.what());
    } catch (const nlohmann::json::type_error &e) {
        fmt::println("错误:JSON数据类型错误 - ", e.what());
    } catch (const std::exception &e) {
        fmt::println("错误：发生未知异常 - ", e.what());
    }

    if (!j.contains("success") || j["success"].get<bool>() != true) {
        fmt::println(
            "wxpusher: 二维码创建错误 "); //,
                                          // j["msg"].get_ref<std::string &>());
        return "";
    }

    return j["data"]["url"].get<std::string>();
}