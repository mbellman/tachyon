#include <chrono>

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_timer.h"

static std::vector<tRecordedTiming> recorded_timings;

uint64 Tachyon_GetMicroseconds() {
  auto now = std::chrono::system_clock::now();

  return std::chrono::time_point_cast<std::chrono::microseconds>(now).time_since_epoch().count();
}

void Tachyon_ResetTimingProfile() {
  recorded_timings.clear();
}

void Tachyon_RecordTiming(const std::string& name, const uint64 duration) {
  recorded_timings.push_back({ name, duration });
}

std::vector<tRecordedTiming>& Tachyon_GetTimingProfile() {
  return recorded_timings;
}