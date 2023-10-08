

#include "http/inc/http.hpp"

#include <cstdlib>

#include <curl/curl.h>
#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>
#include <loguru.hpp>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

static size_t write_callback(void *contents, size_t size, size_t nmemb,
                             void *userp) {
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

std::string http_fetch(const std::string &http_json) {
    json j;
    try {
        j = json::parse(http_json);
    } catch (const json::parse_error &e) {
        LOG_F(ERROR, "解析JSON数据时发生异常 - %s", e.what());
    } catch (const json::type_error &e) {
        LOG_F(ERROR, "JSON数据类型错误 - %s", e.what());
    } catch (const std::exception &e) {
        LOG_F(ERROR, "发生未知异常 - %s", e.what());
    }
    return http_fetch(j);
}

std::string http_fetch(json &http_json) {

    CURL *curl;
    CURLcode res;
    std::string read_buffer;

    if (!http_json.contains("uri") || !http_json.contains("method")) {
        LOG_F(ERROR, "未找到uri(uri) 或 method");
        curl_global_cleanup();
        return "";
    }

    std::string url = http_json["uri"].get_ref<std::string &>();
    if (http_json.contains("params")) {
        json params = http_json["params"];
        url += "?";
        for (auto &i : params.items()) {
            url += i.key();
            url += "=";
            url += i.value().get_ref<std::string &>();
            url += "&";
        }
        url.pop_back();
    }

    std::string method = http_json["method"].get_ref<std::string &>();

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);

        if (http_json.contains("headers"))
            for (auto &i : http_json["headers"].items()) {
                std::string header = i.key();
                header += ": ";
                header += i.value().get_ref<std::string &>();
                struct curl_slist *chunk = NULL;
                chunk = curl_slist_append(chunk, header.c_str());
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
            }

        if (http_json.contains("body")) {
            std::string body = http_json["body"].get_ref<std::string &>();
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        }

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            LOG_F(ERROR, "curl_easy_perform() failed: %s",
                  curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return read_buffer;
}

static std::string method_parser(evhttp_cmd_type method) {

    std::string method_str;

    switch (method) {
    case EVHTTP_REQ_GET:
        method_str = "GET";
        break;
    case EVHTTP_REQ_POST:
        method_str = "POST";
        break;
    case EVHTTP_REQ_HEAD:
        method_str = "HEAD";
        break;
    case EVHTTP_REQ_PUT:
        method_str = "PUT";
        break;
    case EVHTTP_REQ_DELETE:
        method_str = "DELETE";
        break;
    case EVHTTP_REQ_OPTIONS:
        method_str = "OPTIONS";
        break;
    case EVHTTP_REQ_TRACE:
        method_str = "TRACE";
        break;
    case EVHTTP_REQ_CONNECT:
        method_str = "CONNECT";
        break;
    case EVHTTP_REQ_PATCH:
        method_str = "PATCH";
        break;
    default:
        method_str = "Unknown";
    }

    return method_str;
}

json get_http_json(struct evhttp_request *request) {

    json http_json;

    // 获取请求的方法
    evhttp_cmd_type method = evhttp_request_get_command(request);
    http_json["method"] = method_parser(method);

    // 获取请求的 URI
    const char *uri = evhttp_request_get_uri(request);
    http_json["uri"] = uri;

    // 获取请求的 URI 参数, 如果有
    struct evkeyvalq uri_params;
    evhttp_parse_query(uri, &uri_params);
    json param_json;
    for (struct evkeyval *param = uri_params.tqh_first; param;
         param = param->next.tqe_next) {
        param_json[param->key] = param->value;
    }
    http_json["params"] = param_json;

    // 获取请求的 HTTP 报头
    struct evkeyvalq *headers = evhttp_request_get_input_headers(request);
    json header_json;
    for (struct evkeyval *header = headers->tqh_first; header;
         header = header->next.tqe_next) {
        header_json[header->key] = header->value;
    }
    http_json["headers"] = header_json;

    if (method == EVHTTP_REQ_POST || method == EVHTTP_REQ_PUT ||
        method == EVHTTP_REQ_PATCH) {
        // 获取请求的 body
        struct evbuffer *buf = evhttp_request_get_input_buffer(request);
        size_t len = evbuffer_get_length(buf);
        char *body = (char *)malloc(len + 1);
        evbuffer_copyout(buf, body, len);
        body[len] = '\0';
        http_json["body"] = body;
        free(body);
    }

    evhttp_clear_headers(&uri_params);

    return http_json;
}

std::string get_http_json_string(struct evhttp_request *request) {
    return get_http_json(request).dump();
}