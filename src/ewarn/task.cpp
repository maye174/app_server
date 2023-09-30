

#include "ewarn/inc/task.hpp"
#include "server/inc/util.hpp"
#include "wxpusher/inc/data.hpp"

#include <string>
#include <tuple>
#include <vector>

#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>
#include <loguru.hpp>

using list = std::vector<std::tuple<std::string, std::string>>;

// 抓包
std::string http_request_done(std::string &suffix) {
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

std::string find_roomid(std::string &);

void ew_timer_task(evutil_socket_t fd, short events, void *arg) {
    printf("ew_timer_task\n");

    list l = query_data_by_appid(62383);

    // 抓包然后发送
    for (auto &i : l) {
        ;
    }

    struct event *ev = (struct event *)arg;
    struct timeval tv;
    evutil_timerclear(&tv);
    tv.tv_sec = 10800;
    event_add(ev, &tv);
}