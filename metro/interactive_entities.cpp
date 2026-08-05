#include "metro/interactive_entities.h"
#include "metro/collision.h"

using namespace metro;

// ----------------
// Vending Machines
// ----------------

static void InitVendingMachines(Tachyon* tachyon, State& state) {
  // @todo
}

static void UpdateVendingMachines(Tachyon* tachyon, State& state) {
  // @todo
}

// ---------------------------

void InteractiveEntities::Update(Tachyon* tachyon, State& state) {
  UpdateVendingMachines(tachyon, state);
}