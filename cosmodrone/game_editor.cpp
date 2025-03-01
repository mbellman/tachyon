#include <format>
#include <math.h>
#include <vector>

#include "cosmodrone/game_editor.h"
#include "cosmodrone/mesh_library.h"
#include "cosmodrone/object_behavior.h"
#include "cosmodrone/procedural_generation.h"
#include "cosmodrone/world_behavior.h"
#include "cosmodrone/world_setup.h"

#define case(value, __code)\
  case value: {\
    __code\
    break;\
  }\

using namespace Cosmodrone;

constexpr static float PITCH_LIMIT = t_HALF_PI * 0.99f;

enum ActionType {
  POSITION,
  ROTATE,
  SCALE
};

struct EditorState {
  tUIText* editor_font = nullptr;

  bool is_object_picker_active = false;
  uint16 object_picker_index = 0;
  float object_picker_cycle_speed = 0.f;
  float last_object_picker_cycle_time = 0.f;

  bool use_modified_action = false;
  float running_angle_x = 0.f;
  float running_angle_y = 0.f;

  bool was_object_selected_on_mouse_down = false;
  ActionType action_type = ActionType::POSITION;

  std::vector<tObject> selected_objects;
  tVec3f selected_objects_origin;

  bool use_high_speed_camera_movement = false;
  float last_pressed_space_time = 0.f;
} editor;

static bool AreObjectsSelected() {
  return editor.selected_objects.size() > 0;
}

static const MeshAsset& GetSelectedObjectPickerMeshAsset() {
  auto& placeable_meshes = MeshLibrary::GetPlaceableMeshAssets();
  auto& selected_mesh = placeable_meshes[editor.object_picker_index];

  return selected_mesh;
}

static const MeshAsset& GetPlaceableMeshAssetByMeshIndex(uint16 mesh_index) {
  auto& placeable_meshes = MeshLibrary::GetPlaceableMeshAssets();

  for (auto& asset : placeable_meshes) {
    if (asset.mesh_index == mesh_index) {
      return asset;
    }
  }

  return placeable_meshes[0];
}

static const MeshAsset& GetPlaceableMeshAssetByPickerIndexOffset(int16 offset) {
  auto& placeable_meshes = MeshLibrary::GetPlaceableMeshAssets();
  auto max_index = int16(placeable_meshes.size() - 1);
  int16 signed_index = (int16)editor.object_picker_index;
  int16 target_index = signed_index + offset;

  if (target_index < 0) {
    target_index = max_index + target_index + 1;
  }

  if (target_index > max_index) {
    target_index = (target_index - max_index) - 1;
  }

  return placeable_meshes[target_index];
}

static tVec3f GetMostSimilarGlobalAxis(const tVec3f& vector) {
  float abs_x = abs(vector.x);
  float abs_y = abs(vector.y);
  float abs_z = abs(vector.z);

  if (abs_x > abs_y && abs_x > abs_z) {
    return tVec3f(vector.x, 0, 0).unit();
  } else if (abs_y > abs_x && abs_y > abs_z) {
    return tVec3f(0, vector.y, 0).unit();
  } else {
    return tVec3f(0, 0, vector.z).unit();
  }
}

static tVec3f GetMostSimilarObjectAxis(const tVec3f& vector, const tObject& object) {
  tVec3f object_up = object.rotation.getUpDirection();
  tVec3f object_right = object.rotation.getLeftDirection().invert();
  tVec3f object_forward = object.rotation.getDirection();

  float dot_up = tVec3f::dot(vector, object_up);
  float dot_right = tVec3f::dot(vector, object_right);
  float dot_forward = tVec3f::dot(vector, object_forward);

  float up_factor = abs(dot_up);
  float right_factor = abs(dot_right);
  float forward_factor = abs(dot_forward);

  if (up_factor > right_factor && up_factor > forward_factor) {
    return dot_up < 0.f ? object_up.invert() : object_up;
  } else if (right_factor > up_factor && right_factor > forward_factor) {
    return dot_right < 0.f ? object_right.invert() : object_right;    
  } else {
    return dot_forward < 0.f ? object_forward.invert() : object_forward;
  }
}

static inline std::string Serialize(float f) {
  return std::format("{:.3f}", f);
}

static inline std::string Serialize(const tVec3f& vector) {
  return Serialize(vector.x) + "," + Serialize(vector.y) + "," + Serialize(vector.z);
}

static inline std::string Serialize(const Quaternion& quaternion) {
  return (
    Serialize(quaternion.w) + "," +
    Serialize(quaternion.x) + "," +
    Serialize(quaternion.y) + "," +
    Serialize(quaternion.z)
  );
}

