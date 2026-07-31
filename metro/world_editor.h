#pragma once

#include "metro/game_state.h"

namespace metro {
  namespace WorldEditor {
    void Open(Tachyon* tachyon, State& state);
    void Update(Tachyon* tachyon, State& state);
    void Close(Tachyon* tachyon, State& state);
  }
}