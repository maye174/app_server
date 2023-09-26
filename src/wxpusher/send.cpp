
#include "wxpusher/inc/send.hpp"

#include <curl/curl.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>

static size_t write_callback(void *contents, size_t size, size_t nmemb,
                             void *userp) {
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

void send_message(const std::string &appToken, const std::string &content,
                  const std::vector<std::string> &uids) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        std::cerr << "wxpusher send_message: can't init curl\n";
        return;
    }

    std::string response;

    // 构建请求体
    nlohmann::json request_body;
    request_body["appToken"] = appToken;
    request_body["content"] = content;
    request_body["contentType"] = 1;
    request_body["uids"] = uids;
    request_body["verifyPay"] = false;

    std::string body = request_body.dump();

    // 设置请求选项
    curl_easy_setopt(curl, CURLOPT_URL,
                     "https://wxpusher.zjiecode.com/api/send/message");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    // 设置请求头
    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // 发送请求
    CURLcode res = curl_easy_perform(curl);

    // 处理响应
    if (res != CURLE_OK) {
        std::cerr << "请求失败，错误信息：" << curl_easy_strerror(res)
                  << std::endl;
    } else {
        // 使用nlohmann/json库解析响应
        nlohmann::json jsonResponse = nlohmann::json::parse(response);
        int code = jsonResponse["code"];
        std::string msg = jsonResponse["msg"];

        std::cout << "状态码：" << code << std::endl;
        std::cout << "提示消息：" << msg << std::endl;

        // 验证发送状态
        if (code == 1000) {
            std::cout << "发送成功" << std::endl;
        } else {
            std::cout << "发送失败" << std::endl;
        }
    }

    // 清理资源
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}