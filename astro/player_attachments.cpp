#include "astro/player_attachments.h"
#include "astro/player_character.h"

using namespace astro;

// @todo move to constants
static std::vector<float> run_bounce_curve = {
  0.1f,
  0.6f,
  0.9f,
  1.f,
  0.8f,
  0.4f,
  -0.3f,
  -0.2f
};

// @todo move to constants
static std::vector<float> climb_up_jump_hood_flop_curve = {
  // Climbing up
  0.f,
  -0.5f,
  -0.3f,
  -0.2f,
  0.f,
  // First jump
  -0.5f,
  -0.2f,
  // First fall
  0.5f,
  1.f,
  // Second jump
  0.f,
  -0.7f,
  0.f,
  0.7f
};

// @todo move elsewhere
// @todo allow spline sampling
static float SampleCurve(const std::vector<float>& curve, const float t) {
  int max = (int) curve.size();
  float max_time = (float) curve.size();
  float seek_time = t * float(max);
  if (seek_time < 0.f) seek_time += max_time;

  int start_frame = (int) seek_time;
  int end_frame = start_frame + 1;

  auto a = curve[start_frame % max];
  auto b = curve[end_frame % max];
  float alpha = fmodf(seek_time, 1.f);

  return Tachyon_Lerpf(a, b, alpha);
}

static void UpdateSatchel(Tachyon* tachyon, State& state) {
  auto& player_animation = state.player.rig;
  auto& torso_bone = player_animation.active_pose.bones[8];
  auto& satchel = objects(state.meshes.player_satchel)[0];

  // Tilt upward while in freefall
  float freefall_angle = 0.75f * state.player.satchel_freefall;
  Quaternion fall_rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), freefall_angle);

  // Swing as we turn
  float tilt_swing_angle = 5.f * state.tilt_angle;
  Quaternion tilt_swing_rotation = Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), tilt_swing_angle);

  tVec3f offset = tVec3f(0.f, 0.3f + freefall_angle * 0.1f, -0.35f);
  Quaternion rotation = state.player.rotation * torso_bone.rotation;
  tMat4f rotation_matrix = rotation.toMatrix4f();

  satchel.position = state.player.visual_position + rotation_matrix * (torso_bone.translation * tVec3f(1500.f));
  satchel.position += rotation_matrix * (offset * 1500.f);
  satchel.position.y += 100.f * freefall_angle;

  satchel.scale = tVec3f(1500.f);
  satchel.rotation = rotation * fall_rotation * tilt_swing_rotation;

  satchel.color = 0x1110;
  satchel.material = tVec4f(0.8f, 0, 0, 0.2f);

  commit(satchel);
}

static void UpdateBlanket(Tachyon* tachyon, State& state) {
  auto& player_animation = state.player.rig;
  auto& torso_bone = player_animation.active_pose.bones[8];
  auto& blanket = objects(state.meshes.player_blanket)[0];

  // Swing as we turn
  float tilt_swing_angle = 3.f * state.tilt_angle;
  Quaternion tilt_swing_rotation = Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), tilt_swing_angle);

  // Swing back and forth as we run
  float run_swing_angle = 0.5f * state.player.blanket_run_swing;
  Quaternion run_swing_rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), run_swing_angle);

  // The blanket should be partially tilted by default
  Quaternion blanket_tilt_rotation = Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), -0.35f);

  tVec3f offset = tVec3f(0.f, 0.45f + state.player.blanket_freefall * 0.1f, -0.48f);
  Quaternion base_rotation = state.player.rotation * torso_bone.rotation;
  tMat4f base_rotation_matrix = base_rotation.toMatrix4f();

  blanket.position = state.player.visual_position + base_rotation_matrix * (torso_bone.translation * 1500.f);
  blanket.position += base_rotation_matrix * (offset * 1500.f);

  blanket.scale = tVec3f(1500.f);
  blanket.rotation = base_rotation * tilt_swing_rotation * run_swing_rotation * blanket_tilt_rotation;

  blanket.color = 0x2220;
  blanket.material = tVec4f(1.f, 0, 0, 0.5f);

  commit(blanket);
}

