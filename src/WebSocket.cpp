#include "WebSocket.h"
#include <memory>
#include <boost/json.hpp>
#include "Services.h"
#include "Utilities.h"

void WebSocket::returnErrorPage(const std::shared_ptr<bwss::Connection>& connection, const ErrorPageOptions& errorPageOptions) {
  boost::json::object response;
  response["c"] = errorPageOptions.status;
  response["m"] = errorPageOptions.message;
  response["t"] = Utilities::getCurrentEpoch();
  response["s"] = 0;

  connection->send(boost::json::serialize(response), bwss::OpCodes::TEXT_FRAME);
}

void WebSocket::onOpen(std::weak_ptr<bwss::Connection> connectionPtr) {
  std::shared_ptr<bwss::Connection> connection = connectionPtr.lock();
  if (!connection) {
    return;
  }

  // Attach socket data
  SocketData data;
  connection->userData = std::make_shared<SocketData>(data);
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
    connection->send("Invalid data", bwss::OpCodes::TEXT_FRAME);
    return;
  }

  // Prase as object, check if object contains service
  boost::json::object& messageObject = messageValue.get_object();

  boost::json::value* servicePtr = messageObject.if_contains("s");
  if (!servicePtr || !servicePtr->is_int64()) {
    connection->send("Invalid data", bwss::OpCodes::TEXT_FRAME);
    return;
  }

  Services::handle(std::move(connection), std::move(messageObject), servicePtr->get_int64());
}

void WebSocket::onClose(std::weak_ptr<bwss::Connection> connectionPtr) {
  // 🤷 Idk if we need to do anything here
}