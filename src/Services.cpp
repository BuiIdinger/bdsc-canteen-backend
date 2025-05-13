#include "Services.h"
#include "WebSocket.h"
#include <boost/json.hpp>
#include <memory>

void Services::handle(std::shared_ptr<bwss::Connection> connection, boost::json::object message, const int64_t service) {
  // Lookup service
  auto it = serviceMap.find(service);
  if (it == serviceMap.end()) {
    connection->send("Invalid data", bwss::OpCodes::TEXT_FRAME);
    return;
  }

  // Invoke service handlers
  it->second(std::move(connection), std::move(message));
}