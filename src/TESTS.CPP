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

void runTests() {
  std::cout << "Tests running" << "\n";
  runPasswordAndEmailTests();

  std::cout << "Completed tests" << "\n";
}