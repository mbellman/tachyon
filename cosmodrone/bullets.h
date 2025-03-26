#pragma once

#include "cosmodrone/game_types.h"

namespace Cosmodrone {
  namespace Bullets {
    void InitBullets(Tachyon* tachyon, State& state);
    void FireBullet(Tachyon* tachyon, State& state);
    void FireMissile(Tachyon* tachyon, State& state);
    void UpdateBullets(Tachyon* tachyon, State& state, const float dt);
  }
}