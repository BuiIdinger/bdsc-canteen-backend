/*
 * RNADOM TEST
 */

#include <iostream>
#include "Utilities.h"
#include <utility>
#include <string>
#include <vector>

static void runPasswordAndEmailTests() {
  std::vector<std::string> password = {
    {"random"},
    {"hi"},
    {"qhurhqehiueqhiihwthiweihthiwuetuiihuthuiwethuiwetewhiuthiuw"},
  };

  for (const auto& password : password) {
    auto it = Utilities::validatePassword(password);
    if (!it.first) {
      std::cout << "Password isn't valid " << it.second << std::endl;
    } else {
      std::cout << "Password " << it.first << " is vaild" << std::endl;
    }
  }

  std::vector<std::string> emails = {
    {"shayden@somthing.com"},
    {"shayden.....seymour@ursomthing.com.org"},
    {"sahyden"},
    {"buildinger.org"},
    {"shayden.seymour@my.bdsc.school.nz"},
    {"buildinger@buildinger.org"}
  };

  for (const auto& email : emails) {
    auto it = Utilities::validateEmail(email);
    if (!it.first) {
      std::cout << "Email isn't valid " << it.second << std::endl;
    } else {
      std::cout << "Email " << it.first << " is valid" << std::endl;
    }
  }
}

static void runHashingPasswordTests() {
  const std::vector<std::string> passwords = {
    {"password"},
    {"randomstuff-lol"},
    {"eshaysInTOWN&%!("},
    {"shayden158"},
  };

  const std::vector<std::string> verifyPasswords = {
    {"password"},
    {"randomstuff-lol"},
    {"eshaysInTOWN&%!("},
    {"shayden158"},
  };

  for (int i = 0; i < 3; ++i) {
    auto it = Utilities::Password::hash(passwords[i]);
    if (!it.first) {
      std::cout << "Unable to hash password, skipping password " << passwords[i] << "\n";
      continue;
    }

    const std::string hashedPassword = it.second;
    std::cout << "Hashed password is: " << hashedPassword;

    if (!Utilities::Password::verify(hashedPassword, verifyPasswords[i])) {
      std::cout << "Password doesn't match " << "\n";
    } else {
      std::cout << "Password matches" << "\n";
    }
  }
}

void runTests() {
  std::cout << "Tests running" << "\n";
 // runPasswordAndEmailTests();
  runHashingPasswordTests();

  std::cout << "Completed tests" << "\n";
}