#include "metro/vehicles/electric_scooter.h"

using namespace metro;

void ElectricScooter::Spawn(Tachyon* tachyon, State& state, const Scooter& scooter) {
  auto& meshes = state.meshes;

  auto& stem = create(meshes.e_scooter_stem);
  auto& fold = create(meshes.e_scooter_fold);
  auto& deck = create(meshes.e_scooter_deck);
  auto& wheel_1 = create(meshes.e_scooter_wheel);
  auto& wheel_2 = create(meshes.e_scooter_wheel);

  stem.scale = tVec3f(2000.f);
  fold.scale = tVec3f(2000.f);
  deck.scale = tVec3f(2000.f);
  wheel_1.scale = tVec3f(2000.f);
  wheel_2.scale = tVec3f(2000.f);

  commit(stem);
  commit(fold);
  commit(deck);
  commit(wheel_1);
  commit(wheel_2);
}

void ElectricScooter::HandlePhysics(Tachyon* tachyon, State& state, Scooter& scooter) {
  // @todo
}

void ElectricScooter::Update(Tachyon* tachyon, State& state, Scooter& scooter, const int32 index) {
  auto& meshes = state.meshes;

  auto& stem = objects(meshes.e_scooter_stem)[index];
  auto& fold = objects(meshes.e_scooter_fold)[index];
  auto& deck = objects(meshes.e_scooter_deck)[index];
  // @temporary
  uint16 wheel_index = 2 * index;

  auto& wheel_1 = objects(meshes.e_scooter_wheel)[wheel_index];
  auto& wheel_2 = objects(meshes.e_scooter_wheel)[wheel_index + 1];

  stem.position = scooter.position;
  fold.position = scooter.position;
  deck.position = scooter.position;
  // @temporary
  wheel_1.position = scooter.position;
  wheel_2.position = scooter.position;

  commit(stem);
  commit(fold);
  commit(deck);
  // @temporary
  commit(wheel_1);
  commit(wheel_2);
}

void ElectricScooter::Destroy(Tachyon* tachyon, State& state, Scooter& scooter) {
  // @todo
}