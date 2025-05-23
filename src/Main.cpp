#include "WebSocket.h"
#include <bwss/bwss.h>
#include "Database.h"
#include "TESTS.H"
#include "Main.h"

void shutdown(const int& code) noexcept {
  exit(code);
}

int main() {
  // runTests();

  Database::connect();
  
  // Setup WebSocket server
  bwss::onOpen = WebSocket::onOpen;
  bwss::onMessage = WebSocket::onMessage;
  bwss::onClose = WebSocket::onClose;

  bwss::run();

  __builtin_unreachable();
}