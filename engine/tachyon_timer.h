#pragma once

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_console.h"
#include "engine/tachyon_types.h"

#define profile(message) tProfiler __profiler(tachyon, message, false)
#define log_time(message) tProfiler __profiler(tachyon, message, true)

uint64 Tachyon_GetMicroseconds();

struct tProfiler {
  Tachyon* tachyon = nullptr;
  std::string message;
  uint64 start_time = 0;
  bool output_to_console = false;

  tProfiler(Tachyon* tachyon, const char* message, bool output_to_console) {
    start_time = Tachyon_GetMicroseconds();

    this->tachyon = tachyon;
    this->message = message;
    this->output_to_console = output_to_console;
  }

  ~tProfiler() {
    uint64 time = Tachyon_GetMicroseconds() - start_time;
    float time_in_ms = (float)time / 1000.f;
    char string_buffer[20];

    snprintf(string_buffer, 20, "%.3f", time_in_ms);

    std::string time_string = std::string(string_buffer) + "ms";

    if (output_to_console) {
      console_log(message + " " + time_string);
    } else {
      tachyon->dev_labels.push_back({ message, time_string });
    }
  }
};