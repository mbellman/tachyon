#include "engine/tachyon.h"

#include "metro/bikes/common_bike.h"

using namespace metro;

const static tVec3f STEERING_AXIS = tVec3f(-0.2305f, 0.9731f, 0);
const static tVec3f WHEEL_AXIS = tVec3f(0, 0, 1.f);
const static tVec3f LEANING_AXIS = tVec3f(1.f, 0, 0);

static tVec3f UnitBikeToWorldPosition(const Bicycle& bike, const tVec3f& position) {
  tVec3f translation = bike.position;
  Quaternion rotation = bike.computed_rotation;
  tVec3f scale = tVec3f(2000.f);

  return translation + rotation.toMatrix4f() * (position * scale);
}

static tVec3f UnitObjectToWorldPosition(const tObject object, const tVec3f& position) {
  tVec3f translation = object.position;
  Quaternion rotation = object.rotation;
  tVec3f scale = object.scale;

  return translation + rotation.toMatrix4f() * (position * scale);
}

void CommonBike::Spawn(Tachyon* tachyon, State& state, const Bicycle& bike) {
  auto& meshes = state.meshes;

  auto& frame = create(meshes.common_frame);
  auto& fork = create(meshes.common_fork);
  auto& handlebars = create(meshes.common_handlebars);
  auto& grips = create(meshes.common_grips);
  auto& seatpost = create(meshes.common_seatpost);
  auto& saddle = create(meshes.common_saddle);
  auto& wheel1 = create(meshes.common_wheel);
  auto& spokes1 = create(meshes.common_spokes);
  auto& wheel2 = create(meshes.common_wheel);
  auto& spokes2 = create(meshes.common_spokes);

  frame.scale = tVec3f(2000.f);
  fork.scale = tVec3f(2000.f);
  handlebars.scale = tVec3f(2000.f);
  grips.scale = tVec3f(2000.f);
  seatpost.scale = tVec3f(2000.f);
  saddle.scale = tVec3f(2000.f);
  wheel1.scale = tVec3f(2000.f);
  spokes1.scale = tVec3f(2000.f);
  wheel2.scale = tVec3f(2000.f);
  spokes2.scale = tVec3f(2000.f);

  commit(frame);
  commit(fork);
  commit(handlebars);
  commit(grips);
  commit(seatpost);
  commit(saddle);
  commit(wheel1);
  commit(spokes1);
  commit(wheel2);
  commit(spokes2);
}

void CommonBike::Update(Tachyon* tachyon, State& state, Bicycle& bike, const int32 index) {
  auto& meshes = state.meshes;

  Quaternion steering_rotation = Quaternion::fromAxisAngle(STEERING_AXIS, bike.steering_angle);

  bike.computed_rotation =
    Quaternion::FromDirection(bike.facing_direction, tVec3f(0, 1.f, 0)) *
    Quaternion::fromAxisAngle(LEANING_AXIS, bike.leaning_angle);

  auto& frame = objects(meshes.common_frame)[index];
  auto& fork = objects(meshes.common_fork)[index];
  auto& handlebars = objects(meshes.common_handlebars)[index];
  auto& grips = objects(meshes.common_grips)[index];
  auto& seatpost = objects(meshes.common_seatpost)[index];
  auto& saddle = objects(meshes.common_saddle)[index];

  frame.position = bike.position;
  frame.rotation = bike.computed_rotation;
  frame.color = bike.frame_color;
  frame.material = tVec4f(0.3f, 0, 0.2f, 0);

  fork.position = UnitBikeToWorldPosition(bike, tVec3f(0.445f, 0.44f, 0));
  fork.rotation = bike.computed_rotation * steering_rotation;
  fork.color = bike.frame_color;
  fork.material = tVec4f(0.3f, 0, 0.2f, 0);

  handlebars.position = UnitBikeToWorldPosition(bike, tVec3f(0.39f, 0.68f, 0));
  handlebars.rotation = fork.rotation;
  handlebars.color = tVec3f(0.8f);
  handlebars.material = tVec4f(0.4f, 1.f, 0, 0);

  grips.position = handlebars.position;
  grips.rotation = handlebars.rotation;
  grips.color = bike.grips_color;
  grips.material = tVec4f(0.7f, 0, 0, 0.5f);

  seatpost.position = bike.position;
  seatpost.rotation = bike.computed_rotation;
  seatpost.color = tVec3f(0.8f);
  seatpost.material = tVec4f(0.4f, 1.f, 0, 0);

  saddle.position = bike.position;
  saddle.rotation = bike.computed_rotation;
  saddle.color = bike.saddle_color;
  saddle.material = tVec4f(0.6f, 0, 0, 0.2f);

  commit(frame);
  commit(fork);
  commit(handlebars);
  commit(grips);
  commit(seatpost);
  commit(saddle);

  // Wheels
  {
    int32 wheel_index = index * 2;

    Quaternion wheel_axle_rotation = Quaternion::fromAxisAngle(WHEEL_AXIS, -bike.wheel_revolution);

    auto& front_wheel = objects(meshes.common_wheel)[wheel_index];
    auto& front_spokes = objects(meshes.common_spokes)[wheel_index];

    auto& back_wheel = objects(meshes.common_wheel)[wheel_index + 1];
    auto& back_spokes = objects(meshes.common_spokes)[wheel_index + 1];

    front_wheel.position = UnitObjectToWorldPosition(fork, tVec3f(0.15f, -0.43f, 0));
    front_wheel.rotation = fork.rotation * wheel_axle_rotation;
    front_wheel.color = bike.wheel_color;
    front_wheel.material = tVec4f(0.9f, 0, 0, 0.5f);

    front_spokes.position = front_wheel.position;
    front_spokes.rotation = front_wheel.rotation;
    front_spokes.color = tVec3f(0.8f);
    front_spokes.material = tVec4f(0.4f, 1.f, 0, 0);

    back_wheel.position = UnitBikeToWorldPosition(bike, tVec3f(-0.61f, 0, 0));
    back_wheel.rotation = bike.computed_rotation * wheel_axle_rotation;
    back_wheel.color = bike.wheel_color;
    back_wheel.material = tVec4f(0.9f, 0, 0, 0.5f);

    back_spokes.position = back_wheel.position;
    back_spokes.rotation = back_wheel.rotation;
    back_spokes.color = tVec3f(0.8f);
    back_spokes.material = tVec4f(0.4f, 1.f, 0, 0);

    commit(front_wheel);
    commit(front_spokes);
    commit(back_wheel);
    commit(back_spokes);
  }
}

void CommonBike::Destroy(Tachyon* tachyon, State& state, Bicycle& bike) {
  // @todo
}