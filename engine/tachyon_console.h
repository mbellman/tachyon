#pragma once

#include <string>
#include <vector>

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_linear_algebra.h"
#include "engine/tachyon_quaternion.h"
#include "engine/tachyon_types.h"

#define console_log(message) Tachyon_Log(message);
#define console_warn(message) Tachyon_AddConsoleMessage(message, tVec3f(1.f, 0.8f, 0.4f))
#define console_error(message) Tachyon_AddConsoleMessage(message, tVec3f(1.f, 0, 0))
#define console_info(message) Tachyon_AddConsoleMessage(message, tVec3f(0.6f, 0.7f, 1.f))

#define show_overlay_message(message)\
  tachyon->overlay_message = message;\
  tachyon->last_overlay_message_time = tachyon->running_time;\

struct tConsoleMessage {
  uint64 time;
  std::string message;
  tVec3f color;
};

const static uint64 CONSOLE_MESSAGE_DURATION = 20000000;

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
void Tachyon_ManageConsoleMessageLifetimes();
void Tachyon_ClearConsole(Tachyon* tachyon);