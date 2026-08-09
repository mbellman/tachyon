#include "engine/tachyon.h"

#include "metro/bikes/common_bike.h"
#include "metro/utilities.h"

using namespace metro;

const static tVec3f STEERING_AXIS = tVec3f(0, 0.9731f, -0.2305f);
const static tVec3f WHEEL_AXIS = AXIS_X;
const static tVec3f LEANING_AXIS = AXIS_Z;

const static tVec3f BACK_WHEEL_PIVOT_POSITION = tVec3f(0, 0, -0.61f);
const static tVec3f FRONT_WHEEL_PIVOT_POSITION = tVec3f(0, 0, 0.61f);

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

// @todo see how much of this can be refactored when we add more bike types
void CommonBike::HandlePhysics(Tachyon* tachyon, State& state, Bicycle& bike) {
  const float gravity = 500000.f;
  const float mass = 25.f;

  tVec3f ideal_front_wheel_position = bike.front_wheel_position;
  tVec3f ideal_back_wheel_position = bike.back_wheel_position;

  float highest_front_y = -FLT_MAX;
  float highest_back_y = -FLT_MAX;

  tVec3f front_wheel_plane_normal = tVec3f(0, 1.f, 0);
  tVec3f back_wheel_plane_normal = tVec3f(0, 1.f, 0);

  // Wheel collision
  {
    const float above_wheel_buffer = 500.f;
    const float ray_length = 1500.f;
    const float front_wheel_ground_distance = 800.f;
    const float back_wheel_ground_distance = 820.f;

    for_static_entity_containers() {
      for_entities() {
        for (auto& plane : entity.collision_planes) {
          tVec3f start_offset = plane.normal * above_wheel_buffer;
          tVec3f front_ray_start = bike.front_wheel_position + start_offset;
          tVec3f back_ray_start = bike.back_wheel_position + start_offset;
          tVec3f down_ray = plane.normal.invert() * ray_length;

          auto front = Collision::TestRayHit(front_ray_start, down_ray, plane);
          auto back = Collision::TestRayHit(back_ray_start, down_ray, plane);

          if (front.has_collision) {
            tVec3f resolved_position = front.collision_point + plane.normal * front_wheel_ground_distance;

            if (resolved_position.y > highest_front_y) {
              ideal_front_wheel_position = resolved_position;
              highest_front_y = resolved_position.y;
              front_wheel_plane_normal = plane.normal;
            }
          }

          if (back.has_collision) {
            tVec3f resolved_position = back.collision_point + plane.normal * back_wheel_ground_distance;

            if (resolved_position.y > highest_back_y) {
              ideal_back_wheel_position = resolved_position;
              highest_back_y = resolved_position.y;
              back_wheel_plane_normal = plane.normal;
            }
          }
        }
      }
    }
  }

  bool front_wheel_down = highest_front_y > -FLT_MAX;
  bool back_wheel_down = highest_back_y > -FLT_MAX;
  bool in_freefall = !front_wheel_down && !back_wheel_down;

  bike.in_freefall = in_freefall;

  if (ideal_back_wheel_position.y != bike.back_wheel_position.y) {
    tVec3f delta = ideal_back_wheel_position - bike.back_wheel_position;

    bike.position += delta;
  }

  if (front_wheel_down && !back_wheel_down) {
    if (ideal_front_wheel_position.y != bike.front_wheel_position.y) {
      tVec3f delta = ideal_front_wheel_position - bike.front_wheel_position;

      bike.position += delta;
    }
  }

  // Downward wheel forces. This is just used when landing
  // on the front or back wheel first to determine how fast
  // the other should fall, or how fast the bike should pitch
  // to a level position.
  {
    if (front_wheel_down) {
      bike.front_wheel_downward_force = 0.f;
      bike.front_wheel_slope = front_wheel_plane_normal;
    } else {
      bike.front_wheel_downward_force += gravity * state.dt;
    }

    if (back_wheel_down) {
      bike.back_wheel_downward_force = 0.f;
      bike.back_wheel_slope = back_wheel_plane_normal;
    } else {
      bike.back_wheel_downward_force += gravity * state.dt;
    }
  }

  // Pitch
  {
    // @todo this produces more "correct-looking" behavior,
    // but we should use a value with a more physical basis
    const float freefall_pitch_factor = 0.0005f;

    // Pitch the bike based on where the wheels need to be
    if (front_wheel_down && back_wheel_down) {
      float wheel_ground_delta = ideal_front_wheel_position.y - ideal_back_wheel_position.y;
      float wheel_base = (ideal_front_wheel_position - ideal_back_wheel_position).xz().magnitude();
      float pitch = -atanf(wheel_ground_delta / wheel_base);

      bike.pitch = pitch;
    }

    // Pitch forward to try and land the front tire on the ground
    else if (back_wheel_down) {
      float pitch_rate = freefall_pitch_factor * bike.front_wheel_downward_force / mass;

      bike.pitch += pitch_rate * state.dt;
    }

    // Pitch backward to try and land the back tire on the ground
    else if (front_wheel_down) {
      float pitch_rate = freefall_pitch_factor * bike.back_wheel_downward_force / mass;

      bike.pitch -= pitch_rate * state.dt;
    }

    // Pitch forward gradually in freefall
    else {
      float pitch_rate = freefall_pitch_factor * (
        bike.front_wheel_downward_force -
        bike.back_wheel_downward_force
      ) / mass;

      bike.pitch += pitch_rate * state.dt;
    }
  }

  // Apply sloped surface forces as speed changes
  {
    tVec3f average_slope_normal = (
      front_wheel_plane_normal +
      back_wheel_plane_normal
    ).unit();

    float slope_dot = tVec3f::dot(average_slope_normal, bike.facing_direction);

    bike.speed += (gravity / mass) * slope_dot * state.dt;
  }

  // Momentum
  // @todo create constant for air resistance
  {
    if (!front_wheel_down || !back_wheel_down) {
      // @todo terminal  velocity
      bike.momentum.y -= gravity * state.dt;

      // Air resistance
      bike.momentum.x *= 1.f - 0.05f * state.dt;
      bike.momentum.z *= 1.f - 0.05f * state.dt;
    } else {
      bike.momentum = GetMovementDirection(bike) * bike.speed * mass;
    }
  }
}

