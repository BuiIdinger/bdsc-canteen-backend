#include <string>
#include "Utilities.h"
#include <utility>
#include <regex>

std::pair<bool, std::string> Utilities::vaildatePassword(const std::string& password) {
  if (password.size() < 3) {
    return {false, "Password must be longer than 3 charters."};
  }
  if (password.size() > 200) {
    return {false, "Password must be shorter than 200 charters."};
  }
  if (password.empty()) {
    return {false, "Password must not be empty."};
  }

  return {true, ""};
}

std::pair<bool, std::string> Utilities::vaildateEmail(const std::string& email) {
  const std::regex pattern(R"(^(([^<>()\[\]\\.,;:\s@"]+(\.[^<>()\[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$)");

  if (std::regex_match(email, pattern)) {
    return {true, ""};
  } else {
    return {false, "Email is invalid."};
  }
}