#include "Signup.h"
#include <memory>
#include <bwss/bwss.h>
#include <boost/json.hpp>
#include "WebSocket.h"
#include "Utilities.h"
#include <cassandra.h>
#include "Database.h"
#include "Log.h"
#include "Main.h"

namespace Extract {
  static std::pair<bool, std::string> password(const boost::json::object& message) {
    const boost::json::value* passwordValue = message.if_contains("p");
    if (!passwordValue || !passwordValue->is_string()) {
      return {false, "Password doesn't exist in the object or isn't type string."};
    }

    const std::string password = passwordValue->get_string().c_str();

    const auto passwordResult = Utilities::validatePassword(password);
    if (!passwordResult.first) {
      return {false, passwordResult.second};
    }

    return {true, password};
  }

  static std::pair<bool, std::string> email(const boost::json::object& message) {
    const boost::json::value* emailValue = message.if_contains("e");
    if (!emailValue || !emailValue->is_string()) {
      return {false, "Email isn't present in object or type isn't a string."};
    }

    const std::string email = emailValue->get_string().c_str();

    const auto emailResult = Utilities::validateEmail(email);
    if (!emailResult.first) {
      return {false, emailResult.second};
    }

    return {true, email};
  }

  static std::pair<bool, std::string> firstName(const boost::json::object& message) {
    const boost::json::value* firstNameValue = message.if_contains("fn");
    if (!firstNameValue || !firstNameValue->is_string()) {
      return {false, "First name isn't present in the object or type isn't string."};
    }

    const std::string firstName = firstNameValue->get_string().c_str();

    const auto nameResult = Utilities::validateName(firstName);
    if (!nameResult.first) {
      return {false, nameResult.second};
    }

    return {true, firstName};
  }

  static std::pair<bool, std::string> lastName(const boost::json::object& message) {
    const boost::json::value* lastNameValue = message.if_contains("ln");
    if (!lastNameValue || !lastNameValue->is_string()) {
      return {false, "Last name isn't present in the object or type isn't string."};
    }

    const std::string lastName = lastNameValue->get_string().c_str();

    const auto nameResult = Utilities::validateName(lastName);
    if (!nameResult.first) {
      return {false, nameResult.second};
    }

    return {true, lastName};
  }

  static std::pair<bool, std::variant<bool, std::string>> marketingEmails(const boost::json::object& message) {
    const boost::json::value* marketingEmailsValue = message.if_contains("me");
    if (!marketingEmailsValue || !marketingEmailsValue->is_bool()) {
      return {false, "Marketing emails isn't present in the object or isn't type bool."};
    }

    const bool marketingEmails = marketingEmailsValue->get_bool();
    return { true, marketingEmails };
  }
}

void Services::Authentication::Signup::prepareStatements() {
  Database::prepareStatement(selectFromEmailLookup, "SELECT id FROM users_by_email WHERE email = ?;");
  Database::prepareStatement(insertIntoUsersTable, "INSERT INTO users (id, email, password, first_name, last_name, marketing_emails, authentication_token,     created_at, points) VALUES (?, ?, ?, ?, ?, ?, ?,     toTimestamp(now()), 999999)");
  Database::prepareStatement(insertIntoEmailLookupTable, "INSERT INTO users_by_email (email, id) VALUES (?, ?);");
}

