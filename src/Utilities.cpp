#include <string>
#include "Utilities.h"
#include <utility>
#include <regex>
#include <chrono>
#include <boost/json.hpp>
#include <bwss/bwss.h>
#include "WebSocket.h"
#include <memory>
#include <sstream>
#include <random>
#include <iomanip>
#include <vector>

int64_t Utilities::getCurrentEpoch() {
  return std::chrono::system_clock::now().time_since_epoch().count();
}

std::pair<bool, std::string> Utilities::validatePassword(const std::string& password) {
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

std::pair<bool, std::string> Utilities::validateEmail(const std::string& email) {
  // This regex was AI created because nobody knows how to do regex's
  const std::regex regex(R"(^(([^<>()\[\]\\.,;:\s@"]+(\.[^<>()\[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$)");

  if (std::regex_match(email, regex)) {
    return {true, ""};
  }

  return {false, "Email is invalid."};
}

std::pair<bool, std::string> Utilities::validateName(const std::string& name) {
  const std::regex regex("^[A-Za-z]+(?:[\\s'-][A-Za-z]+)*$");

  if (std::regex_match(name, regex)) {
    return {true, ""};
  } else {
    return {false, "Name is invalid."};
  }
}

void Utilities::changePage(const std::shared_ptr<bwss::Connection>& connection, const int32_t service, const std::string& page) {
  boost::json::object message;
  boost::json::object messageData;

  message["s"] = service;
  message["p"] = page;

  connection->send(boost::json::serialize(message), bwss::OpCodes::TEXT_FRAME);
}

std::shared_ptr<bwss::Connection> Utilities::CallbackConnection::get(const int &socket) {
  const auto it = callbackConnections.find(socket);
  if (it == callbackConnections.end()) {
    return nullptr;
  }

  callbackConnections.erase(socket);

  return it->second.lock();
}

void Utilities::CallbackConnection::insert(int socket, std::shared_ptr<bwss::Connection> connection) {
  callbackConnections.insert({ socket, std::move(connection) });
}

void Utilities::CallbackConnection::erase(const int& socket) {
  callbackConnections.erase(socket);
}

static std::string generateRandomBytes(const size_t& size) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<> dis(0, 255);

  std::vector<unsigned char> random_bytes(size);
  for (auto& byte : random_bytes) {
    byte = static_cast<unsigned char>(dis(gen));
  }

  std::ostringstream oss;
  for (const auto& byte : random_bytes) {
    oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
  }

  return oss.str();
}

std::string Utilities::generateAuthenticationToken() {
  std::string authenticationToken;
  authenticationToken += "BUILDINGER_AUTH";
  authenticationToken += "_";
  authenticationToken += generateRandomBytes(128);

  return std::move(authenticationToken);
}