static void SaveWorldData(Tachyon* tachyon, State& state) {
  auto start = Tachyon_GetMicroseconds();
  auto& placeable_meshes = MeshLibrary::GetPlaceableMeshAssets();
  std::string data = "";

  for (auto& mesh : placeable_meshes) {
    auto& instances = objects(mesh.mesh_index);

    data += ("@" + mesh.mesh_name + "\n");

    // Perform a sequence-preserving loop over the objects
    // to ensure they always serialize in the same order
    // (e.g. if objects are removed/shuffled during runtime)
    for (uint16 id = 0; id <= instances.highest_used_id; id++) {
      tObject* instance = instances.getById(id);

      if (instance != nullptr) {
        data += Serialize(instance->position) + ",";
        data += Serialize(instance->rotation) + ",";
        data += std::to_string(instance->color.rgba) + "\n";
      }
    }
  }

  #if USE_PROCEDURAL_GENERATION == 1
    Tachyon_WriteFileContents("./cosmodrone/data/world_2.txt", data);
  #else
    Tachyon_WriteFileContents("./cosmodrone/data/world.txt", data);
  #endif

  auto save_time = Tachyon_GetMicroseconds() - start;
  auto message = std::format("Saved world data in {}us", save_time);

  add_console_message(message, tVec3f(1.f));
}

static void HandleCamera(Tachyon* tachyon, State& state, const float dt) {
  if (!is_window_focused()) {
    return;
  }

  if (AreObjectsSelected() && is_mouse_held_down()) {
    return;
  }

  auto& camera = tachyon->scene.camera;
  float slerp_alpha = 30.f * dt;

  if (tachyon->mouse_delta_x != 0 || tachyon->mouse_delta_y != 0 || slerp_alpha > 1.f) {
    slerp_alpha = 1.f;
  }

  // Handle mouse movements
  {
    if (is_key_held(tKey::SHIFT) && AreObjectsSelected()) {
      auto offset = camera.position - editor.selected_objects_origin;
      auto unit_offset = offset.unit();

      tCamera3p camera3p;
      camera3p.radius = offset.magnitude();
      camera3p.azimuth = atan2f(unit_offset.z, unit_offset.x);
      camera3p.altitude = atan2f(unit_offset.y, unit_offset.xz().magnitude());

      if (tachyon->mouse_delta_x != 0 || tachyon->mouse_delta_y != 0) {
        camera3p.azimuth += (float)tachyon->mouse_delta_x / 1000.f;
        camera3p.altitude += (float)tachyon->mouse_delta_y / 1000.f;
        camera3p.limitAltitude(0.99f);

        camera.position = editor.selected_objects_origin + camera3p.calculatePosition();
      }

      camera.orientation.face(editor.selected_objects_origin - camera.position, tVec3f(0, 1.f, 0));
    } else {
      camera.orientation.yaw += (float)tachyon->mouse_delta_x / 1000.f;
      camera.orientation.pitch += (float)tachyon->mouse_delta_y / 1000.f;
    }

    if (camera.orientation.pitch > PITCH_LIMIT) camera.orientation.pitch = PITCH_LIMIT;
    else if (camera.orientation.pitch < -PITCH_LIMIT) camera.orientation.pitch = -PITCH_LIMIT;
  }

  // Handle WASD controls
  {
    const float move_speed =
      (is_key_held(tKey::SPACE) && is_key_held(tKey::SHIFT)) ? 1000000.f :
      editor.use_high_speed_camera_movement ? 300000.f :
      is_key_held(tKey::SPACE) ? 50000.f :
      2000.f;

    if (is_key_held(tKey::W)) {
      camera.position += camera.orientation.getDirection() * dt * move_speed;
    } else if (is_key_held(tKey::S)) {
      camera.position += camera.orientation.getDirection() * -dt * move_speed;
    }

    if (is_key_held(tKey::A)) {
      camera.position += camera.orientation.getLeftDirection() * dt * move_speed;
    } else if (is_key_held(tKey::D)) {
      camera.position += camera.orientation.getRightDirection() * dt * move_speed;
    }
  }

  camera.rotation = Quaternion::slerp(camera.rotation, camera.orientation.toQuaternion(), slerp_alpha);
}

static void CycleObjectPickerLeft(int16 max) {
  if (editor.object_picker_index == 0) {
    editor.object_picker_index = max;
  } else {
    editor.object_picker_index--;
  }
}

static void CycleObjectPickerRight(int16 max) {
  if (editor.object_picker_index == max) {
    editor.object_picker_index = 0;
  } else {
    editor.object_picker_index++;
  }
}

