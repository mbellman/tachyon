#include "astro/astrolabe.h"
#include "astro/items.h"

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

  rear.color = tVec3f(0.1f);
  rear.material = tVec4f(0, 1.f, 0, 0);

  base.color = tVec3f(0.7f, 0.4f, 0.1f);
  base.material = tVec4f(0, 1.f, 1.f, 0.4f);

  plate.color = 0x4110;
  plate.material = tVec4f(0, 0, 0, 0);

  ring.color = tVec3f(0.9f, 0.8f, 0.1f);
  ring.material = tVec4f(0, 1.f, 0, 0.1f);

  hand.color = tVec3f(0.4f, 0.1f, 0.1f);
  hand.material = tVec4f(0.1f, 1.f, 0, 0);

  rear.rotation =
  base.rotation =
  plate.rotation =
  ring.rotation =
  (
    Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -0.9f) *
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

  rear.position += (rear.position - camera.position).unit() * 20.f;
  rear.scale = tVec3f(205.f);
  rear.position += camera.rotation.getLeftDirection() * 1.f;
  rear.position += camera.rotation.getUpDirection() * 18.f;

  hand.position -= camera.rotation.getLeftDirection() * 6.f;

  // Fragments
  {
    auto& fragment_ul = objects(meshes.astrolabe_fragment_ul)[0];
    auto& fragment_ll = objects(meshes.astrolabe_fragment_ll)[0];

    fragment_ul.position = base.position;
    fragment_ul.scale = base.scale;
    fragment_ul.rotation = base.rotation;
    fragment_ul.color = tVec3f(0.7f, 0.5f, 0.2f);
    fragment_ul.material = base.material;

    if (Items::HasItem(state, ASTROLABE_LOWER_LEFT)) {
      fragment_ll.position = base.position;
      fragment_ll.scale = base.scale;
      fragment_ll.rotation = base.rotation;
      fragment_ll.color = tVec3f(0.7f, 0.5f, 0.2f);
      fragment_ll.material = base.material;
    }

    commit(fragment_ul);
    commit(fragment_ll);
  }

  // Add light for visibility
  {
    // @temporary
    // @todo put a light id in state
    // @todo destroy light when opening the editor
    static int32 light_id = -1;

    if (light_id == -1) {
      light_id = create_point_light();
    }

    auto* light = get_point_light(light_id);

    light->position = base.position + tVec3f(-10.f, -4.f, 8.f);
    light->radius = 300.f;
    light->color = tVec3f(1.f, 0.8f, 0.4f);
    light->power = 0.5f + 20.f * abs(state.astro_turn_speed);
    light->glow_power = 0.f;
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