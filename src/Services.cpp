#include "Services.h"
#include "WebSocket.h"
#include <boost/json.hpp>
#include <memory>
#include <limits>

void Services::handle(std::shared_ptr<bwss::Connection> connection, boost::json::object message, const uint64_t service) {
  if (service > std::numeric_limits<uint32_t>::max()) {
    connection->send("Invalid data", bwss::OpCodes::TEXT_FRAME);
    return;
  }

  // Lookup service
  const auto it = serviceMap.find(static_cast<int32_t>(service));
  if (it == serviceMap.end()) {
    connection->send("Invalid data", bwss::OpCodes::TEXT_FRAME);
    return;
  }

  // Invoke service handlers
  it->second(std::move(connection), std::move(message));
}