

#include "ewarn/inc/task.hpp"
#include "server/inc/start.hpp"
#include "server/inc/util.hpp"
#include "wxpusher/inc/data.hpp"
#include "wxpusher/inc/send.hpp"

#include <map>
#include <string>
#include <tuple>
#include <vector>

#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>
#include <loguru.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

using list = std::vector<std::tuple<std::string, std::string>>;

// 抓包
static std::string http_request_done(const std::string &_suffix) {

    LOG_F(INFO, "%s", _suffix.c_str());

    std::string suffix = "sysid=1&areaid=1" + _suffix;

    std::string url =
        "http://218.22.140.88:8181/epay/wxpage/wanxiao/eleresult?" + suffix;

    std::string content = fetch_url(url);

    std::string target = "度";
    size_t pos = content.find(target);

    std::string number;
    size_t start = pos;

    if (pos == std::string::npos) {
        LOG_F(ERROR, "ewarn http_request_done: 未找到目标字符串");
        return "";
    }

    // 读取度前面的数字，包括小数点和负号
    while (start > 0 &&
           (isdigit(content[start - 1]) || content[start - 1] == '.' ||
            content[start - 1] == '-')) {
        --start;
    }
    number = content.substr(start, pos - start);

    return number;
}

void ew_timer_task(evutil_socket_t fd, short events, void *arg) {
    LOG_F(INFO, "ew_timer_task");

    list l = query_data_by_appid(62383);

    std::map<std::string, std::vector<std::string>> m;

    // 抓包然后发送
    for (auto &i : l) {
        std::string number = http_request_done(std::get<1>(i));

        if (number.empty() || std::stof(number) > 20.0)
            continue;

        // LOG_F(INFO, "低于20度: %s", number.c_str());

        if (m.find(std::get<1>(i)) == m.end())
            m[std::get<1>(i)] = std::vector<std::string>();
        m[std::get<1>(i)].emplace_back(std::get<0>(i));
    }

    for (auto &[k, v] : m) {
        LOG_F(INFO, "发送寝室号: %s", k.c_str());

        wxpusher_send_message(get_someting_from_env("WXPUSHER_APP_EW_TOKEN"),
                              "你的电费不足20度", v);
    }

    auto d = (timer_data *)arg;
    event_add(d->ev, &d->interval);
}