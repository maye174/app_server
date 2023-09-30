
#include "wxpusher/inc/send.hpp"

#include <curl/curl.h>
#include <loguru.hpp>
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
        LOG_F(ERROR, "can't init curl");
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
        LOG_F(ERROR, "请求失败，错误信息：%s \n", curl_easy_strerror(res));
    } else {
        // 使用nlohmann/json库解析响应
        nlohmann::json j;
        try {
            j = nlohmann::json::parse(response);
        } catch (const nlohmann::json::parse_error &e) {
            LOG_F(ERROR, "解析JSON数据时发生异常 - %s", e.what());
            return;
        } catch (const nlohmann::json::type_error &e) {
            LOG_F(ERROR, "JSON数据类型错误 - %s", e.what());
            return;
        } catch (const std::exception &e) {
            LOG_F(ERROR, "发生未知异常 - %s", e.what());
            return;
        }

        int code = j["code"];
        std::string msg = j["msg"];

        LOG_F(INFO, "状态码：%d", code);
        LOG_F(INFO, "提示消息：%s", msg.c_str());

        // 验证发送状态
        if (code == 1000) {
            LOG_F(INFO, "发送成功\n");
        } else {
            LOG_F(ERROR, "发送失败\n");
        }
    }

    // 清理资源
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}