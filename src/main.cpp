//
// Create by github:Maye174
// 2023/9/25
//

#include "inc/start.hpp"

#include <iostream>

#ifdef _WIN32
#include <winsock2.h>
#endif

std::string html_content = "<!DOCTYPE html>\
<html lang=\"zh-CN\">\
\
<head>\
    <meta charset=\"UTF-8\">\
    <title>a</title>\
    <script src=\"https://cdn.jsdelivr.net/npm/qrcode@1.4.4/build/qrcode.min.js\"></script>\
    <script>\
        async function createQRCode(event) {\
            event.preventDefault();\
\
            const building_number = document.getElementById('building_number').value;\
            const room_number = document.getElementById('room_number').value;\
\
            const response = await fetch('/api/ew_creata_qrcode', {\
                method: 'POST',\
                headers: {\
                    'Content-Type': 'application/json'\
                },\
                body: JSON.stringify({ building_number, room_number })\
            });\
\
            const data = await response.json();\
            const qrcode_url = data.url;\
            const qrcode_element = document.getElementById('qrcode');\
            qrcode_element.innerHTML = `<img src=\"${qrcode_url}\" alt=\"QR Code\">`;\
        }\
    </script>\
</head>\
\
<body>\
    <form onsubmit=\"createQRCode(event)\">\
        <label for=\"building_number\">a:</label>\
        <input type=\"text\" id=\"building_number\" required>\
        <label for=\"room_number\">b:</label>\
        <input type=\"text\" id=\"room_number\" required>\
        <button type=\"submit\">bu</button>\
    </form>\
    <div id=\"qrcode\"></div>\
</body>\
\
</html>";

static void http_request_handler(struct evhttp_request *req, void *arg) {
    const char *uri = evhttp_request_get_uri(req);
    std::cout << "Requested URI: " << uri << std::endl;

    struct evbuffer *buf = evbuffer_new();
    if (!buf) {
        std::cerr << "Failed to create response buffer" << std::endl;
        return;
    }

    evbuffer_add(buf, html_content.c_str(), html_content.size());

    evhttp_send_reply(req, 200, "OK", buf);
    evbuffer_free(buf);
}

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    struct event_base *base = event_base_new();
    if (!base) {
        std::cerr << "Failed to create event base\n";
        return 1;
    }

    struct evhttp *http = evhttp_new(base);
    if (!http) {
        std::cerr << "Failed to create HTTP server\n";
        return 1;
    }

    evhttp_set_gencb(http, http_request_handler, nullptr);
    register_callback(http);

    if (evhttp_bind_socket(http, "0.0.0.0", 3000) != 0) {
        std::cerr << "Failed to bind to port 3000\n";
        return 1;
    }

    while (1) {
        event_base_loop(base, EVLOOP_NONBLOCK);
        Sleep(100);
    }

    evhttp_free(http);
    event_base_free(base);

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}