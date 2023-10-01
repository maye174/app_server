

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
static std::string http_request_done(const std::string &j_s) {

    LOG_F(INFO, "%s", j_s.c_str());

    json j;

    try {
        j = json::parse(j_s);
    } catch (const json::parse_error &e) {
        LOG_F(ERROR, "解析JSON数据时发生异常 - %s", e.what());
    } catch (const json::type_error &e) {
        LOG_F(ERROR, "JSON数据类型错误 - %s", e.what());
    } catch (const std::exception &e) {
        LOG_F(ERROR, "发生未知异常 - %s", e.what());
    }

    if (!j.contains("b") || !j.contains("r")) {
        LOG_F(ERROR, "未找到building_number 或者 "
                     "room_number\n 400");
        return "";
    }

    std::string building_number = j["b"].get_ref<std::string &>();
    std::string room_number = j["r"].get_ref<std::string &>();

    std::string suffix = "sysid=1&roomid=" + room_number +
                         "&areaid=1&buildid=" + building_number;

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

    while (start > 0 &&
           (isdigit(content[start - 1]) || content[start - 1] == '.')) {
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
        LOG_F(INFO, "%s", std::get<1>(i).c_str());

        std::string number = http_request_done(std::get<1>(i));

        if (number.empty() || std::stoi(number) /*< 20*/)
            continue;

        if (m.find(std::get<1>(i)) == m.end())
            m[std::get<1>(i)] = std::vector<std::string>();
        m[std::get<1>(i)].emplace_back(std::get<0>(i));
    }

    for (auto &[k, v] : m) {
        wxpusher_send_message(get_someting_from_env("WXPUSHER_APP_EW_TOKEN"),
                              "你的电费不足20度", v);
    }

    auto d = (timer_data *)arg;
    event_add(d->ev, &d->interval);
}