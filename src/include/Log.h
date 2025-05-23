#pragma once

#include <memory>
#include <functional>
#include <bwss/bwss.h>
#include <string>
#include <source_location>

namespace Log {
  enum Categories {
    CRITICAL,
    ERROR,
    WARN,
    INFO,
    DATABASE_CRITICAL,
    DATABASE_ERROR,
    DATABASE_WARN,
    DATABASE_INFO,
  };

  const std::unordered_map<Categories, std::string> categoriesMap = {
    {Categories::CRITICAL, "[CRITICAL]"},
    {Categories::ERROR, "[ERROR]"},
    {Categories::WARN, "[WARN]"},
    {Categories::INFO, "[INFO]"},
    {Categories::DATABASE_CRITICAL, "[DATABASE_CRITICAL]"},
    {Categories::DATABASE_ERROR, "[DATABASE_ERROR]"},
    {Categories::DATABASE_WARN, "[DATABASE_WARN]"},
    {Categories::DATABASE_INFO, "[DATABASE_INFO]"},
  };

  /**
   * @brief Logs a critical message, and shutdowns the server
   */
  void critical(const std::string& message, const std::source_location& location = std::source_location::current());

  /**
   * @brief Logs an error message
   */
  void error(const std::string& message, const std::source_location& location = std::source_location::current());

  /**
   * @brief Logs a warning message
   */
  void warn(const std::string& message, const std::source_location& location = std::source_location::current());

  /**
   * @brief Logs an info message
   */
  void info(const std::string& message, const std::source_location& location = std::source_location::current());

  /**
   * Logs related to database operations
   */
  namespace Database {
    /**
     * @brief Logs a critical error message, and shutdowns the server
     */
    void critical(const std::string& message, const std::source_location& location = std::source_location::current());

    /**
     * @brief Logs an error message, a callback can be provided and is automatically
     * invoked after the message is logged
     */
    void error(const std::string& message, const std::function<void()>& callback, const std::source_location& location = std::source_location::current());

    /**
     * @brief Logs an error message
     */
    void error(const std::string& message, const std::source_location& location = std::source_location::current());

    /**
     * @brief Logs a warning message
     */
    void warn(const std::string& message, const std::source_location& location = std::source_location::current());

    /**
     * @brief Logs an info message
     */
    void info(const std::string& message, const std::source_location& location = std::source_location::current());
  }

  /**
   * @brief Forms an output message to log
   * @returns The formed message
   */
  std::string formMessage(const Categories& category, const std::string& message, const std::source_location& location = std::source_location::current());

  /**
   * @brief Logs a message to the log file
   */
  void output(const std::string& message);
}
