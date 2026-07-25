#include "engine/tachyon.h"

#include "metro/bikes/common_bike.h"
#include "metro/collision.h"
#include "metro/utilities.h"

using namespace metro;

const static tVec3f STEERING_AXIS = tVec3f(0, 0.9731f, -0.2305f);
const static tVec3f WHEEL_AXIS = tVec3f(1.f, 0, 0);
const static tVec3f LEANING_AXIS = tVec3f(0, 0, 1.f);

const static tVec3f ROTATE_PIVOT_POSITION = tVec3f(0, 0, 0.61f);
const static tVec3f ROCKING_PIVOT_POSITION = tVec3f(0, 0, -0.61f);

void CommonBike::Spawn(Tachyon* tachyon, State& state, const Bicycle& bike) {
  auto& meshes = state.meshes;

  auto& frame = create(meshes.common_frame);
  auto& fork = create(meshes.common_fork);
  auto& handlebars = create(meshes.common_handlebars);
  auto& grips = create(meshes.common_grips);
  auto& seatpost = create(meshes.common_seatpost);
  auto& saddle = create(meshes.common_saddle);
  auto& crank = create(meshes.common_crank);
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
  crank.scale = tVec3f(2000.f);
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

void CommonBike::HandleComplexPhysics(Tachyon* tachyon, State& state, Bicycle& bike) {
  tVec3f down_ray = tVec3f(0, -10000.f, 0);

  bike.front_wheel_fall_velocity += 50000.f * state.dt;
  bike.back_wheel_fall_velocity += 50000.f * state.dt;

  auto& s = objects(state.meshes.dev_sphere)[0];

  s.position = bike.front_wheel_position;
  s.scale = tVec3f(300.f);
  s.color = tVec3f(0, 0, 1.f);

  commit(s);

  for (auto& plane : state.collision_planes) {
    auto front_collision = Collision::TestRayHit(bike.front_wheel_position, down_ray, plane);
    auto back_collision = Collision::TestRayHit(bike.back_wheel_position, down_ray, plane);

    if (front_collision.hit) {
      bike.front_wheel_fall_velocity = 0.f;
      bike.front_wheel_position.y = front_collision.point.y + 800.f;
    }

    if (back_collision.hit) {
      bike.back_wheel_fall_velocity = 0.f;
      bike.back_wheel_position.y = back_collision.point.y + 800.f;
    }
  }

  if (
    bike.front_wheel_fall_velocity == 0.f &&
    bike.back_wheel_fall_velocity == 0.f
  ) {
    bike.position.y = bike.front_wheel_position.y;

    return;
  }

  float fall_velocity = (bike.front_wheel_fall_velocity + bike.back_wheel_fall_velocity) / 2.f;

  bike.position.y -= fall_velocity * state.dt;
}

void CommonBike::Update(Tachyon* tachyon, State& state, Bicycle& bike, const int32 index) {
  auto& meshes = state.meshes;

  // Rotate around the default pivot position
  {
    // Track the pivot before recomputing rotation
    tVec3f old_pivot = UnitBikeToWorldPosition(bike, ROTATE_PIVOT_POSITION);

    bike.computed_rotation =
      Quaternion::FromDirection(bike.facing_direction, tVec3f(0, 1.f, 0)) *
      Quaternion::fromAxisAngle(LEANING_AXIS, bike.leaning_angle + bike.rocking_factor);

    // Offset the bike by the pivot delta to keep it centered on the pivot.
    // We rotate around the back wheel for more physically grounded motion.
    tVec3f new_pivot = UnitBikeToWorldPosition(bike, ROTATE_PIVOT_POSITION);

    bike.position += new_pivot - old_pivot;
  }

  // As the bike rocks when pedaling faster, rotate around
  // the rocking pivot position
  {
    bike.visual_position = bike.position;
    bike.visual_rotation = bike.computed_rotation;

    tVec3f old_rocking_pivot = UnitVisualBikeToWorldPosition(bike, ROCKING_PIVOT_POSITION);

    bike.visual_rotation =
      Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), bike.rocking_factor) *
      bike.computed_rotation;

    tVec3f new_rocking_pivot = UnitVisualBikeToWorldPosition(bike, ROCKING_PIVOT_POSITION);

    bike.visual_position += new_rocking_pivot - old_rocking_pivot;
  }

  auto& frame = objects(meshes.common_frame)[index];
  auto& fork = objects(meshes.common_fork)[index];
  auto& handlebars = objects(meshes.common_handlebars)[index];
  auto& grips = objects(meshes.common_grips)[index];
  auto& seatpost = objects(meshes.common_seatpost)[index];
  auto& saddle = objects(meshes.common_saddle)[index];

  Quaternion steering_rotation = Quaternion::fromAxisAngle(STEERING_AXIS, bike.steering_angle);

  frame.position = bike.visual_position;
  frame.rotation = bike.visual_rotation;
  frame.color = bike.frame_color;
  frame.material = tVec4f(0.3f, 0, 0.2f, 0);

  fork.position = UnitVisualBikeToWorldPosition(bike, tVec3f(0, 0.44f, 0.445f));
  fork.rotation = bike.visual_rotation * steering_rotation;
  fork.color = bike.frame_color;
  fork.material = tVec4f(0.3f, 0, 0.2f, 0);

  handlebars.position = UnitVisualBikeToWorldPosition(bike, tVec3f(0, 0.68f, 0.39f));
  handlebars.rotation = fork.rotation;
  handlebars.color = tVec3f(0.8f);
  handlebars.material = tVec4f(0.4f, 1.f, 0, 0);

  grips.position = handlebars.position;
  grips.rotation = handlebars.rotation;
  grips.color = bike.grips_color;
  grips.material = tVec4f(0.7f, 0, 0, 0.5f);

  seatpost.position = bike.visual_position;
  seatpost.rotation = bike.visual_rotation;
  seatpost.color = tVec3f(0.8f);
  seatpost.material = tVec4f(0.4f, 1.f, 0, 0);

  saddle.position = bike.visual_position;
  saddle.rotation = bike.visual_rotation;
  saddle.color = bike.saddle_color;
  saddle.material = tVec4f(0.6f, 0, 0, 0.2f);

  commit(frame);
  commit(fork);
  commit(handlebars);
  commit(grips);
  commit(seatpost);
  commit(saddle);

  // Crank + pedals
  // @todo pedals
  {
    auto& crank = objects(meshes.common_crank)[index];

    Quaternion pedal_rotation = Quaternion::fromAxisAngle(WHEEL_AXIS, bike.pedal_revolution);

    crank.position = UnitVisualBikeToWorldPosition(bike, tVec3f(0, -0.013f, -0.14f));
    crank.rotation = bike.visual_rotation * pedal_rotation;
    crank.color = tVec3f(0.8f);
    crank.material = tVec4f(0.4f, 1.f, 0, 0);

    commit(crank);
  }

  // Wheels
  {
    int32 wheel_index = index * 2;

    Quaternion wheel_axle_rotation = Quaternion::fromAxisAngle(WHEEL_AXIS, bike.wheel_revolution);

    auto& front_wheel = objects(meshes.common_wheel)[wheel_index];
    auto& front_spokes = objects(meshes.common_spokes)[wheel_index];

    auto& back_wheel = objects(meshes.common_wheel)[wheel_index + 1];
    auto& back_spokes = objects(meshes.common_spokes)[wheel_index + 1];

    front_wheel.position = UnitObjectToWorldPosition(fork, tVec3f(0, -0.43f, 0.15f));
    front_wheel.rotation = fork.rotation * wheel_axle_rotation;
    front_wheel.color = bike.wheel_color;
    front_wheel.material = tVec4f(0.9f, 0, 0, 0.5f);

    front_spokes.position = front_wheel.position;
    front_spokes.rotation = front_wheel.rotation;
    front_spokes.color = tVec3f(0.8f);
    front_spokes.material = tVec4f(0.4f, 1.f, 0, 0);

    back_wheel.position = UnitVisualBikeToWorldPosition(bike, tVec3f(0, 0, -0.61f));
    back_wheel.rotation = bike.visual_rotation * wheel_axle_rotation;
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

    bike.front_wheel_position = front_wheel.position;
    bike.back_wheel_position = back_wheel.position;
  }
}

void CommonBike::Destroy(Tachyon* tachyon, State& state, Bicycle& bike) {
  // @todo
}