void Services::Authentication::Signup::Callbacks::insertIntoEmailLookup(CassFuture* future, void* data) {
  const std::unique_ptr<CallbackData::InsertIntoEmailLookup> callbackData(static_cast<CallbackData::InsertIntoEmailLookup*>(data));

  std::shared_ptr<bwss::Connection> connection = Utilities::CallbackConnection::get(callbackData->socket);
  if (!connection) {
    return;
  }

  std::unique_ptr<const CassResult, void(*)(const CassResult*)> result(nullptr, cass_result_free);

  if (cass_future_error_code(future) != CASS_OK) {
    const char* errorMessage;
    size_t errorMessageSize;
    cass_future_error_message(future, &errorMessage, &errorMessageSize);
    Log::Database::error(
     std::string(errorMessage, errorMessageSize),
     [connection]() {
       WebSocket::sendNotification(connection, {
         WebSocket::StatusCodes::INTERNAL_SERVER_ERROR,
         ERROR_OCCURRED}
        );
       Utilities::changePage(connection, static_cast<int32_t>(ServicesIdent::AUTHENTICATION_MODEL_PAGE_CHANGE), pages.find(Pages::SIGNUP_CREATE_ACCOUNT)->second);
     });
    return;
  }

  result.reset(cass_future_get_result(future));
  if (!result) {
    Log::Database::error(
    "No result returned from query.",
    [connection]() {
      WebSocket::sendNotification(connection, {
         WebSocket::StatusCodes::INTERNAL_SERVER_ERROR,
         ERROR_OCCURRED}
      );
      Utilities::changePage(connection, static_cast<int32_t>(ServicesIdent::AUTHENTICATION_MODEL_PAGE_CHANGE), pages.find(Pages::SIGNUP_CREATE_ACCOUNT)->second);
    });
    return;
  }

  /*
   * Success, send startup data
   */
  char tmpId[CASS_UUID_STRING_LENGTH];
  cass_uuid_string(callbackData->id, tmpId);

  std::shared_ptr<SocketData> userData = connection->getUserData<SocketData>();
  userData->authenticationToken = std::move(callbackData->authenticationToken);
  userData->id = std::string(tmpId);
  userData->firstName = std::move(callbackData->firstName);
  userData->lastNName = std::move(callbackData->lastName);
  userData->email = std::move(callbackData->email);

  Utilities::changePage(connection, static_cast<int32_t>(ServicesIdent::AUTHENTICATION_MODEL_PAGE_CHANGE), pages.find(Pages::CLOSE)->second);

  /*
   * Send startup data
   */
  boost::json::object message;
  boost::json::object messageData;

  messageData["id"] = userData->id;
  messageData["fn"] = userData->firstName;
  messageData["lastName"] = userData->lastNName;
  messageData["e"] = userData->email;

  message["data"] = messageData;
  message["s"] = static_cast<int>(ServicesIdent::SET_STARTUP_USER_DATA);

  connection->send(boost::json::serialize(message), bwss::OpCodes::TEXT_FRAME);
}

void Services::Authentication::Signup::Callbacks::insertIntoUsers(CassFuture* future, void* data) {
  const std::unique_ptr<CallbackData::InsertIntoUsers> callbackData(static_cast<CallbackData::InsertIntoUsers*>(data));

  std::shared_ptr<bwss::Connection> connection = Utilities::CallbackConnection::get(callbackData->socket);
  if (!connection) {
    return;
  }

  std::unique_ptr<const CassResult, void(*)(const CassResult*)> result(nullptr, cass_result_free);

  if (cass_future_error_code(future) != CASS_OK) {
    const char* errorMessage;
    size_t errorMessageSize;
    cass_future_error_message(future, &errorMessage, &errorMessageSize);
    Log::Database::error(
     std::string(errorMessage, errorMessageSize),
     [connection]() {
       WebSocket::sendNotification(connection, {
         WebSocket::StatusCodes::INTERNAL_SERVER_ERROR,
         ERROR_OCCURRED}
       );
       Utilities::changePage(connection, static_cast<int32_t>(ServicesIdent::AUTHENTICATION_MODEL_PAGE_CHANGE), pages.find(Pages::SIGNUP_CREATE_ACCOUNT)->second);
     });
    return;
  }

  std::unique_ptr<CassStatement, void(*)(CassStatement*)> statement(nullptr, cass_statement_free);
  std::unique_ptr<CassFuture, void(*)(CassFuture*)> newFuture(nullptr, cass_future_free);

  statement.reset(cass_prepared_bind(insertIntoEmailLookupTable.get()));

  const CassError emailBindError = cass_statement_bind_string_by_name(statement.get(), "email", callbackData->email.c_str());
  if (emailBindError != CASS_OK) {
    Log::Database::error(
    cass_error_desc(emailBindError),
    [connection]() {
      WebSocket::sendNotification(connection, {
        WebSocket::StatusCodes::INTERNAL_SERVER_ERROR,
        ERROR_OCCURRED}
      );
      Utilities::changePage(connection, static_cast<int32_t>(ServicesIdent::AUTHENTICATION_MODEL_PAGE_CHANGE), pages.find(Pages::SIGNUP_CREATE_ACCOUNT)->second);
    });
    return;
  }

  const CassError idBindError = cass_statement_bind_uuid_by_name(statement.get(), "id", callbackData->id);
  if (idBindError != CASS_OK) {
    Log::Database::error(
    cass_error_desc(idBindError),
    [connection]() {
      WebSocket::sendNotification(connection, {
        WebSocket::StatusCodes::INTERNAL_SERVER_ERROR,
        ERROR_OCCURRED}
      );
      Utilities::changePage(connection, static_cast<int32_t>(ServicesIdent::AUTHENTICATION_MODEL_PAGE_CHANGE), pages.find(Pages::SIGNUP_CREATE_ACCOUNT)->second);
    });
    return;
  }

  newFuture.reset(cass_session_execute(Database::connection.get(), statement.get()));

  CallbackData::InsertIntoEmailLookup* newCallbackData = new CallbackData::InsertIntoEmailLookup;
  newCallbackData->socket = connection->socket;
  newCallbackData->authenticationToken = std::move(callbackData->authenticationToken);
  newCallbackData->id = std::move(callbackData->id);
  newCallbackData->firstName = std::move(callbackData->firstName);
  newCallbackData->lastName= std::move(callbackData->lastName);
  newCallbackData->email = std::move(callbackData->email);

  Utilities::CallbackConnection::insert(connection->socket, connection);

  const CassError setCallbackError = cass_future_set_callback(newFuture.get(), insertIntoEmailLookup, newCallbackData);
  if (setCallbackError != CASS_OK) {
    delete newCallbackData;
    newCallbackData = nullptr;
    Utilities::CallbackConnection::erase(connection->socket);
    Log::Database::error(
      cass_error_desc(setCallbackError),
      [connection]() {
        WebSocket::sendNotification(connection, {
          WebSocket::StatusCodes::INTERNAL_SERVER_ERROR,
          ERROR_OCCURRED}
        );
        Utilities::changePage(connection, static_cast<int32_t>(ServicesIdent::AUTHENTICATION_MODEL_PAGE_CHANGE), pages.find(Pages::SIGNUP_CREATE_ACCOUNT)->second);
    });
  }
}

