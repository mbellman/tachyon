#include "astro/astrolabe.h"
#include "astro/items.h"
#include "astro/targeting.h"

using namespace astro;

// @todo cleanup
//
// @todo figure out a better way to display the astrolabe
// orthographically + with the correct aspect ratio, still
// allowing it to receive lighting somehow?
void Astrolabe::Update(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  auto& meshes = state.meshes;

  auto& rear = objects(meshes.astrolabe_rear)[0];
  auto& base = objects(meshes.astrolabe_base)[0];
  auto& plate = objects(meshes.astrolabe_plate)[0];
  auto& plate2 = objects(meshes.astrolabe_plate2)[0];
  auto& plate3 = objects(meshes.astrolabe_plate3)[0];
  auto& plate4 = objects(meshes.astrolabe_plate4)[0];
  auto& ring = objects(meshes.astrolabe_ring)[0];
  auto& hand = objects(meshes.astrolabe_hand)[0];

  base.scale =
  plate.scale =
  plate2.scale =
  plate3.scale =
  plate4.scale =
  ring.scale =
  hand.scale =
  tVec3f(200.f);

  rear.scale = tVec3f(205.f);

  rear.color = 0x1110;
  rear.material = tVec4f(0, 1.f, 0, 0);

  base.color = tVec3f(0.7f, 0.4f, 0.1f);
  base.material = tVec4f(0.2f, 1.f, 1.f, 0.1f);

  // Plates
  {
    plate.color = 0x1130;
    plate2.color = 0x4120;
    plate3.color = 0x3130;
    plate4.color = 0x1120;

    plate.material =
    plate2.material =
    plate3.material =
    plate4.material = tVec4f(0.1f, 0, 0, 0.4f);
  }

  ring.color = tVec3f(0.9f, 0.8f, 0.1f);
  ring.material = tVec4f(0.3f, 1.f, 0, 0.1f);

  hand.color = tVec3f(0.7f, 0.1f, 0.1f);
  hand.material = tVec4f(0.2f, 1.f, 0, 0);

  rear.rotation =
  base.rotation =
  plate.rotation =
  plate2.rotation =
  plate3.rotation =
  plate4.rotation =
  ring.rotation =
  (
    Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -state.camera_angle) *
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -t_HALF_PI * 0.85f)
  );

  // Hand behavior
  {
    float hand_angle = -state.astro_time * 0.02f;

    if (state.astro_time > astro_time_periods.present) {
      // @hack Tweaky alignment stuff for the hand
      //
      // @todo just define constants for the angles if necessary; this is ridiculous
      hand_angle -= state.astro_time * 0.0015f;
    }

    hand.rotation =
    (
      base.rotation *
      Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), hand_angle)
    );
  }

  // Ring behavior
  {
    float ring_angle = -state.astro_time * 0.0412f - 0.04f;

    if (state.astro_time < -76.f) {
      // @hack Tweaky alignment stuff for the rotating ring
      //
      // @todo just define constants for the angles if necessary; this is ridiculous
      float correction = state.astro_time + 76.f;

      ring_angle += correction * 0.0022f;
    }

    ring.rotation =
    (
      base.rotation *
      Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), ring_angle)
    );
  }

  rear.position =
  base.position =
  plate.position =
  plate2.position =
  plate3.position =
  plate4.position =
  ring.position =
  hand.position =
  (
    camera.position +
    camera.rotation.getDirection() * tVec3f(1.f, -1.f, 1.f) * 2000.f +
    camera.rotation.getLeftDirection() * 1200.f +
    camera.rotation.getUpDirection() * tVec3f(1.f, -1.f, 1.f) * 580.f
  );

  // Adjust rear position
  rear.position += camera.rotation.getLeftDirection() * 12.f;
  rear.position += rear.rotation.getUpDirection().invert() * 9.f;

  // Adjust hand position
  hand.position -= camera.rotation.getLeftDirection() * 6.f;

  // Light
  // @todo factor
  {
    auto* light = get_point_light(state.astrolabe_light_id);
    tVec3f default_light_color = tVec3f(1.f, 0.8f, 0.4f);

    light->position = base.position + tVec3f(-10.f, -4.f, 8.f);
    light->radius = 300.f;
    light->color = default_light_color;
    light->power = 0.5f + 20.f * abs(state.astro_turn_speed);
    light->glow_power = 0.f;

    if (
      state.game_time_at_start_of_turn != 0.f &&
      time_since(state.game_time_at_start_of_turn) < 2.f
    ) {
      float alpha = time_since(state.game_time_at_start_of_turn) / 2.f;

      light->power += 3.f * sinf(alpha * t_PI);
    }
  }

  commit(rear);
  commit(base);
  commit(plate);
  commit(plate2);
  commit(plate3);
  commit(plate4);
  commit(ring);
  commit(hand);
}

// @deprecated @todo remove
float Astrolabe::GetMaxAstroTime(const State& state) {
  if (Items::HasItem(state, ASTROLABE_UPPER_RIGHT)) {
    return astro_time_periods.future;
  }

  return astro_time_periods.present;
}

// @deprecated @todo remove
float Astrolabe::GetMinAstroTime(const State& state) {
  if (Items::HasItem(state, ASTROLABE_LOWER_RIGHT)) {
    return astro_time_periods.very_distant_past;
  }

  if (Items::HasItem(state, ASTROLABE_LOWER_LEFT)) {
    return astro_time_periods.distant_past;
  }

  return astro_time_periods.past;
}

float Astrolabe::GetMaxTurnSpeed() {
  // @todo increase when we get more fragments (?)
  return 0.25f;
}