static void HandleObjectPickerInputs(Tachyon* tachyon, const float dt) {
  auto& placeable_meshes = MeshLibrary::GetPlaceableMeshAssets();
  auto max = placeable_meshes.size() - 1;

  if (did_press_key(tKey::Q)) {
    CycleObjectPickerLeft(max);
  }

  if (did_press_key(tKey::E)) {
    CycleObjectPickerRight(max);
  }

  float time_since_last_cycle = tachyon->running_time - editor.last_object_picker_cycle_time;

  if (is_key_held(tKey::Q)) {
    editor.object_picker_cycle_speed -= 5.f * dt;

    if (time_since_last_cycle > 1.f / abs(editor.object_picker_cycle_speed)) {
      CycleObjectPickerLeft(max);
    }
  }

  if (is_key_held(tKey::E)) {
    editor.object_picker_cycle_speed += 5.f * dt;

    if (time_since_last_cycle > 1.f / abs(editor.object_picker_cycle_speed)) {
      CycleObjectPickerRight(max);
    }
  }
}

static void RenderObjectPickerList(Tachyon* tachyon) {
  for (int16 i = -10; i < 10; i++) {
    float alpha = powf(1.f - abs(i) / 11.f, 5.f);

    Tachyon_DrawUIText(tachyon, editor.editor_font, {
      .screen_x = tachyon->window_width - 300,
      .screen_y = tachyon->window_height / 2 + i * 25,
      .centered = false,
      .alpha = alpha,
      .string = GetPlaceableMeshAssetByPickerIndexOffset(i).mesh_name
    });
  }
}

static void HandleObjectPickerCycleChange(Tachyon* tachyon) {
  if (editor.selected_objects.size() > 1) {
    return;
  }

  auto& camera = tachyon->scene.camera;

  tVec3f spawn_position = camera.position + camera.orientation.getDirection() * 4000.f;

  if (editor.is_object_picker_active && editor.selected_objects.size() == 1) {
    spawn_position = get_live_object(editor.selected_objects[0])->position;

    remove(editor.selected_objects[0]);
  }

  if (!editor.is_object_picker_active) {
    editor.is_object_picker_active = true;
  }

  auto& selected_mesh = GetSelectedObjectPickerMeshAsset();
  auto mesh_index = selected_mesh.mesh_index;
  auto& selected = create(mesh_index);

  selected.position = spawn_position;
  selected.scale = selected_mesh.defaults.scale;
  selected.rotation = Quaternion(1.f, 0, 0, 0);
  selected.color = selected_mesh.defaults.color;
  selected.material = selected_mesh.defaults.material;

  if (editor.selected_objects.size() == 0) {
    editor.selected_objects.push_back(selected);
  } else {
    editor.selected_objects[0] = selected;
  }

  editor.selected_objects_origin = spawn_position;
  editor.last_object_picker_cycle_time = tachyon->running_time;
}

static void HandleActionTypeCycleChange(Tachyon* tachyon, int8 change) {
  if (change > 0) {
    switch (editor.action_type) {
      case(ActionType::POSITION, {
        editor.action_type = ActionType::ROTATE;
      })
      case(ActionType::ROTATE, {
        editor.action_type = ActionType::SCALE;
      })
      case(ActionType::SCALE, {
        editor.action_type = ActionType::POSITION;
      })
    }
  } else {
    switch (editor.action_type) {
      case(ActionType::POSITION, {
        editor.action_type = ActionType::SCALE;
      })
      case(ActionType::ROTATE, {
        editor.action_type = ActionType::POSITION;
      })
      case(ActionType::SCALE, {
        editor.action_type = ActionType::ROTATE;
      })
    }
  }
}

static void HandleRotationSnapping(float& running_angle, float& angle) {
  constexpr static float SNAP_INCREMENT = t_PI / 12.f;

  if (abs(running_angle) > SNAP_INCREMENT) {
    angle = running_angle > 0.f ? SNAP_INCREMENT : -SNAP_INCREMENT;

    running_angle = 0.f;
  } else {
    angle = 0.f;
  }
}

