#pragma once

#include <string>
#include <vector>

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_linear_algebra.h"
#include "engine/tachyon_quaternion.h"

struct tConsoleMessage {
  uint64 time;
  std::string message;
  tVec3f color;
};

#define add_console_message(message, color) Tachyon_AddConsoleMessage(message, color)
#define console_log(value) Tachyon_Log(value);

void Tachyon_AddConsoleMessage(const std::string& message, const tVec3f& color);
void Tachyon_Log(const char* message);
void Tachyon_Log(const std::string& message);
void Tachyon_Log(const int value);
void Tachyon_Log(const size_t value);
void Tachyon_Log(const float value);
void Tachyon_Log(const bool value);
void Tachyon_Log(const tVec3f& vector);
void Tachyon_Log(const Quaternion& q);
const std::vector<tConsoleMessage>& Tachyon_GetConsoleMessages();
void Tachyon_ProcessConsoleMessages();