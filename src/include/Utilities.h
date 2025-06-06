#pragma once

#include <string>
#include <utility>
#include <boost/json.hpp>
#include <bwss/bwss.h>
#include <memory>

namespace Utilities {
  int64_t getCurrentEpoch();
  std::pair<bool, std::string> validatePassword(const std::string& password);
  std::pair<bool, std::string> validateEmail(const std::string& email);
  std::pair<bool, std::string> validateName(const std::string& name);

  // Changes a page on the client side
  void changePage(const std::shared_ptr<bwss::Connection>& connection, int32_t service, const std::string& page);

  namespace CallbackConnection {
    // Gets a connection ptr from a socket when in callbacks
    std::shared_ptr<bwss::Connection> get(const int& socket);
    void insert(int socket, std::shared_ptr<bwss::Connection> connection);
    void erase(const int& socket);
  }

  std::string generateRandomBytes(const size_t& size);
  std::string generateAuthenticationToken();

  namespace Password {
    std::pair<bool, std::string> hash(const std::string& password);
    bool verify(const std::string& storedPassword, const std::string& inputPassword);
  }
}