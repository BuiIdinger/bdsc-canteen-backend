#include "WebSocket.h"
#include <bwss/bwss.h>
#include "Database.h"
#include "TESTS.H"
#include "Main.h"

void shutdown(const int& code) noexcept {
  exit(code);
}

/**
 * Hey there! Welcome to the backend code of this project.
 *
 * <!---------------------------------------------------------------------------------------------->
 *
 * Each feature on the app will have somthing called a service associated with
 * that feature, I use a unique int32_t to identify services on this app.
 *
 * Every feature will typically have its own source file just for that features
 * logic, every file is named appropriately so it's pretty self-explanatory to know what files
 * allow different features to work.
 *
 * <!---------------------------------------------------------------------------------------------->
 *
 * You might see alot of std::pair<bool, std::string> or somthing similar with different types.
 * That's my way of handling errors when returning from functions, I use the bool to detect if an
 * error has occurred, and then the string to explain the error.
 *
 * An example code of using my way of error handling:
 *
 * template<typename T>
 * std::pair<bool, T> changeName(const std::string& name, std::string newName) {
 *   if (newName.compare("shayden") {
 *      return { true, "This name is forbidden" }; // Name is not allowed, return true, and a string explaining the error
 *   }
 *
 *   name = newName;
 *   return { false, 0 }; // I return an integer to save overhead from constructing a new std::string
 * }
 *
 * const std::string currentName = "bob";
 *
 * auto it = changeName(currentName, "david");
 * if (it.first) { // Checking if an error occurred, the first item would evaluate to true if so
 *   std::cout << it.second << "\n"; // I'm printing out why this error occurred
 * }
 *
 * // Name was successfully changed!
 *
 * <!---------------------------------------------------------------------------------------------->
 *
 * Every test is located in TESTS.H, below is runTests() commented out, which will invoke any
 * tests in that function, these are more like unit tests. Currently, there is no need to run your
 * own tests, as every feature I've tested myself and has passed. The tests file is only there to
 * comply with the assignment conditions.
 *
 * If you database access for any reason, please ask me. Although its really not needed,
 * I've documented all my table designs on the main documentation and left out the boring stuff
 * like setting-up the servers.
 *
 * If you don't know C++, that's should be fine. The code is possibly somewhat self-explanatory if you have knowledge of other
 * languages, I hope. Yes it is very overkill to be using for this kind of project, GO would be a better
 * choice of course, but I still don't know Go deeply. I only know C++ deeply, thus is why I've used it for
 * this project.
 *
 * This project is pretty difficult to compile as it requires having a database setup
 * with correct TLS, authentication and more. Thus, I suggest not to compile this repo due
 * to those reasons, instead I recommend looking at the prod version at https://canteen-app.buildinger.org
 *
 * Any namespace starting with bwss is created by me, it's my own WebSocket library written in C++.
 *
 * Everything in this project is asynchronous.
 */
int main() {
  // runTests();

  Database::connect();
  
  // Setup WebSocket server
  bwss::onOpen = WebSocket::onOpen;
  bwss::onMessage = WebSocket::onMessage;
  bwss::onClose = WebSocket::onClose;

  bwss::run();

  UNREACHABLE;
}