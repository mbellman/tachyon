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

void Tachyon_Log(const std::string& message) {
  Tachyon_AddConsoleMessage(message, tVec3f(1.f));
}

void Tachyon_Log(const float value) {
  Tachyon_AddConsoleMessage(std::to_string(value), tVec3f(1.f));
}

void Tachyon_Log(const tVec3f& vector) {
  Tachyon_AddConsoleMessage(vector.toString(), tVec3f(1.f));
}

void Tachyon_Log(const Quaternion& q) {
  Tachyon_AddConsoleMessage(q.toString(), tVec3f(1.f));
}

const std::vector<tConsoleMessage>& Tachyon_GetConsoleMessages() {
  return console_messages;
}

void Tachyon_ProcessConsoleMessages() {
  if (console_messages.size() == 0) {
    return;
  }

  auto now = Tachyon_GetMicroseconds();

  for (int32 i = console_messages.size() - 1; i >= 0; i--) {
    if (now - console_messages[i].time > 20000000) {
      console_messages.erase(console_messages.begin() + i);
    }
  }
}