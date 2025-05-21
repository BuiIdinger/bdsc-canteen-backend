#pragma once

#include <string>
#include <utility>

namespace Utilities {
  std::pair<bool, std::string> vaildatePassword(const std::string& password);
  std::pair<bool, std::string> vaildateEmail(const std::string& email);
}