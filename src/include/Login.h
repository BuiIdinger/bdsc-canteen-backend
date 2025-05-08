#pragma once

#include <boost/json.hpp>
#include <memory>
#include "WebSocket.h"

namespace Services {
  void login(std::shared_ptr<bwss::Connection> connection, boost::json::object message);
} // namespace Services