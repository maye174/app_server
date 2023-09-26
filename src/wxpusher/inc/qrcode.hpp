#pragma once

#include <string>

std::string create_qrcode(const std::string &appToken, const std::string &extra,
                          int validTime);