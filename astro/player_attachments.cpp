#include "astro/player_attachments.h"
#include "astro/player_character.h"

using namespace astro;

static float GetSwingIntensity(Tachyon* tachyon, State& state, const float off_ladder_duration) {
  float swing_intensity = state.player_velocity.magnitude() / PlayerCharacter::MAX_RUN_SPEED;

  if (
    state.last_off_ladder_time != 0.f &&
    time_since(state.last_off_ladder_time) < off_ladder_duration
  ) {
    float alpha = time_since(state.last_off_ladder_time) / off_ladder_duration;

    swing_intensity += 1.f - alpha;
  }

  if (state.is_on_ladder) {
    float t = fmodf(state.player.rig.seek_time, 8.f) / 8.f;
    float alpha = 2.f * t * t_TAU;

    float speed = 3500.f * (0.5f + 0.5f * sinf(alpha));

    swing_intensity = speed / 3500.f;
  }

  return swing_intensity;
}

static void UpdateBlanket(Tachyon* tachyon, State& state) {
  float swing_intensity = GetSwingIntensity(tachyon, state, 1.5f);
  auto& player_animation = state.player.rig;
  auto& torso_bone = player_animation.active_pose.bones[8];
  auto& blanket = objects(state.meshes.player_blanket)[0];

  float bounce_t = player_animation.seek_time;
  if (bounce_t < 0.f) bounce_t += 8.f;
  float bounce_alpha = t_TAU * fmodf(bounce_t, 8.f) / 8.f;
  float bounce_angle = 0.6f * swing_intensity * swing_intensity * abs(sinf(bounce_alpha));
  Quaternion bounce_rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), bounce_angle);

  float swing_angle = 0.35f * swing_intensity * cosf(bounce_alpha);
  Quaternion swing_rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), swing_angle);

  Quaternion tilt_rotation = Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), -0.5f);

  tVec3f offset = tVec3f(0.f, 0.35f + bounce_angle * 0.1f, -0.48f);
  Quaternion base_rotation = state.player.rotation * torso_bone.rotation;
  tMat4f base_rotation_matrix = base_rotation.toMatrix4f();

  blanket.position = state.player.visual_position + base_rotation_matrix * (torso_bone.translation * tVec3f(1500.f));
  blanket.position += base_rotation_matrix * (offset * 1500.f);

  blanket.position.y += 150.f - 300.f * swing_angle;

  blanket.scale = tVec3f(1500.f);
  blanket.rotation = base_rotation * swing_rotation * tilt_rotation;

  blanket.color = 0x2220;
  blanket.material = tVec4f(1.f, 0, 0, 0.5f);

  commit(blanket);
}

static void UpdateSatchel(Tachyon* tachyon, State& state) {
  float swing_intensity = GetSwingIntensity(tachyon, state, 1.5f);
  auto& player_animation = state.player.rig;
  auto& torso_bone = player_animation.active_pose.bones[8];
  auto& satchel = objects(state.meshes.player_satchel)[0];

  float bounce_t = player_animation.seek_time;
  if (bounce_t < 0.f) bounce_t += 8.f;
  float bounce_alpha = t_TAU * fmodf(bounce_t, 8.f) / 8.f;
  float bounce_angle = 0.6f * swing_intensity * swing_intensity * abs(sinf(bounce_alpha));
  Quaternion bounce_rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), bounce_angle);

  float swing_angle = 0.15f * swing_intensity * cosf(bounce_alpha) + 3.f * state.tilt_angle;
  Quaternion swing_rotation = Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), swing_angle);

  tVec3f offset = tVec3f(0.f, 0.25f + bounce_angle * 0.1f, -0.325f);
  Quaternion rotation = state.player.rotation * torso_bone.rotation;
  tMat4f rotation_matrix = rotation.toMatrix4f();

  satchel.position = state.player.visual_position + rotation_matrix * (torso_bone.translation * tVec3f(1500.f));
  satchel.position += rotation_matrix * (offset * 1500.f);

  satchel.position.y += 200.f * bounce_angle;

  satchel.scale = tVec3f(1500.f);
  satchel.rotation = rotation * bounce_rotation * swing_rotation;

  satchel.color = 0x1110;
  satchel.material = tVec4f(0.8f, 0, 0, 0.2f);

  commit(satchel);
}

static void UpdateFlasks(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;
  auto& player_animation = state.player.rig;

  float swing_intensity = GetSwingIntensity(tachyon, state, 2.f);
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

    float speed_ratio = state.previous_move_delta / 90.f;

    lantern_swing += state.dt * speed_ratio;
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
  UpdateBlanket(tachyon, state);
  UpdateSatchel(tachyon, state);
  UpdateFlasks(tachyon, state);
  UpdateLantern(tachyon, state);
}