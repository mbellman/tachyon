#include <vector>

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_console.h"
#include "engine/tachyon_timer.h"

static std::vector<tConsoleMessage> console_messages;

void Tachyon_AddConsoleMessage(const std::string& message, const tVec3f& color) {
  console_messages.push_back({
    .time = Tachyon_GetMicroseconds(),
    .message = message,
    .color = color
  });

  if (console_messages.size() > 10) {
    console_messages.erase(console_messages.begin());
  }
}

const std::vector<tConsoleMessage>& Tachyon_GetConsoleMessages() {
  return console_messages;
}

void Tachyon_ProcessConsoleMessages() {
  // @todo remove messages by time
}