#include "metro/vehicles/electric_scooter.h"

using namespace metro;

void ElectricScooter::Spawn(Tachyon* tachyon, State& state, const Scooter& scooter) {
  auto& meshes = state.meshes;

  auto& stem = create(meshes.e_scooter_stem);
  auto& fold = create(meshes.e_scooter_fold);
  auto& handlebars = create(meshes.e_scooter_handlebars);
  auto& grips = create(meshes.e_scooter_grips);
  auto& deck = create(meshes.e_scooter_deck);
  auto& wheel_1 = create(meshes.e_scooter_wheel);
  auto& wheel_2 = create(meshes.e_scooter_wheel);

  stem.scale = tVec3f(2000.f);
  fold.scale = tVec3f(2000.f);
  handlebars.scale = tVec3f(2000.f);
  grips.scale = tVec3f(2000.f);
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
  auto& handlebars = objects(meshes.e_scooter_handlebars)[index];
  auto& grips = objects(meshes.e_scooter_grips)[index];
  auto& deck = objects(meshes.e_scooter_deck)[index];
  // @temporary
  uint16 wheel_index = 2 * index;

  auto& wheel_1 = objects(meshes.e_scooter_wheel)[wheel_index];
  auto& wheel_2 = objects(meshes.e_scooter_wheel)[wheel_index + 1];

  {
    scooter.flat_rotation = Quaternion::FromDirection(scooter.facing_direction, Y_UP);

    // @temporary
    scooter.directional_rotation = scooter.flat_rotation;
  }

  stem.position = scooter.position;
  stem.rotation = scooter.directional_rotation;
  stem.color = 0x1110;
  stem.material = tVec4f(0.3f, 0, 0.2f, 0);

  fold.position = scooter.position;
  fold.rotation = scooter.directional_rotation;
  fold.color = tVec3f(1.f);
  fold.material = tVec4f(0.5f, 0, 0.5f, 0.2f);

  deck.position = scooter.position;
  deck.rotation = scooter.directional_rotation;
  deck.color = 0x1110;
  deck.material = tVec4f(0.6f, 0, 0, 0.2f);

  handlebars.position = scooter.position;
  handlebars.rotation = scooter.directional_rotation;
  handlebars.color = tVec3f(0.8f);
  handlebars.material = tVec4f(0.4f, 1.f, 0, 0);

  grips.position = scooter.position;
  grips.rotation = scooter.directional_rotation;
  grips.color = tVec3f(0.1f);
  grips.material = tVec4f(0.7f, 0, 0, 0.5f);

  // @temporary
  wheel_1.position = scooter.position;
  wheel_1.rotation = scooter.directional_rotation;
  wheel_1.color = tVec3f(0.1f);
  wheel_1.material = tVec4f(0.8f, 0, 0, 0.5f);

  // @temporary
  wheel_2.position = scooter.position;
  wheel_2.rotation = scooter.directional_rotation;
  wheel_2.color = tVec3f(0.1f);
  wheel_2.material = tVec4f(0.8f, 0, 0, 0.5f);

  commit(stem);
  commit(fold);
  commit(handlebars);
  commit(grips);
  commit(deck);
  // @temporary
  commit(wheel_1);
  commit(wheel_2);
}

void ElectricScooter::Destroy(Tachyon* tachyon, State& state, Scooter& scooter) {
  // @todo
}