static void HandleSelectedObjectMouseMovements(Tachyon* tachyon) {
  auto& camera = tachyon->scene.camera;
  auto& first_selected = *get_live_object(editor.selected_objects[0]);
  auto is_horizontal_action = abs(tachyon->mouse_delta_x) > abs(tachyon->mouse_delta_y);
  auto camera_up = camera.orientation.getUpDirection();
  auto camera_right = camera.orientation.getRightDirection();

  switch (editor.action_type) {
    case(ActionType::POSITION, {
      auto distance = (editor.selected_objects_origin - camera.position).magnitude();
      auto movement_factor = distance / 5000.f;
      auto use_object_axis = editor.use_modified_action;

      if (is_key_held(tKey::CONTROL)) {
        // Hold CONTROL for fine repositioning
        movement_factor *= 0.1f;
      }

      tVec3f offset;

      if (is_horizontal_action) {
        auto axis = use_object_axis 
          ? GetMostSimilarObjectAxis(camera_right, first_selected)
          : GetMostSimilarGlobalAxis(camera_right);

        offset = axis * (float)tachyon->mouse_delta_x * movement_factor;
      } else {
        auto axis = use_object_axis
          ? GetMostSimilarObjectAxis(camera_up, first_selected)
          : tVec3f(0, 1.f, 0);

        offset = axis * -(float)tachyon->mouse_delta_y * movement_factor;
      }

      for (auto& object : editor.selected_objects) {
        auto& selected = *get_live_object(object);

        selected.position += offset;
      }

      editor.selected_objects_origin += offset;
    })
    case(ActionType::ROTATE, {
      constexpr static float SNAP_INCREMENT = t_PI / 12.f;
      auto use_snapping = editor.use_modified_action;

      if (is_horizontal_action) {
        auto axis = GetMostSimilarObjectAxis(camera_up, first_selected);
        auto angle = (float)tachyon->mouse_delta_x * 0.005f;

        editor.running_angle_x += angle;

        if (use_snapping) {
          HandleRotationSnapping(editor.running_angle_x, angle);
        }

        for (auto& object : editor.selected_objects) {
          auto& selected = *get_live_object(object);

          selected.rotation *= Quaternion::fromAxisAngle(axis, angle);
        }
      } else {
        auto axis = GetMostSimilarObjectAxis(camera_right, first_selected);
        auto angle = (float)tachyon->mouse_delta_y * 0.005f;

        editor.running_angle_y += angle;

        if (use_snapping) {
          HandleRotationSnapping(editor.running_angle_y, angle);
        }

        for (auto& object : editor.selected_objects) {
          auto& selected = *get_live_object(object);

          selected.rotation *= Quaternion::fromAxisAngle(axis, angle);
        }
      }
    })
    case(ActionType::SCALE, {
      // @todo (may just remove this)
    })
  }
}

static void ResetSelectedObjectTransform(Tachyon* tachyon) {
  auto& selected = *get_live_object(editor.selected_objects[0]);

  switch (editor.action_type) {
    case(ActionType::ROTATE, {
      selected.rotation = Quaternion(1.f, 0, 0, 0);
    })
    case(ActionType::SCALE, {
      selected.scale = tVec3f(1000.f);
    })
  }
}

static void RestoreSelectedObjectColors(Tachyon* tachyon) {
  for (auto& object : editor.selected_objects) {
    auto& live = *get_live_object(object);

    live.color = object.color;

    commit(live);
  }
}

static void RecalculateSelectedObjectsOrigin(Tachyon* tachyon) {
  tVec3f average;

  for (auto& object : editor.selected_objects) {
    auto& live = *get_live_object(object);

    average += live.position;
  }

  editor.selected_objects_origin = average / float(editor.selected_objects.size());
}

enum Direction {
  UP, RIGHT, LEFT, DOWN
};

static void CopySelectedObjects(Tachyon* tachyon, State& state, Direction direction) {
  RestoreSelectedObjectColors(tachyon);

  auto& camera = tachyon->scene.camera;
  auto& first_selected = *get_live_object(editor.selected_objects[0]);
  tVec3f move_distance = first_selected.scale;

  if (editor.selected_objects.size() == 1) {
    auto& copy = first_selected;

    // @todo refactor
    if (
      copy.mesh_index == state.meshes.girder_1 ||
      copy.mesh_index == state.meshes.girder_1b ||
      copy.mesh_index == state.meshes.girder_2 ||
      copy.mesh_index == state.meshes.girder_3 ||
      copy.mesh_index == state.meshes.girder_4 ||
      copy.mesh_index == state.meshes.girder_5 ||
      copy.mesh_index == state.meshes.mega_girder_2 ||
      copy.mesh_index == state.meshes.beam_1 ||
      copy.mesh_index == state.meshes.beam_2 ||
      copy.mesh_index == state.meshes.silo_7 ||
      copy.mesh_index == state.meshes.radio_tower_1
    ) {
      move_distance *= 1.99f;
    }

    // @todo refactor
    if (copy.mesh_index == state.meshes.silo_3) {
      move_distance *= 2.05f;
    }

    // @todo refactor
    if (copy.mesh_index == state.meshes.silo_2) {
      move_distance *= 1.4f;
    }
  }

  for (auto& object : editor.selected_objects) {
    auto& original = *get_live_object(object);
    auto& copy = create(original.mesh_index);

    copy.rotation = original.rotation;
    copy.scale = original.scale;
    copy.color = original.color;
    copy.material = original.material;

    tVec3f axis;

    switch (direction) {
      case(LEFT, {
        axis = GetMostSimilarObjectAxis(camera.orientation.getLeftDirection(), first_selected);
      })
      case(RIGHT, {
        axis = GetMostSimilarObjectAxis(camera.orientation.getRightDirection(), first_selected);
      })
      case(UP, {
        axis = GetMostSimilarObjectAxis(camera.orientation.getUpDirection(), first_selected);
      })
      case(DOWN, {
        axis = GetMostSimilarObjectAxis(camera.orientation.getUpDirection().invert(), first_selected);
      })
    }

    copy.position = original.position + axis * move_distance;

    commit(copy);

    object = copy;
  }

  RecalculateSelectedObjectsOrigin(tachyon);

  // Save when objects are copied
  SaveWorldData(tachyon, state);
}