void Services::Authentication::Signup::Callbacks::selectEmail(CassFuture* future, void* data) {
  const std::unique_ptr<CallbackData::SelectEmail> callbackData(static_cast<CallbackData::SelectEmail*>(data));

  std::shared_ptr<bwss::Connection> connection = Utilities::CallbackConnection::get(callbackData->socket);
  if (!connection) {
    return;
  }

  std::unique_ptr<const CassResult, void(*)(const CassResult*)> result(nullptr, cass_result_free);

  if (cass_future_error_code(future) != CASS_OK) {
    const char* errorMessage;
    size_t errorMessageSize;
    cass_future_error_message(future, &errorMessage, &errorMessageSize);
    Log::Database::error(
     std::string(errorMessage, errorMessageSize),
     [connection]() {
       WebSocket::sendNotification(connection, {
         WebSocket::StatusCodes::INTERNAL_SERVER_ERROR,
         ERROR_OCCURRED}
        );
       Utilities::changePage(connection, static_cast<int32_t>(ServicesIdent::AUTHENTICATION_MODEL_PAGE_CHANGE), pages.find(Pages::SIGNUP_CREATE_ACCOUNT)->second);
     });
    return;
  }

  result.reset(cass_future_get_result(future));
  if (!result) {
    Log::Database::error(
    "No result returned from query.",
    [connection]() {
      WebSocket::sendNotification(connection, {
         WebSocket::StatusCodes::INTERNAL_SERVER_ERROR,
         ERROR_OCCURRED}
      );
      Utilities::changePage(connection, static_cast<int32_t>(ServicesIdent::AUTHENTICATION_MODEL_PAGE_CHANGE), pages.find(Pages::SIGNUP_CREATE_ACCOUNT)->second);
    });
    return;
  }

  const CassRow* row = cass_result_first_row(result.get());
  if (row) {
    WebSocket::sendNotification(connection, {
      WebSocket::StatusCodes::INTERNAL_SERVER_ERROR,
      "It looks like you already have an account, sign-in instead."}
    );
    Utilities::changePage(connection, static_cast<int32_t>(ServicesIdent::AUTHENTICATION_MODEL_PAGE_CHANGE), pages.find(Pages::SIGNUP_PERSONAL_DETAILS)->second);
    return;
  }

  /*
   * We are good to proceed, a user wasn't found with this email
   *
   * Normally I would actually verify that this user owns the email address, but
   * I don't want to pay for somthing like Google Workspace in order to send
   * automated emails, plus it's just more extra work and the fact is I'm really not bothered
   * to work with Googles trash documentation and a stupid 600 request oAuth process
   */

  std::string authenticationToken = Utilities::generateAuthenticationToken();

  static std::unique_ptr<CassUuidGen, void(*)(CassUuidGen*)> uuidGenerator(cass_uuid_gen_new(), cass_uuid_gen_free);
  CassUuid id;
  cass_uuid_gen_random(uuidGenerator.get(), &id);

  std::unique_ptr<CassStatement, void(*)(CassStatement*)> statement(nullptr, cass_statement_free);
  std::unique_ptr<CassFuture, void(*)(CassFuture*)> newFuture(nullptr, cass_future_free);

  statement.reset(cass_prepared_bind(insertIntoUsersTable.get()));

  const CassError idBindError = cass_statement_bind_uuid_by_name(statement.get(), "id", id);
  if (idBindError != CASS_OK) {
    Log::Database::error(
    cass_error_desc(idBindError),
    [connection]() {
      WebSocket::sendNotification(connection, {
        WebSocket::StatusCodes::INTERNAL_SERVER_ERROR,
        ERROR_OCCURRED}
      );
      Utilities::changePage(connection, static_cast<int32_t>(ServicesIdent::AUTHENTICATION_MODEL_PAGE_CHANGE), pages.find(Pages::SIGNUP_CREATE_ACCOUNT)->second);
    });
    return;
  }

  const CassError emailBindError = cass_statement_bind_string_by_name(statement.get(), "email", callbackData->email.c_str());
  if (emailBindError != CASS_OK) {
    Log::Database::error(
    cass_error_desc(emailBindError),
    [connection]() {
      WebSocket::sendNotification(connection, {
        WebSocket::StatusCodes::INTERNAL_SERVER_ERROR,
        ERROR_OCCURRED}
      );
      Utilities::changePage(connection, static_cast<int32_t>(ServicesIdent::AUTHENTICATION_MODEL_PAGE_CHANGE), pages.find(Pages::SIGNUP_CREATE_ACCOUNT)->second);
    });
    return;
  }

  const CassError passwordBindError = cass_statement_bind_string_by_name(statement.get(), "password", callbackData->password.c_str());
  if (passwordBindError != CASS_OK) {
    Log::Database::error(
    cass_error_desc(passwordBindError),
    [connection]() {
      WebSocket::sendNotification(connection, {
        WebSocket::StatusCodes::INTERNAL_SERVER_ERROR,
        ERROR_OCCURRED}
      );
      Utilities::changePage(connection, static_cast<int32_t>(ServicesIdent::AUTHENTICATION_MODEL_PAGE_CHANGE), pages.find(Pages::SIGNUP_CREATE_ACCOUNT)->second);
    });
    return;
  }

  const CassError firstNameBindError = cass_statement_bind_string_by_name(statement.get(), "first_name", callbackData->firstName.c_str());
  if (firstNameBindError != CASS_OK) {
    Log::Database::error(
    cass_error_desc(firstNameBindError),
    [connection]() {
      WebSocket::sendNotification(connection, {
        WebSocket::StatusCodes::INTERNAL_SERVER_ERROR,
        ERROR_OCCURRED}
      );
      Utilities::changePage(connection, static_cast<int32_t>(ServicesIdent::AUTHENTICATION_MODEL_PAGE_CHANGE), pages.find(Pages::SIGNUP_CREATE_ACCOUNT)->second);
    });
    return;
  }

  const CassError lastNameBindError = cass_statement_bind_string_by_name(statement.get(), "last_name", callbackData->lastName.c_str());
  if (lastNameBindError != CASS_OK) {
    Log::Database::error(
    cass_error_desc(lastNameBindError),
    [connection]() {
      WebSocket::sendNotification(connection, {
        WebSocket::StatusCodes::INTERNAL_SERVER_ERROR,
        ERROR_OCCURRED}
      );
      Utilities::changePage(connection, static_cast<int32_t>(ServicesIdent::AUTHENTICATION_MODEL_PAGE_CHANGE), pages.find(Pages::SIGNUP_CREATE_ACCOUNT)->second);
    });
    return;
  }

  const CassError marketingEmailsBindError = cass_statement_bind_bool_by_name(statement.get(), "marketing_emails", static_cast<cass_bool_t>(callbackData->marketingEmails));
  if (marketingEmailsBindError != CASS_OK) {
    Log::Database::error(
    cass_error_desc(marketingEmailsBindError),
    [connection]() {
      WebSocket::sendNotification(connection, {
        WebSocket::StatusCodes::INTERNAL_SERVER_ERROR,
        ERROR_OCCURRED}
      );
      Utilities::changePage(connection, static_cast<int32_t>(ServicesIdent::AUTHENTICATION_MODEL_PAGE_CHANGE), pages.find(Pages::SIGNUP_CREATE_ACCOUNT)->second);
    });
    return;
  }

  const CassError authenticationTokenBindError = cass_statement_bind_string_by_name(statement.get(), "authentication_token", authenticationToken.c_str());
  if (authenticationTokenBindError != CASS_OK) {
    Log::Database::error(
    cass_error_desc(authenticationTokenBindError),
    [connection]() {
      WebSocket::sendNotification(connection, {
        WebSocket::StatusCodes::INTERNAL_SERVER_ERROR,
        ERROR_OCCURRED}
      );
      Utilities::changePage(connection, static_cast<int32_t>(ServicesIdent::AUTHENTICATION_MODEL_PAGE_CHANGE), pages.find(Pages::SIGNUP_CREATE_ACCOUNT)->second);
    });
    return;
  }

  CallbackData::InsertIntoUsers* newCallbackData = new CallbackData::InsertIntoUsers;
  newCallbackData->socket = connection->socket;
  newCallbackData->authenticationToken = std::move(authenticationToken);
  newCallbackData->id = std::move(id);
  newCallbackData->firstName = std::move(callbackData->firstName);
  newCallbackData->lastName= std::move(callbackData->lastName);
  newCallbackData->email = std::move(callbackData->email);

  newFuture.reset(cass_session_execute(Database::connection.get(), statement.get()));

  Utilities::CallbackConnection::insert(connection->socket, connection);

  const CassError setCallbackError = cass_future_set_callback(newFuture.get(), insertIntoUsers, newCallbackData);
  if (setCallbackError != CASS_OK) {
    delete newCallbackData;
    newCallbackData = nullptr;
    Utilities::CallbackConnection::erase(connection->socket);
    Log::Database::error(
      cass_error_desc(setCallbackError),
      [connection]() {
        WebSocket::sendNotification(connection, {
          WebSocket::StatusCodes::INTERNAL_SERVER_ERROR,
          ERROR_OCCURRED}
        );
        Utilities::changePage(connection, static_cast<int32_t>(ServicesIdent::AUTHENTICATION_MODEL_PAGE_CHANGE), pages.find(Pages::SIGNUP_CREATE_ACCOUNT)->second);
    });
  }
}

