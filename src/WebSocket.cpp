#include "WebSocket.h"
#include <memory>
#include <boost/json.hpp>
#include <iostream>
#include "Services.h"

void WebSocket::onOpen(std::weak_ptr<bwss::Connection> connectionPtr) {
  std::shared_ptr<bwss::Connection> connection = connectionPtr.lock();
  if (!connection) {
    return;
  }

  // Attach socket data
  SocketData data;
  connection->userData = std::make_shared<SocketData>(data);
  // std::shared_ptr<SocketData> userData = connection->getUserData<SocketData>();
}

void WebSocket::onMessage(std::weak_ptr<bwss::Connection> connectionPtr, std::string message) {
  std::shared_ptr<bwss::Connection> connection = connectionPtr.lock();
  if (!connection) {
    return;
  }

  // Prase message
  boost::system::error_code praseError;
  boost::json::value messageValue = boost::json::parse(message, praseError);
  if (praseError || !messageValue.is_object()) {
    connection->send("Invaild data", bwss::OpCodes::TEXT_FRAME);
    return;
  }

  // Prase as object, check if object contains service
  boost::json::object const& messageObject = messageValue.get_object();
  if (!messageObject.contains("s") || !messageObject.at("s").is_int64()) {
    connection->send("Invaild data", bwss::OpCodes::TEXT_FRAME);
    return;
  }

  Services::handle(std::move(messageObject), messageObject.at("s").get_int64());
}

void WebSocket::onClose(std::weak_ptr<bwss::Connection> connectionPtr) {
  // 🤷 Idk if we need to do anything here
}