static void MaybeSelectObject(Tachyon* tachyon) {
  auto& placeable_meshes = MeshLibrary::GetPlaceableMeshAssets();
  auto& camera = tachyon->scene.camera;
  auto forward = camera.orientation.getDirection();
  float highest_candidate_score = 0.f;
  tObject candidate;

  for (auto& mesh : placeable_meshes) {
    auto& instances = objects(mesh.mesh_index);

    if (instances.disabled) {
      continue;
    }

    for (auto& object : instances) {
      auto camera_to_object = object.position - camera.position;
      auto object_dot = tVec3f::dot(forward, camera_to_object.unit());
      auto distance = camera_to_object.magnitude();
      auto scale_limit = std::clamp(object.scale.x * 3.f, 50000.f, 1000000.f);

      if (distance > scale_limit || object_dot < 0.6f) continue;

      auto score = (100.f * powf(object_dot, 20.f)) / distance;

      if (score > highest_candidate_score) {
        highest_candidate_score = score;
        candidate = object;
      }
    }
  }

  if (highest_candidate_score > 0.f) {
    if (!is_key_held(tKey::ALT)) {
      editor.selected_objects.clear();
    }

    bool already_selected = false;

    for (auto& object : editor.selected_objects) {
      if (candidate == object) {
        already_selected = true;

        break;
      }
    }

    if (!already_selected) {
      editor.selected_objects.push_back(candidate);

      RecalculateSelectedObjectsOrigin(tachyon);
    }

    if (editor.selected_objects.size() > 1) {
      editor.is_object_picker_active = false;
    }
  }
}

