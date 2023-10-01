

#pragma once

#include <string>
#include <vector>

void wxpusher_send_message(const std::string &appToken,
                           const std::string &content,
                           const std::vector<std::string> &uids);