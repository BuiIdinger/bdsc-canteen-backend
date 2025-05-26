#pragma once

#include <string>
#include <bwss/bwss.h>
#include <memory>

struct SocketData {
  std::string token;
  int16_t authenticationLevel = 0;
};

namespace WebSocket {
  void onOpen(std::weak_ptr<bwss::Connection> connectionPtr);
  void onMessage(std::weak_ptr<bwss::Connection> connectionPtr, std::string message);
  void onClose(std::weak_ptr<bwss::Connection> connectionPtr);

  struct ErrorPageOptions {
    int32_t status = 200;
    std::string message = "Success.";
  };

  void returnErrorNotification(const std::shared_ptr<bwss::Connection>& connection, const ErrorPageOptions& errorPageOptions);
} // namespace WebSocket