// @todo CTRL-Z (?)
static void HandleInputs(Tachyon* tachyon, State& state, const float dt) {
  auto initial_object_picker_index = editor.object_picker_index;

  if (editor.is_object_picker_active) {
    HandleObjectPickerInputs(tachyon, dt);
  }

  // Cycling through objects
  if (
    did_press_key(tKey::Q) || did_press_key(tKey::E) ||
    editor.object_picker_index != initial_object_picker_index
  ) {
    HandleObjectPickerCycleChange(tachyon);
  }

  if (!is_key_held(tKey::Q) && !is_key_held(tKey::E)) {
    editor.object_picker_cycle_speed *= 1.f - 5.f * dt;
  }

  // Changing the action type
  if (did_wheel_down()) {
    HandleActionTypeCycleChange(tachyon, +1);
  } else if (did_wheel_up()) {
    HandleActionTypeCycleChange(tachyon, -1);
  }

  // Handling selected objects
  if (AreObjectsSelected()) {
    if (is_mouse_held_down() && editor.was_object_selected_on_mouse_down) {
      HandleSelectedObjectMouseMovements(tachyon);
    }

    // @todo use a different key, as ENTER also toggles lighting (or change that hotkey)
    if (did_press_key(tKey::ENTER) && editor.selected_objects.size() == 1) {
      ResetSelectedObjectTransform(tachyon);
    }

    // @todo DeleteSelectedObjects()
    if (did_press_key(tKey::BACKSPACE)) {
      for (auto& object: editor.selected_objects) {
        remove(object);
      }

      editor.is_object_picker_active = false;
      editor.selected_objects.clear();

      // Save when objects are deleted
      SaveWorldData(tachyon, state);
    }

    if (did_press_key(tKey::ARROW_LEFT)) {
      CopySelectedObjects(tachyon, state, LEFT);
    }

    if (did_press_key(tKey::ARROW_RIGHT)) {
      CopySelectedObjects(tachyon, state, RIGHT);
    }

    if (did_press_key(tKey::ARROW_UP)) {
      CopySelectedObjects(tachyon, state, UP);
    }

    if (did_press_key(tKey::ARROW_DOWN)) {
      CopySelectedObjects(tachyon, state, DOWN);
    }

    // @temporary
    // @todo implement a color picker, or color controls
    if (did_press_key(tKey::J) && editor.selected_objects.size() == 1) {
      editor.selected_objects[0].color = tVec3f(1.f, 0.1f, 0.1f);
    }
  }

  // Object selection
  if (did_left_click_down()) {
    if (editor.selected_objects.size() > 0 && !is_key_held(tKey::ALT)) {
      editor.was_object_selected_on_mouse_down = true;
    } else {
      MaybeSelectObject(tachyon);
    }
  }

  if(did_left_click_up()) {
    editor.was_object_selected_on_mouse_down = false;
  }

  if (!AreObjectsSelected() && is_key_held(tKey::ARROW_LEFT)) {
    // Fast-rewind time
    state.current_game_time -= 500.f * dt;

    WorldBehavior::UpdateWorld(tachyon, state, 0.f);
  }

  if (!AreObjectsSelected() && is_key_held(tKey::ARROW_RIGHT)) {
    // Fast-forward time
    state.current_game_time += 500.f * dt;

    WorldBehavior::UpdateWorld(tachyon, state, 0.f);
  }

  if (!AreObjectsSelected() && did_press_key(tKey::R)) {
    // Respawn
    // @todo factor
    auto& camera = tachyon->scene.camera;

    state.ship_position = camera.position + camera.orientation.getDirection() * 1000.f;
    state.ship_velocity = tVec3f(0.f);

    auto& hull = objects(state.meshes.hull)[0];
    auto& trim = objects(state.meshes.trim)[0];
    auto& streams = objects(state.meshes.streams)[0];
    auto& thrusters = objects(state.meshes.thrusters)[0];

    hull.position = state.ship_position;
    trim.position = state.ship_position;
    streams.position = state.ship_position;
    thrusters.position = state.ship_position;

    commit(hull);
    commit(trim);
    commit(streams);
    commit(thrusters);
  }

  // Toggle global/local transformations
  if (did_press_key(tKey::C)) {
    editor.use_modified_action = !editor.use_modified_action;
  }

  // Toggle guidelines
  if (did_press_key(tKey::G)) {
    objects(state.meshes.editor_guideline).disabled = !objects(state.meshes.editor_guideline).disabled;
  }

  // Toggle high-visibility mode
  if (did_press_key(tKey::ENTER)) {
    tachyon->use_high_visibility_mode = !tachyon->use_high_visibility_mode;
  }

  // Activate high-speed camera movement with SPACE+SPACE
  {
    if (did_press_key(tKey::SPACE)) {
      if (tachyon->running_time - editor.last_pressed_space_time < 0.3f) {
        editor.use_high_speed_camera_movement = true;
      }

      editor.last_pressed_space_time = tachyon->running_time;
    }

    if (!is_key_held(tKey::SPACE)) {
      editor.use_high_speed_camera_movement = false;
    }
  }
}

