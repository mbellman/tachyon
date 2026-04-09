#pragma once

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_console.h"
#include "engine/tachyon_types.h"

#define profile(message) tProfiler __profiler(message, false)
#define log_time(message) tProfiler __profiler(message, true)

struct tRecordedTiming {
  std::string name;
  uint64 duration;
};

uint64 Tachyon_GetMicroseconds();
void Tachyon_ResetTimingProfile();
void Tachyon_RecordTiming(const std::string& name, uint64 duration);
std::vector<tRecordedTiming>& Tachyon_GetTimingProfile();

struct tProfiler {
  std::string message;
  uint64 start_time = 0;
  bool output_to_console = false;

  tProfiler(const char* message, bool output_to_console) {
    start_time = Tachyon_GetMicroseconds();

    this->message = message;
    this->output_to_console = output_to_console;
  }

  ~tProfiler() {
    uint64 duration = Tachyon_GetMicroseconds() - start_time;

    Tachyon_RecordTiming(this->message, duration);

    if (output_to_console) {
      float duration_in_ms = float(duration) / 1000.f;
      std::string time_string = std::to_string(duration_in_ms) + "ms";

      console_log(message + " " + time_string);
    }
  }
};