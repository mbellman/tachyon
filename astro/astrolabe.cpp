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
  auto& ring = objects(meshes.astrolabe_ring)[0];
  auto& hand = objects(meshes.astrolabe_hand)[0];

  rear.scale =
  base.scale =
  plate.scale =
  ring.scale =
  hand.scale =
  tVec3f(200.f);

  rear.scale = tVec3f(205.f);

  rear.color = tVec3f(0.1f);
  rear.material = tVec4f(0, 1.f, 0, 0);

  base.color = tVec3f(0.7f, 0.4f, 0.1f);
  base.material = tVec4f(0, 1.f, 1.f, 0.4f);

  plate.color = tVec3f(0.25f, 0.1f, 0.1f);
  plate.material = tVec4f(0, 0, 0, 0);

  ring.color = tVec3f(0.9f, 0.8f, 0.1f);
  ring.material = tVec4f(0, 1.f, 0, 0.1f);

  hand.color = tVec3f(0.5f, 0.1f, 0.1f);
  hand.material = tVec4f(0.1f, 1.f, 0, 0);

  rear.rotation =
  base.rotation =
  plate.rotation =
  ring.rotation =
  (
    Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -state.camera_angle) *
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -t_HALF_PI * 0.85f)
  );

  hand.rotation =
  (
    base.rotation *
    Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -state.astro_time * 0.02f)
  );

  // Ring behavior
  {
    float ring_angle = -state.astro_time * 0.0412f - 0.04f;

    if (state.astro_time < -76.f) {
      // @hack Tweaky alignment stuff for the rotating ring
      //
      // @todo as unscrupulous as this correction is, we might be able
      // to get away with correcting the hand rotation as well and then
      // using equidistant min/max time segments, which would bring us
      // closer to normalizing some of this.
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

  // Fragments
  {
    tVec3f fragment_color = tVec3f(0.7f, 0.5f, 0.2f);

    auto& fragment_upper_left = objects(meshes.astrolabe_fragment_ul)[0];
    auto& fragment_lower_left = objects(meshes.astrolabe_fragment_ll)[0];

    fragment_upper_left.position = base.position;
    fragment_upper_left.scale = base.scale;
    fragment_upper_left.rotation = base.rotation;
    fragment_upper_left.color = fragment_color;
    fragment_upper_left.material = base.material;

    if (Items::HasItem(state, ASTROLABE_LOWER_LEFT)) {
      fragment_lower_left.position = base.position;
      fragment_lower_left.scale = base.scale;
      fragment_lower_left.rotation = base.rotation;
      fragment_lower_left.color = fragment_color;
      fragment_lower_left.material = base.material;
    } else {
      fragment_lower_left.scale = tVec3f(0.f);
    }

    commit(fragment_upper_left);
    commit(fragment_lower_left);
  }

  // Light
  {
    auto* light = get_point_light(state.astrolabe_light_id);
    tVec3f default_light_color = tVec3f(1.f, 0.8f, 0.4f);

    light->position = base.position + tVec3f(-10.f, -4.f, 8.f);
    light->radius = 300.f;
    light->color = default_light_color;
    light->power = 0.5f + 20.f * abs(state.astro_turn_speed);
    light->glow_power = 0.f;

    // @todo factor
    if (time_since(state.game_time_at_start_of_turn) < 2.f) {
      float alpha = time_since(state.game_time_at_start_of_turn) / 2.f;

      light->power += 5.f * sinf(alpha * t_PI);
    }
  }

  commit(rear);
  commit(base);
  commit(plate);
  commit(ring);
  commit(hand);
}

float Astrolabe::GetMaxAstroTime(const State& state) {
  if (Items::HasItem(state, ASTROLABE_UPPER_RIGHT)) {
    return astro_time_periods.future;
  }

  return astro_time_periods.present;
}

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