void Services::Authentication::Signup::signup(std::shared_ptr<bwss::Connection> connection, boost::json::object message) {
  // Get data object
  const boost::json::value* dataValue = message.if_contains("data");
  if (!dataValue || !dataValue->is_object()) {
    connection->send("Missing or invalid 'data' object", bwss::OpCodes::TEXT_FRAME);
    return;
  }

  const boost::json::object& dataObject = dataValue->get_object();

  // Email
  auto emailResult = Extract::email(dataObject);
  if (!emailResult.first) {
    WebSocket::sendNotification(connection, {
      WebSocket::StatusCodes::INTERNAL_SERVER_ERROR,
      emailResult.second}
    );

    Utilities::changePage(connection, static_cast<int32_t>(ServicesIdent::AUTHENTICATION_MODEL_PAGE_CHANGE), pages.find(Pages::SIGNUP_CREATE_ACCOUNT)->second);
    return;
  }

  // Password
  auto passwordResult = Extract::password(dataObject);
  if (!passwordResult.first) {
    WebSocket::sendNotification(connection, {
        WebSocket::StatusCodes::BAD_REQUEST,
        passwordResult.second}
    );
    Utilities::changePage(connection, static_cast<int32_t>(ServicesIdent::AUTHENTICATION_MODEL_PAGE_CHANGE), pages.find(Pages::SIGNUP_CREATE_ACCOUNT)->second);
    return;
  }

  // First name
  auto firstNameResult = Extract::firstName(dataObject);
  if (!firstNameResult.first) {
    WebSocket::sendNotification(connection, {
      WebSocket::StatusCodes::BAD_REQUEST,
      firstNameResult.second}
    );
    Utilities::changePage(connection, static_cast<int32_t>(ServicesIdent::AUTHENTICATION_MODEL_PAGE_CHANGE), pages.find(Pages::SIGNUP_PERSONAL_DETAILS)->second);
    return;
  }

  // Last name
  auto lastNameResult = Extract::lastName(dataObject);
  if (!lastNameResult.first) {
    WebSocket::sendNotification(connection, {
      WebSocket::StatusCodes::BAD_REQUEST,
      lastNameResult.second}
    );
    Utilities::changePage(connection, static_cast<int32_t>(ServicesIdent::AUTHENTICATION_MODEL_PAGE_CHANGE), pages.find(Pages::SIGNUP_PERSONAL_DETAILS)->second);
    return;
  }

  // Marketing emails
  const auto marketingEmailsResult = Extract::marketingEmails(dataObject);
  if (!marketingEmailsResult.first) {
    WebSocket::sendNotification(connection, {
     WebSocket::StatusCodes::BAD_REQUEST,
     std::get<std::string>(marketingEmailsResult.second)}
    );
    Utilities::changePage(connection, static_cast<int32_t>(ServicesIdent::AUTHENTICATION_MODEL_PAGE_CHANGE), pages.find(Pages::SIGNUP_PERSONAL_DETAILS)->second);
    return;
  }

  std::unique_ptr<CassStatement, void(*)(CassStatement*)> statement(nullptr, cass_statement_free);
  std::unique_ptr<CassFuture, void(*)(CassFuture*)> future(nullptr, cass_future_free);

  /*
   * Check if an account already exists with this email
   */
  statement.reset(cass_prepared_bind(selectFromEmailLookup.get()));

  const CassError emailBindError = cass_statement_bind_string(statement.get(), 0, emailResult.second.c_str());
  if (emailBindError != CASS_OK) {
    Log::Database::error(
    cass_error_desc(emailBindError),
    [connection]() {
        WebSocket::sendNotification(connection, {
          WebSocket::StatusCodes::INTERNAL_SERVER_ERROR,
          ERROR_OCCURRED}
        );
        Utilities::changePage(connection, static_cast<int32_t>(ServicesIdent::AUTHENTICATION_MODEL_PAGE_CHANGE), pages.find(Pages::SIGNUP_CREATE_ACCOUNT)->second);
    });
    return;
  }

  future.reset(cass_session_execute(Database::connection.get(), statement.get()));

  Utilities::CallbackConnection::insert(connection->socket, connection);

  CallbackData::SelectEmail* callbackData = new CallbackData::SelectEmail;

  callbackData->socket = connection->socket;
  callbackData->email = std::move(emailResult.second);
  callbackData->password = std::move(passwordResult.second);
  callbackData->firstName = std::move(firstNameResult.second);
  callbackData->lastName = std::move(lastNameResult.second);
  callbackData->marketingEmails = std::get<bool>(marketingEmailsResult.second);

  // Hash password
  auto passwordHashResult = Utilities::Password::hash(callbackData->password);
  if (!passwordHashResult.first) {
    Log::error("Failed to hash password");
    WebSocket::sendNotification(connection, {
      WebSocket::StatusCodes::INTERNAL_SERVER_ERROR,
      ERROR_OCCURRED}
    );
    Utilities::changePage(connection, static_cast<int32_t>(ServicesIdent::AUTHENTICATION_MODEL_PAGE_CHANGE), pages.find(Pages::SIGNUP_CREATE_ACCOUNT)->second);
    return;
  }

  callbackData->password = std::move(passwordHashResult.second);

  const CassError setCallbackError = cass_future_set_callback(future.get(), Callbacks::selectEmail, callbackData);
  if (setCallbackError != CASS_OK) {
    delete callbackData;
    callbackData = nullptr;
    Utilities::CallbackConnection::erase(connection->socket);
    Log::Database::error(
      cass_error_desc(setCallbackError),
      [connection]() {
        WebSocket::sendNotification(connection, {
          WebSocket::StatusCodes::INTERNAL_SERVER_ERROR,
          ERROR_OCCURRED}
        );
        Utilities::changePage(connection, static_cast<int32_t>(ServicesIdent::AUTHENTICATION_MODEL_PAGE_CHANGE), pages.find(Pages::SIGNUP_CREATE_ACCOUNT)->second);
    });
  }
} // scope end