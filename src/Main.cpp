#include "WebSocket.h"
#include <bwss/bwss.h>
#include "Database.h"

int main() {
  // Database::connect();
  
  // Setup WebSocket server
  bwss::onOpen = WebSocket::onOpen;
  bwss::onMessage = WebSocket::onMessage;
  bwss::onClose = WebSocket::onClose;

  bwss::run();

  __builtin_unreachable();
}