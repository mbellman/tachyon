#include "cosmodrone/lights.h"

using namespace Cosmodrone;

static void AddStaticLights(Tachyon* tachyon, State& state) {
  auto& point_lights = tachyon->point_lights;

  for (auto& bulb : objects(state.meshes.light_1_bulb)) {
    tVec3f offset = bulb.rotation.toMatrix4f() * tVec3f(0.f, 0.1f, 0);
    offset *= bulb.scale;

    point_lights.push_back({
      .position = bulb.position + offset,
      .radius = 2000.f,
      .color = tVec3f(1.f, 0.3f, 0.1f),
      .power = 1.f
    });
  }

  for (auto& bulb : objects(state.meshes.light_2_bulb)) {
    tVec3f offset = bulb.rotation.toMatrix4f() * tVec3f(0.f, 0.1f, 0);
    offset *= bulb.scale;

    point_lights.push_back({
      .position = bulb.position + offset,
      .radius = 2000.f,
      .color = tVec3f(0.1f, 0.4f, 1.f),
      .power = 1.f
    });
  }

  for (auto& bulb : objects(state.meshes.light_3_bulb)) {
    tVec3f offset = bulb.rotation.toMatrix4f() * tVec3f(0.f, 0.1f, 0);
    offset *= bulb.scale;

    point_lights.push_back({
      .position = bulb.position + offset,
      .radius = 7000.f,
      .color = tVec3f(0.5f, 1.f, 0.8f),
      .power = 1.f
    });
  }
}

static void AddBlinkingLights(Tachyon* tachyon, State& state) {
  auto& point_lights = tachyon->point_lights;

  for (auto& bulb : objects(state.meshes.light_4_bulb)) {
    tVec3f offset = bulb.rotation.toMatrix4f() * tVec3f(0.f, 0.1f, 0);
    offset *= bulb.scale;

    point_lights.push_back({
      .position = bulb.position + offset,
      .radius = 5000.f,
      .color = tVec3f(1.f, 0.6f, 0.2f),
      .power = 1.f
    });

    state.blinking_lights.push_back({
      .bulb = bulb,
      .light_index = uint32(point_lights.size() - 1)
    });
  }
}

static void AddMovingLights(Tachyon* tachyon, State& state) {
  auto& point_lights = tachyon->point_lights;

  for (auto& light : objects(state.meshes.station_drone_light)) {
    point_lights.push_back({
      .position = light.position,
      .radius = 5000.f,
      .color = tVec3f(0.2f, 0.5f, 1.f),
      .power = 1.f
    });

    state.moving_lights.push_back({
      .light_object = light,
      .light_index = uint32(point_lights.size() - 1)
    });
  }

  for (auto& light : objects(state.meshes.procedural_elevator_car_light)) {
    point_lights.push_back({
      .position = light.position,
      .radius = 5000.f,
      .color = tVec3f(1.f, 0.8f, 0.6f),
      .power = 1.f
    });

    state.moving_lights.push_back({
      .light_object = light,
      .light_index = uint32(point_lights.size() - 1)
    });
  }
}

static void AddGasFlareLights(Tachyon* tachyon, State& state) {
  auto& point_lights = tachyon->point_lights;

  for (auto& flare : objects(state.meshes.gas_flare_1_spawn)) {
    point_lights.push_back({
      // @todo @fix this repositioning breaks light syncing
      .position = flare.position - flare.rotation.getUpDirection() * flare.scale.y * 0.9f,
      .radius = 40000.f,
      .color = tVec3f(1.f, 0.5f, 0.1f),
      .power = 3.f
    });

    state.gas_flare_light_indexes.push_back(point_lights.size() - 1);
  }
}

static void AddSolarRotatorLights(Tachyon* tachyon, State& state) {
  auto& point_lights = tachyon->point_lights;

  for (auto& rotator : objects(state.meshes.solar_rotator_2_body)) {
    point_lights.push_back({
      .position = rotator.position,
      .radius = 30000.f,
      .color = tVec3f(1.f, 0, 0),
      .power = 3.f
    });

    point_lights.push_back({
      .position = rotator.position,
      .radius = 30000.f,
      .color = tVec3f(1.f, 0, 0),
      .power = 3.f
    });

    state.solar_rotator_lights.push_back({
      .rotator = rotator,
      .light_index_1 = uint32(point_lights.size() - 2),
      .light_index_2 = uint32(point_lights.size() - 1)
    });
  }
}

static void UpdateBlinkingLights(Tachyon* tachyon, State& state) {
  for (auto& blinking_light : state.blinking_lights) {
    auto& light = tachyon->point_lights[blinking_light.light_index];
    auto& bulb = *get_live_object(blinking_light.bulb);
    auto power = 0.5f * sinf(state.current_game_time * 3.f + light.position.x * 0.03f) + 0.5f;
    power = powf(power, 5.f);

    light.power = power;
    bulb.color = tVec4f(1.f, 0.5f, 0.2f, power);

    commit(bulb);
  }
}

// @todo handle multiple moving lights per object
static void UpdateMovingLights(Tachyon* tachyon, State& state) {
  for (auto& moving_light : state.moving_lights) {
    auto& light = tachyon->point_lights[moving_light.light_index];
    auto& bulb = *get_live_object(moving_light.light_object);

    light.position = bulb.position;

    if (bulb.mesh_index == state.meshes.station_drone_light) {
      // @optimize
      light.position = bulb.position + bulb.rotation.getDirection() * 1500.f;
    }

    if (bulb.mesh_index == state.meshes.procedural_elevator_car_light) {
      // @optimize
      light.position =
        bulb.position +
        bulb.rotation.toMatrix4f() * (tVec3f(0, 1.f, -0.5f) * 3400.f);
    }
  }
}

static void UpdateGasFlareLights(Tachyon* tachyon, State& state) {
  for (auto light_index : state.gas_flare_light_indexes) {
    auto& light = tachyon->point_lights[light_index];
    auto t = state.current_game_time * 0.5f + light.position.x;
    auto power = 5.f * (0.5f * sinf(t) + 0.5f);

    light.power = power;
  }
}

static void UpdateSolarRotatorLights(Tachyon* tachyon, State& state) {
  for (auto& lights : state.solar_rotator_lights) {
    auto& rotator = *get_live_object(lights.rotator);
    auto matrix = rotator.rotation.toMatrix4f();
    auto& light_1 = tachyon->point_lights[lights.light_index_1];
    auto& light_2 = tachyon->point_lights[lights.light_index_2];

    light_1.position =
      rotator.position +
      matrix * (rotator.scale * tVec3f(0, 0, 1.f) * 2.625f);

    light_2.position =
      rotator.position +
      matrix * (rotator.scale * tVec3f(0, 0, -1.f) * 2.625f);
  }
}

void Lights::InitLights(Tachyon* tachyon, State& state) {
  auto& point_lights = tachyon->point_lights;

  state.gas_flare_light_indexes.clear();
  state.blinking_lights.clear();
  state.moving_lights.clear();
  state.solar_rotator_lights.clear();

  // @todo only clear generated lights
  tachyon->point_lights.clear();

  AddStaticLights(tachyon, state);
  AddBlinkingLights(tachyon, state);
  AddMovingLights(tachyon, state);
  AddGasFlareLights(tachyon, state);
  AddSolarRotatorLights(tachyon, state);
}

void Lights::UpdateLights(Tachyon* tachyon, State& state) {
  UpdateBlinkingLights(tachyon, state);
  UpdateMovingLights(tachyon, state);
  UpdateGasFlareLights(tachyon, state);
  UpdateSolarRotatorLights(tachyon, state);
}