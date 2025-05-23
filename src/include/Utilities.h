#pragma once

#include <string>
#include <utility>
#include <boost/json.hpp>
#include <bwss/bwss.h>

namespace Utilities {
  int64_t getCurrentEpoch();
  std::pair<bool, std::string> validatePassword(const std::string& password);
  std::pair<bool, std::string> validateEmail(const std::string& email);
  std::pair<bool, std::string> validateName(const std::string& name);
  void revertPage(const std::shared_ptr<bwss::Connection>& connection, int32_t service, const std::string& page);
}