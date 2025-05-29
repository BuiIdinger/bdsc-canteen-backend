#pragma once

#include <memory>
#include <bwss/bwss.h>
#include <boost/json.hpp>
#include <unordered_map>
#include <cassandra.h>

namespace Services::Authentication::Signup {
  enum class Pages {
    DEFAULT,
    LOGIN,
    TWO_FA,
    SIGNUP_CREATE_ACCOUNT,
    SIGNUP_LEGAL,
    SIGNUP_PERSONAL_DETAILS,
    LOADING,
    CLOSE,
  };

  const inline std::unordered_map<Pages, std::string> pages = {
    {Pages::DEFAULT, "DEFAULT"},
    {Pages::LOGIN, "LOGIN"},
    {Pages::TWO_FA, "TWO_FA"},
    {Pages::SIGNUP_CREATE_ACCOUNT, "SIGNUP_CREATE_ACCOUNT"},
    {Pages::SIGNUP_LEGAL, "SIGNUP_LEGAL"},
    {Pages::SIGNUP_PERSONAL_DETAILS, "SIGNUP_PERSONAL_DETAILS"},
    {Pages::LOADING, "LOADING"},
    {Pages::CLOSE, "CLOSE"},
  };

  namespace CallbackData {
    struct SelectEmail{
      int socket;
      std::string email;
      std::string password;
      std::string firstName;
      std::string lastName;
      bool marketingEmails;
    };

    struct InsertIntoUsers {
      int socket;
      CassUuid id;
      std::string email;
      std::string firstName;
      std::string lastName;
      std::string authenticationToken;
    };

    struct InsertIntoEmailLookup {
      int socket;
      CassUuid id;
      std::string email;
      std::string firstName;
      std::string lastName;
      std::string authenticationToken;
    };
  }

  inline std::unique_ptr<const CassPrepared, void(*)(const CassPrepared*)> selectFromEmailLookup(nullptr, cass_prepared_free);
  inline std::unique_ptr<const CassPrepared, void(*)(const CassPrepared*)> insertIntoUsersTable(nullptr, cass_prepared_free);
  inline std::unique_ptr<const CassPrepared, void(*)(const CassPrepared*)> insertIntoEmailLookupTable(nullptr, cass_prepared_free);
  void prepareStatements();

  void signup(std::shared_ptr<bwss::Connection> connection, boost::json::object message);

  // Callbacks
  namespace Callbacks {
    void selectEmail(CassFuture* future, void* data);
    void insertIntoUsers(CassFuture* future, void* data);
    void insertIntoEmailLookup(CassFuture* future, void* data);
  }
}