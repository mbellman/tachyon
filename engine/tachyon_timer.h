#pragma once

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_types.h"

#define profile(message) tProfiler __profiler(tachyon, message)

uint64 Tachyon_GetMicroseconds();

struct tProfiler {
  Tachyon* tachyon = nullptr;
  std::string message;
  uint64 start_time = 0;

  tProfiler(Tachyon* tachyon, const char* message) {
    start_time = Tachyon_GetMicroseconds();

    this->tachyon = tachyon;
    this->message = message;
  }

  ~tProfiler() {
    uint64 time = Tachyon_GetMicroseconds() - start_time;

    tachyon->dev_labels.push_back({ message, std::to_string(time) + "us" });
  }
};