
#pragma once

#include <string>
#include <tuple>
#include <vector>

void api_user_attention_callback(struct evhttp_request *req, void *arg);

std::vector<std::tuple<std::string, std::string>>
query_data_by_appid(int appid);