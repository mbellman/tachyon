#pragma once

#include <string>
#include <vector>

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_linear_algebra.h"

struct tConsoleMessage {
  uint64 time;
  std::string message;
  tVec3f color;
};

#define add_console_message(message, color) Tachyon_AddConsoleMessage(message, color)

void Tachyon_AddConsoleMessage(const std::string& message, const tVec3f& color);
const std::vector<tConsoleMessage>& Tachyon_GetConsoleMessages();
void Tachyon_ProcessConsoleMessages();