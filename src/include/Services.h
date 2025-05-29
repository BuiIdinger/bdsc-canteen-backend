#pragma once

#include <unordered_map>
#include <functional>
#include <boost/json.hpp>
#include <memory>

// Imported services for service handler map
#include "Signup.h"

namespace Services {
  void handle(std::shared_ptr<bwss::Connection> connection, boost::json::object message, uint64_t service);
} // namespace Services

using ServiceHandler = std::function<void(std::shared_ptr<bwss::Connection> connection, boost::json::object)>;

// A list of services
inline static std::unordered_map<int, ServiceHandler> serviceMap = {
  {2, &Services::Authentication::Signup::signup},
};