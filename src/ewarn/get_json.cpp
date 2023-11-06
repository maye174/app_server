#include <cstring>
#include <curl/curl.h>
#include <loguru.hpp>
#include <string>

static size_t write_callback(void *contents, size_t size, size_t nmemb,
                             void *userp) {
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

std::string load_builds_json() {
    CURL *curl;
    CURLcode res;
    std::string read_buffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {

        curl_easy_setopt(
            curl, CURLOPT_URL,
            "https://card.aqnu.edu.cn/epay/wxpage/wanxiao/getbuild.json");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS,
                         "sysid=1&areaid=1&districtid=1");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);

        // 取消SSL证书验证
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

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

std::string load_rooms_json(const std::string &buildid) {

    if (buildid.empty()) {
        return "";
    }

    CURL *curl;
    CURLcode res;
    std::string read_buffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        std::string str =
            "sysid=1&areaid=1&districtid=1&buildid=" + buildid + "&floorid=1";

        curl_easy_setopt(
            curl, CURLOPT_URL,
            "https://card.aqnu.edu.cn/epay/wxpage/wanxiao/getroom.json");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, str.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);

        // 取消SSL证书验证
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

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