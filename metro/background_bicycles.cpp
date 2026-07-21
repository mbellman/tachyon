#include "metro/background_bicycles.h"

using namespace metro;

static void UpdateNearBicycles(Tachyon* tachyon, State& state) {
  // @todo
}

static void UpdateFarBicycles(Tachyon* tachyon, State& state) {
  // @todo
}

void BackgroundBicycles::Update(Tachyon* tachyon, State& state) {
  UpdateNearBicycles(tachyon, state);
  UpdateFarBicycles(tachyon, state);
}