static void HandleSelectedObjects(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  auto& first_selected = *get_live_object(editor.selected_objects[0]);
  bool should_flash = uint32(tachyon->running_time * 2.f) % 2;

  for (auto& object : editor.selected_objects) {
    auto& selected = *get_live_object(object);

    selected.color = object.color;
    selected.color.rgba &= should_flash ? 0xF0F0 : 0xFFF0;
    selected.color.rgba |= should_flash ? 0x0006 : 0x0001;

    commit(selected);
  }

  // @todo refactor
  {
    objects(state.meshes.editor_position).disabled = true;
    objects(state.meshes.editor_rotation).disabled = true;
    objects(state.meshes.editor_scale).disabled = true;

    if (editor.action_type == ActionType::POSITION) {
      objects(state.meshes.editor_position).disabled = false;

      auto& indicator = objects(state.meshes.editor_position)[0];
      auto selected_object_direction = (editor.selected_objects_origin - camera.position).unit();

      indicator.position = camera.position + selected_object_direction * 600.f;
      indicator.scale = tVec3f(90.f);

      if (editor.use_modified_action) {
        indicator.rotation = first_selected.rotation;
        indicator.color = tVec4f(1.f, 1.f, 0.f, 1.f);
      } else {
        indicator.rotation = Quaternion(1.f, 0, 0, 0);
        indicator.color = tVec4f(1.f);
      }

      commit(indicator);
    } else if (editor.action_type == ActionType::ROTATE) {
      objects(state.meshes.editor_rotation).disabled = false;

      auto& indicator = objects(state.meshes.editor_rotation)[0];
      auto selected_object_direction = (editor.selected_objects_origin - camera.position).unit();

      indicator.position = camera.position + selected_object_direction * 600.f;
      indicator.rotation = first_selected.rotation;
      indicator.scale = tVec3f(90.f);

      if (editor.use_modified_action) {
        indicator.color = tVec4f(1.f, 1.f, 0.f, 1.f);
      } else {
        indicator.color = tVec4f(1.f);
      }

      commit(indicator);
    } else if (editor.action_type == ActionType::SCALE) {
      objects(state.meshes.editor_scale).disabled = false;

      auto& indicator = objects(state.meshes.editor_scale)[0];
      auto selected_object_direction = (editor.selected_objects_origin - camera.position).unit();

      indicator.position = camera.position + selected_object_direction * 600.f;
      indicator.color = tVec4f(1.f, 1.f, 1.f, 1.f);
      indicator.rotation = first_selected.rotation;
      indicator.scale = tVec3f(90.f);

      commit(indicator);
    }
  }

  // @todo move to HandleInputs()
  // @todo DeselectObject()
  if (did_right_click_down()) {
    RestoreSelectedObjectColors(tachyon);

    editor.is_object_picker_active = false;
    editor.selected_objects.clear();

    // Save when deselecting an object
    SaveWorldData(tachyon, state);
  }

  if (editor.selected_objects.size() > 1) {
    return;
  }

  // @todo factor -> RenderMeshStats()
  auto mesh_name = GetPlaceableMeshAssetByMeshIndex(first_selected.mesh_index).mesh_name;
  auto generated_meshes = MeshLibrary::GetGeneratedMeshAssets();
  auto& record = tachyon->mesh_pack.mesh_records[first_selected.mesh_index];
  // @todo properly count based on active LoDs
  auto total_vertices = record.lod_1.vertex_end - record.lod_1.vertex_start;
  auto total_triangles = (record.lod_1.face_element_end - record.lod_1.face_element_start) / 3;
  auto total_instances = record.group.total_active;

  uint32 total_lod_2_vertices = 0;
  uint32 total_lod_2_triangles = 0;
  uint32 total_lod_3_vertices = 0;
  uint32 total_lod_3_triangles = 0;

  for (auto& asset : generated_meshes) {
    if (asset.generated_from == first_selected.mesh_index) {
      auto& mesh = mesh(asset.mesh_index);
      auto& lod_2 = mesh.lod_2;
      auto& lod_3 = mesh.lod_3;

      total_lod_2_vertices += lod_2.vertex_end - lod_2.vertex_start;
      total_lod_2_triangles += (lod_2.face_element_end - lod_2.face_element_start) / 3;

      total_lod_3_vertices += lod_3.vertex_end - lod_3.vertex_start;
      total_lod_3_triangles += (lod_3.face_element_end - lod_3.face_element_start) / 3;
    }
  }

  if (total_lod_2_vertices == 0) {
    total_lod_2_vertices = record.lod_2.vertex_end - record.lod_2.vertex_start;
  }

  if (total_lod_3_vertices == 0) {
    total_lod_3_vertices = record.lod_3.vertex_end - record.lod_3.vertex_start;
  }

  if (total_lod_2_triangles == 0) {
    total_lod_2_triangles = (record.lod_2.face_element_end - record.lod_2.face_element_start) / 3;
  }

  if (total_lod_3_triangles == 0) {
    total_lod_3_triangles = (record.lod_3.face_element_end - record.lod_3.face_element_start) / 3;
  }

  add_dev_label(mesh_name, "(" + std::to_string(total_instances) + " / " + std::to_string(record.group.total) + " active)");
  add_dev_label("Vertices", std::to_string(total_vertices) + " [" + std::to_string(total_vertices * total_instances) + "]");

  if (total_lod_2_vertices > 0) {
    add_dev_label("  (LoD 2)", std::to_string(total_lod_2_vertices) + " [" + std::to_string(total_lod_2_vertices * total_instances) + "]");
  }

  if (total_lod_3_vertices > 0) {
    add_dev_label("  (LoD 3)", std::to_string(total_lod_3_vertices) + " [" + std::to_string(total_lod_3_vertices * total_instances) + "]");
  }

  add_dev_label("Triangles", std::to_string(total_triangles) + " [" + std::to_string(total_triangles * total_instances) + "]");

  if (total_lod_2_triangles > 0) {
    add_dev_label("  (LoD 2)", std::to_string(total_lod_2_triangles) + " [" + std::to_string(total_lod_2_triangles * total_instances) + "]");
  }

  if (total_lod_3_triangles > 0) {
    add_dev_label("  (LoD 3)", std::to_string(total_lod_3_triangles) + " [" + std::to_string(total_lod_3_triangles * total_instances) + "]");
  }

  add_dev_label("Position", first_selected.position.toString());
  add_dev_label("Rotation", first_selected.rotation.toString());
}