void CommonBike::Update(Tachyon* tachyon, State& state, Bicycle& bike, const int32 index) {
  auto& meshes = state.meshes;

  // Store the bike index if we're currently riding it
  {
    if (bike.id == state.player_bike_id) {
      state.player_bike_index = index;
    }
  }

  // Rotate around the turning pivot position
  {
    // Track the pivot before recomputing rotation
    tVec3f old_pivot = UnitBikeToWorldPosition(bike, BACK_WHEEL_PIVOT_POSITION);

    bike.flat_rotation =
      Quaternion::FromDirection(bike.facing_direction, tVec3f(0, 1.f, 0)) *
      Quaternion::fromAxisAngle(LEANING_AXIS, bike.leaning_angle);

    // Offset the bike by the pivot delta to keep it centered on the pivot.
    // We rotate around the back wheel for more physically grounded motion.
    tVec3f new_pivot = UnitBikeToWorldPosition(bike, BACK_WHEEL_PIVOT_POSITION);

    bike.position += old_pivot - new_pivot;
    bike.visual_position = bike.position;
  }

  // Apply pitch
  {
    bike.directional_rotation = bike.flat_rotation * Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), bike.pitch);
    bike.visual_rotation = bike.directional_rotation;
  }

  // When pedaling faster, apply rocking effects in the form of slight
  // lean adjustments and wobbling back and forth around the front wheel.
  // This is a purely visual effect, and does not affect the bicycle's
  // actual motion vector.
  {
    tVec3f old_rocking_pivot = UnitVisualBikeToWorldPosition(bike, FRONT_WHEEL_PIVOT_POSITION);

    bike.visual_rotation =
      Quaternion::fromAxisAngle(AXIS_Y, bike.rocking_factor) *
      bike.visual_rotation *
      Quaternion::fromAxisAngle(LEANING_AXIS, bike.rocking_factor);

    tVec3f new_rocking_pivot = UnitVisualBikeToWorldPosition(bike, FRONT_WHEEL_PIVOT_POSITION);

    bike.visual_position += old_rocking_pivot - new_rocking_pivot;
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
  auto& meshes = state.meshes;

  #define remove_one(__mesh_index) remove_object(objects(__mesh_index)[0])

  remove_one(meshes.common_frame);
  remove_one(meshes.common_fork);
  remove_one(meshes.common_handlebars);
  remove_one(meshes.common_grips);
  remove_one(meshes.common_seatpost);
  remove_one(meshes.common_saddle);
  remove_one(meshes.common_crank);

  remove_one(meshes.common_wheel);
  remove_one(meshes.common_wheel);

  remove_one(meshes.common_spokes);
  remove_one(meshes.common_spokes);

  #undef remove_one
}