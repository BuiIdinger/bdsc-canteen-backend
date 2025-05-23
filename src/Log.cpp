#include "Log.h"
#include <iostream>
#include "Main.h"
#include <source_location>
#include <utility>

/**
 * Regular logs
 */
void Log::critical(const std::string& message, const std::source_location& location) {
  output(formMessage(Categories::CRITICAL, message, location));
  shutdown(EXIT_FAILURE);
}

void Log::error(const std::string& message, const std::source_location& location) {
  output(formMessage(Categories::ERROR, message, location));
}

void Log::warn(const std::string& message, const std::source_location& location) {
  output(formMessage(Categories::WARN, message, location));
}

void Log::info(const std::string& message, const std::source_location& location) {
  output(formMessage(Categories::INFO, message, location));
}

/**
 * Database logs
 */
void Log::Database::critical(const std::string &message, const std::source_location &location) {
  output(formMessage(Categories::DATABASE_CRITICAL, message, location));
  shutdown(EXIT_FAILURE);
}

void Log::Database::error(const std::string &message, const std::source_location &location) {
  output(formMessage(Categories::DATABASE_ERROR, message, location));
}

void Log::Database::error(const std::string& message, const std::function<void()>& callback, const std::source_location& location) {
  output(formMessage(Categories::DATABASE_ERROR, message, location));

  if (callback) {
    callback();
  }
}

void Log::Database::warn(const std::string &message, const std::source_location &location) {
  output(formMessage(Categories::DATABASE_WARN, message, location));
}

void Log::Database::info(const std::string &message, const std::source_location &location) {
  output(formMessage(Categories::DATABASE_INFO, message, location));
}


/**
 * Helper function
 */
// Forms the message to be used for outputting into the log file
std::string Log::formMessage(const Categories& category, const std::string& message, const std::source_location& location) {
  std::string output;
  output += categoriesMap.find(category)->second;

  // Form message
  output += " ";
  output += message;
  output += " ";

  // Form line and column
  output += "(";
  output += std::to_string(location.line());
  output += ":";
  output += std::to_string(location.column());
  output += ") ";

  output += "\n";

  return std::move(output);
}

void Log::output(const std::string& message) {
  std::cerr << message << std::endl;
}