static void HandleGuidelines(Tachyon* tachyon, State& state) {
  if (objects(state.meshes.editor_guideline).disabled) {
    return;
  }

  auto& camera = tachyon->scene.camera;

  for (auto& guideline : objects(state.meshes.editor_guideline)) {
    float distance_x = abs(guideline.position.x - camera.position.x);
    float distance_y = abs(guideline.position.y - camera.position.y);
    float distance_z = abs(guideline.position.z - camera.position.z);
    float brightness;

    if (guideline.scale.y > guideline.scale.x) {
      // Vertical line
      float scale = 20.f + (distance_x + distance_z) / 2000.f;
      brightness = 1.f - (distance_x + distance_z) / 1000000.f;

      guideline.scale = tVec3f(scale, guideline.scale.y, scale);
    } else if (guideline.scale.x > guideline.scale.z) {
      // Horizontal x
      float scale = 20.f + (distance_y + distance_z) / 2000.f;
      brightness = 1.f - (distance_y + distance_z) / 1000000.f;

      guideline.scale = tVec3f(guideline.scale.x, scale, scale);
    } else {
      // Horizontal z
      float scale = 20.f + (distance_x + distance_y) / 2000.f;
      brightness = 1.f - (distance_y + distance_x) / 1000000.f;

      guideline.scale = tVec3f(scale, scale, guideline.scale.z);
    }

    if (brightness < 0.f) brightness = 0.f;
    brightness *= brightness;
    brightness *= brightness;
    if (brightness < 0.1f) brightness = 0.1f;

    float emissive = brightness * 0.5f;
    if (emissive < 0.1f) emissive = 0.1f;

    guideline.color = tVec4f(brightness, 0, 0, emissive);

    commit(guideline);
  }
}

static void DisableGeneratedMeshes(Tachyon* tachyon) {
  for (auto& asset : MeshLibrary::GetGeneratedMeshAssets()) {
    objects(asset.mesh_index).disabled = true;
  }
}

static void EnablePlaceholderMeshes(Tachyon* tachyon) {
  for (auto& asset : MeshLibrary::GetPlaceableMeshAssets()) {
    if (asset.placeholder) {
      objects(asset.mesh_index).disabled = false;
    }
  }
}

static void ResetInitialObjects(Tachyon* tachyon) {
  for (auto& record : tachyon->mesh_pack.mesh_records) {
    for (auto& initial : record.group.initial_objects) {
      auto& live = *get_live_object(initial);

      live.position = initial.position;
      live.scale = initial.scale;
      live.rotation = initial.rotation;
      live.color = initial.color;
      live.material = initial.material;

      commit(live);
    }

    record.group.initial_objects.clear();
  }
}

void Editor::InitializeEditor(Tachyon* tachyon, State& state) {
  create(state.meshes.editor_position);
  create(state.meshes.editor_rotation);
  create(state.meshes.editor_scale);

  editor.editor_font = Tachyon_CreateUIText("./fonts/CascadiaMonoNF.ttf", 20);
}

void Editor::HandleEditor(Tachyon* tachyon, State& state, const float dt) {
  HandleCamera(tachyon, state, dt);
  HandleInputs(tachyon, state, dt);

  if (editor.is_object_picker_active) {
    RenderObjectPickerList(tachyon);
  }

  if (AreObjectsSelected()) {
    HandleSelectedObjects(tachyon, state);
  } else {
    objects(state.meshes.editor_position).disabled = true;
    objects(state.meshes.editor_rotation).disabled = true;
    objects(state.meshes.editor_scale).disabled = true;
  }

  HandleGuidelines(tachyon, state);

  add_dev_label("Game time", std::to_string(state.current_game_time));
  add_dev_label("Camera position", tachyon->scene.camera.position.toString());
}

void Editor::EnableEditor(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  auto forward = state.ship_position - camera.position;

  camera.orientation.face(forward, tVec3f(0, 1.f, 0));

  objects(state.meshes.editor_guideline).disabled = false;

  DisableGeneratedMeshes(tachyon);
  EnablePlaceholderMeshes(tachyon);
  ResetInitialObjects(tachyon);

  ProceduralGeneration::RemoveAutoPlacedObjects(tachyon, state);

  tachyon->show_developer_tools = true;
  tachyon->use_high_visibility_mode = true;

  state.is_editor_active = true;
}

void Editor::DisableEditor(Tachyon* tachyon, State& state) {
  if (AreObjectsSelected()) {
    if (editor.is_object_picker_active) {
      remove(editor.selected_objects[0]);
    } else {
      RestoreSelectedObjectColors(tachyon);
    }
  }

  editor.is_object_picker_active = false;

  objects(state.meshes.editor_position).disabled = true;
  objects(state.meshes.editor_rotation).disabled = true;
  objects(state.meshes.editor_scale).disabled = true;

  WorldSetup::RebuildWorld(tachyon, state);

  // @todo respawn piloted vehicle
  state.is_piloting_vehicle = false;

  // Reset target trackers to avoid stale references
  // if objects are deleted in the editor
  state.on_screen_target_trackers.clear();

  state.is_editor_active = false;

  tachyon->use_high_visibility_mode = false;
}