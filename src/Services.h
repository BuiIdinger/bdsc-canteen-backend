#pragma once

#include <unordered_map>
#include <functional>
#include <boost/json.hpp>

using ServiceHandler = std::function<void(const boost::json::object&)>;

// A list of services
inline static std::unordered_map<int, ServiceHandler> serviceMap = {
  // {1,  [](const boost::json::object& obj) {}},
};

namespace Services {
  void handle(const boost::json::object message, const int64_t service);
} // namespace Services