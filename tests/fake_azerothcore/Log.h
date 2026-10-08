#pragma once
#include <string>

void FakeLog(std::string const& severity, std::string const& message);

#define LOG_INFO(category, message) FakeLog("INFO", message)
#define LOG_WARN(category, message) FakeLog("WARN", message)
