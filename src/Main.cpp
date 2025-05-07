#include <stdlib.h>
#include <utility>
#include "WebSocket.h"
#include <bwss/bwss.h>

int main() {
  // Setup WebSocket server
  bwss::onOpen = WebSocket::onOpen;
  bwss::onMessage = WebSocket::onMessage;
  bwss::onClose = WebSocket::onClose;

  bwss::run();

  __builtin_unreachable();
}