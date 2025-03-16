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