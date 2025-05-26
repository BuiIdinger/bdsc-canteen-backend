#pragma once

#include <string>
#include <bwss/bwss.h>
#include <memory>
#include <unordered_map>

struct SocketData {
  std::string token;
  int16_t authenticationLevel = 0;
};

/*
 * We need this map, as casting to a raw void pointer is unsafe, also
 * I cannot modify the callbacks functions definition
 *
 * We use the fd of the connection as a key, and the weak pointer as
 * the value
 */
inline std::unordered_map<int, std::weak_ptr<bwss::Connection>> callbackConnections;

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