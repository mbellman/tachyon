#include <chrono>

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_timer.h"

uint64 Tachyon_GetMicroseconds() {
  auto now = std::chrono::system_clock::now();

  return std::chrono::time_point_cast<std::chrono::microseconds>(now).time_since_epoch().count();
}