static float GetSwingIntensity(Tachyon* tachyon, State& state) {
  if (state.is_on_ladder) {
    return abs(state.player.climb_speed) / 5000.f;
  }
  else if (PlayerCharacter::IsClimbingOffLadder(tachyon, state)) {
    float time_since_stopping_climb = time_since(state.player.last_climbing_stop_time);
    float swing_intensity = 1.f - time_since_stopping_climb / 2.f;

    clamp_to_0(swing_intensity);

    return swing_intensity;
  }
  else {
    float speed_ratio = state.player_velocity.magnitude() / PlayerCharacter::MAX_RUN_SPEED;

    return speed_ratio;
  }
}

static void UpdateFlasks(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;
  auto& player_animation = state.player.rig;
  float swing_intensity = GetSwingIntensity(tachyon, state);

  Quaternion side_swing_rotation = Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), 4.f * state.tilt_angle);

  // Flask 1
  {
    auto& flask = objects(meshes.player_flask)[0];

    float swing_angle = 0.75f * swing_intensity * sinf(get_scene_time() * 10.f);
    Quaternion swing_rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), swing_angle);
    tVec3f offset = tVec3f(600.f, 200.f + 50.f * swing_angle, 0.f);

    flask.position = state.player.visual_position + state.player.rotation_matrix * offset;
    flask.rotation = state.player.rotation * swing_rotation * side_swing_rotation;
    flask.scale = tVec3f(1750.f);
    flask.color.rgba = 0xA444;
    flask.material = tVec4f(0.2f, 0, 1.f, 0.5f);

    commit(flask);
  }

  // Flask 2
  {
    auto& flask = objects(meshes.player_flask)[1];

    float swing_angle = 0.6f * swing_intensity * sinf(get_scene_time() * 10.f - 1.25f);
    Quaternion swing_rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), swing_angle);
    tVec3f offset = tVec3f(610.f, 250.f + 50.f * swing_angle, -200.f);

    flask.position = state.player.visual_position + state.player.rotation_matrix * offset;
    flask.rotation = state.player.rotation * swing_rotation * side_swing_rotation;
    flask.scale = tVec3f(1500.f);
    flask.color.rgba = 0x64A4;
    flask.material = tVec4f(0.2f, 0, 1.f, 0.5f);

    commit(flask);
  }
}

static void UpdateLantern(Tachyon* tachyon, State& state) {
  // Lantern object
  {
    // @temporary
    // @todo define state for this!
    static float lantern_swing = 0.f;

    float swing_intensity = GetSwingIntensity(tachyon, state);

    lantern_swing += state.dt * swing_intensity;
    lantern_swing *= 1.f - state.dt;

    if (lantern_swing < 0.f) lantern_swing = 0.f;
    if (lantern_swing > 1.f) lantern_swing = 1.f;

    float lantern_angle = lantern_swing * sinf(get_scene_time() * 10.f);
    Quaternion lantern_rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), lantern_angle);

    auto& lantern = objects(state.meshes.player_lantern)[0];

    tVec3f offset = state.player.rotation_matrix * tVec3f(
      525.f,
      -100.f + 200.f * abs(lantern_angle),
      -100.f * lantern_angle
    );

    lantern.position = state.player_position + offset;
    lantern.scale = tVec3f(80.f, 120.f, 80.f);
    lantern.rotation = state.player.rotation * lantern_rotation;
    lantern.color = state.is_nighttime ? tVec4f(1.f, 0.7f, 0.3f, 1.f) : tVec4f(0.8f, 0.6f, 0.4f, 1.f);
    lantern.material = tVec4f(1.f, 0, 0, 1.f);

    commit(lantern);
  }

  // Point light source
  {
    auto& light = *get_point_light(state.player_light_id);

    tVec3f offset_from_player = state.player.rotation_matrix * tVec3f(710.f, 50.f, 0);

    // @todo body_position + offset
    light.position = state.player_position + offset_from_player;
    light.position.y -= 300.f;
    light.radius = state.is_nighttime ? 4000.f : 2500.f;
    light.color = tVec3f(0.5f, 0.3f, 0.6f);
    light.color = get_point_light(state.astrolabe_light_id)->color;
    light.power = state.is_nighttime ? 1.f : 0.5f;
    light.glow_power = 0.f;

    // @todo factor (Astrolabe::)
    if (time_since(state.game_time_at_start_of_turn) < 2.f) {
      float alpha = time_since(state.game_time_at_start_of_turn) / 2.f;

      light.power += sinf(alpha * t_PI);
    }
  }

  // Player light
  // @todo feature dynamic behavior
  auto& fx = tachyon->fx;

  fx.player_light_color = tVec3f(1.8f, 1.6f, 1.2f);
  fx.player_light_radius = 6000.f;
}

