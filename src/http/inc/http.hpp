
#pragma once

// #include <functional>
#include <string>

#include <event2/event.h>
#include <event2/http.h>
#include <nlohmann/json.hpp>

// typedef struct {
//     int id;
//     struct event_base *base;
//     struct evhttp *ev;
// } http_data;

std::string http_fetch(const std::string &http_json);

std::string http_fetch(nlohmann::json &http_json);

nlohmann::json get_http_json(struct evhttp_request *request);

std::string get_http_json_string(struct evhttp_request *request);