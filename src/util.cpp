
#include "inc/util.hpp"

#include <cstdlib> // 添加此头文件以使用getenv函数
#include <iostream>

#include <curl/curl.h>

std::string get_someting_from_env(const char *str) {
    const char *appToken = std::getenv(str);
    if (appToken) {
        return std::string(appToken);
    } else {
        std::cerr << "APP_TOKEN environment variable not found\n";
        return "";
    }
}

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

std::string fetch_url(const std::string &url) {

    CURL *curl;
    CURLcode res;
    std::string read_buffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: "
                      << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return read_buffer;
}