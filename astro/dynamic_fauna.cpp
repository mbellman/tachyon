#include "astro/dynamic_fauna.h"

using namespace astro;

static void HandleButterflies(Tachyon* tachyon, State& state) {
  // @todo
}

void DynamicFauna::HandleBehavior(Tachyon* tachyon, State& state) {
  profile("DynamicFauna::HandleBehavior()");

  HandleButterflies(tachyon, state);
}