void PlayerAttachments::Update(Tachyon* tachyon, State& state) {
  // @todo cleanup
  bool is_climbing_off_ladder = PlayerCharacter::IsClimbingOffLadder(tachyon, state);
  float time_since_landing = time_since(state.player.last_freefall_landing_time);
  auto& rig = state.player.rig;

  if (time_since_landing < 0.1f) {
    float alpha = 1.f - time_since_landing / 0.1f;
    clamp_to_0(alpha);

    // Quickly reduce satchel/blanket freefall after landing
    state.player.satchel_freefall = state.player.airborne_freefall * alpha;
    state.player.blanket_freefall = state.player.airborne_freefall * alpha;
  }
  else if (state.is_on_ladder) {
    float satchel_freefall = abs(state.player.climb_speed * 0.0001f);

    state.player.satchel_freefall = Tachyon_Lerpf(
      state.player.satchel_freefall,
      satchel_freefall,
      15.f * state.dt
    );

    state.player.blanket_freefall = (
      state.player.blanket_freefall,
      0.f,
      5.f * state.dt
    );

    state.player.blanket_run_swing = Tachyon_Lerpf(
      state.player.blanket_run_swing,
      0.f,
      5.f * state.dt
    );
  }
  else if (is_climbing_off_ladder) {
    if (state.did_climb_down) {
      if (state.fall_velocity > 0.f) {
        // During the fall phase, increase freefall
        float satchel_freefall = state.fall_velocity * 0.0002f;
        float blanket_freefall = state.fall_velocity * 0.0004f;

        if (satchel_freefall > state.player.satchel_freefall) {
          state.player.satchel_freefall = satchel_freefall;
        }

        if (blanket_freefall > state.player.blanket_freefall) {
          state.player.blanket_freefall = state.fall_velocity * 0.0004f;
        }
      } else {
        // After landing, bring freefall to 0
        // @todo bounce easing
        state.player.satchel_freefall = Tachyon_Lerpf(
          state.player.satchel_freefall,
          0.f,
          15.f * state.dt
        );

        state.player.blanket_freefall = Tachyon_Lerpf(
          state.player.blanket_freefall,
          0.f,
          15.f * state.dt
        );
      }
    } else if (rig.current_animation == &state.animations.player_climb_up_jump) {
      float t = rig.current_animation_time;
      float max_time = (float) rig.current_animation->frames.size();
      float alpha = t / max_time;
      float sample = SampleCurve(climb_up_jump_hood_flop_curve, alpha);

      if (sample < 0.f) sample = 0.f;

      state.player.satchel_freefall = sample;
      state.player.blanket_freefall = 1.5f * sample;
    } else {
      // @temporary
      // @todo climb-up behavior
      state.player.satchel_freefall = 0.f;
      state.player.blanket_freefall = 0.f;
    }
  }
  else {
    float run_cycle_time = fmodf(state.player.rig.next_animation_time + 1.f, 8.f) / 8.f;

    float satchel_bounce = SampleCurve(run_bounce_curve, 2.f * run_cycle_time - 0.5f);
    float blanket_bounce = SampleCurve(run_bounce_curve, 2.f * run_cycle_time - 0.25f);
    float blanket_run_swing = sinf(t_TAU * run_cycle_time);

    state.player.satchel_freefall = state.run_oscillation * satchel_bounce;
    state.player.blanket_freefall = state.run_oscillation * blanket_bounce;
    state.player.blanket_run_swing = state.run_oscillation * blanket_run_swing;

    if (state.did_jump_off_ledge) {
      float t = time_since(state.player.last_ledge_jump_time);
      t -= state.player.ledge_jump_duration * 0.75f;
      clamp_to_0(t);

      float alpha = t / (t + 0.5f);

      state.player.airborne_freefall = 4.f * alpha;

      // Raise the satchel/blanket as we fall
      state.player.satchel_freefall += state.player.airborne_freefall;
      state.player.blanket_freefall += state.player.airborne_freefall;
    }
  }

  UpdateSatchel(tachyon, state);
  UpdateBlanket(tachyon, state);
  UpdateFlasks(tachyon, state);
  UpdateLantern(tachyon, state);
}