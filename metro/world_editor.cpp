#include <format>

#include "metro/world_editor.h"
#include "metro/background_bicycles.h"
#include "metro/editor_utilities.h"
#include "metro/serialization.h"
#include "metro/utilities.h"

using namespace metro;

struct HighlightBox {
  tVec3f position;
  tVec3f scale;
  Quaternion rotation = Quaternion(1.f, 0, 0, 0);
};

enum CloneDirection {
  LEFT,
  RIGHT
};

enum TransformType {
  POSITION,
  SCALE,
  ROTATION
};

static struct EditorState {
  TransformType transform_type = POSITION;
  EntityType entity_type = COMMON_BIKE;
  void* selection = nullptr;
  tVec3f highlight_color = tVec3f(1.f, 0, 1.f);
  bool is_placing_new_entity = false;
} editor;

static inline std::string Format(const tVec3f& v) {
  return std::format("{:.2f}, {:.2f}, {:.2f}", v.x, v.y, v.z);
}

static inline std::string Format(const Quaternion& q) {
  return std::format("{:.3f}, {:.3f}, {:.3f}, {:.3f}", q.w, q.x, q.y, q.z);
}

static inline bool IsAnythingSelected() {
  return editor.selection != nullptr;
}

static std::string GetSelectionLabel() {
  auto entity_name = Serialization::EntityTypeToString(editor.entity_type);
  std::string id_string;

  switch (GetEntityCategory(editor.entity_type)) {
    case BICYCLE:
      id_string = std::format("{:X}", ((Bicycle*)editor.selection)->id);
      break;
    case STATIC_ENTITY:
      id_string = std::format("{:X}", ((StaticEntity*)editor.selection)->id);
      break;
    case INTERACTIVE_ENTITY: // @todo
    default:
      break;
  }

  return entity_name + " (ID: 0x" + id_string + ")";
}

// ---------------------
// Positioning utilities
// ---------------------

static tVec3f GetSelectionPosition() {
  switch (GetEntityCategory(editor.entity_type)) {
    case BICYCLE:
      return ((Bicycle*)editor.selection)->spawn_position;
    case STATIC_ENTITY:
      return ((StaticEntity*)editor.selection)->position;
    case INTERACTIVE_ENTITY: // @todo
    default:
      return tVec3f(0.f);
  }
}

static void MoveSelection(const tVec3f& offset) {
  switch (GetEntityCategory(editor.entity_type)) {
    case BICYCLE: {
      auto& bike = *(Bicycle*) editor.selection;

      bike.spawn_position += offset;
      bike.position = bike.spawn_position;

      break;
    }
    case STATIC_ENTITY: {
      auto& entity = *(StaticEntity*) editor.selection;

      entity.position += offset;
      entity.needs_update = true;

      break;
    }
    case INTERACTIVE_ENTITY: // @todo
    default:
      break;
  }
}

// -----------------
// Scaling utilities
// -----------------

static tVec3f GetSelectionScale() {
  switch (GetEntityCategory(editor.entity_type)) {
    case BICYCLE:
      // @temporary
      return tVec3f(2000.f);
    case STATIC_ENTITY:
      return ((StaticEntity*)editor.selection)->scale;
    case INTERACTIVE_ENTITY: // @todo
    default:
      return tVec3f(0.f);
  }
}

static void SetSelectionScale(const tVec3f& scale) {
  switch (GetEntityCategory(editor.entity_type)) {
    case BICYCLE:
      // Bicycles cannot be scaled
      break;
    case STATIC_ENTITY: {
      auto& entity = *(StaticEntity*) editor.selection;

      entity.scale = scale;
      entity.needs_update = true;

      break;
    }
    case INTERACTIVE_ENTITY: // @todo
    default:
      break;
  }
}

static void ScaleSelection(const tVec3f& scale_change) {
  switch (GetEntityCategory(editor.entity_type)) {
    case BICYCLE:
      // Bicycles cannot be scaled
      break;
    case STATIC_ENTITY: {
      auto& entity = *(StaticEntity*) editor.selection;

      entity.scale += scale_change;
      entity.needs_update = true;

      break;
    }
    case INTERACTIVE_ENTITY: // @todo
    default:
      break;
  }
}

// ------------------
// Rotation utilities
// ------------------

static Quaternion GetSelectionRotation() {
  switch (GetEntityCategory(editor.entity_type)) {
    case BICYCLE:
      return ((Bicycle*)editor.selection)->flat_rotation;
    case STATIC_ENTITY:
      return ((StaticEntity*)editor.selection)->rotation;
    case INTERACTIVE_ENTITY: // @todo
    default:
      return Quaternion(1.f, 0, 0, 0);
  }
}

static void SetSelectionRotation(const Quaternion& rotation) {
  switch (GetEntityCategory(editor.entity_type)) {
    case BICYCLE: {
      auto& bike = *(Bicycle*) editor.selection;
      tVec3f direction = rotation.getDirection().invert();
      float angle = atan2f(direction.z, direction.x);

      bike.spawn_facing_direction = direction;
      bike.facing_direction = bike.spawn_facing_direction;
      bike.flat_rotation = rotation;

      break;
    }
    case STATIC_ENTITY: {
      auto& entity = *(StaticEntity*) editor.selection;

      entity.rotation = rotation;
      entity.needs_update = true;

      break;
    }
    case INTERACTIVE_ENTITY: // @todo
    default:
      break;
  }
}

static void RotateSelection(const tVec3f& axis, const float angle) {
  switch (GetEntityCategory(editor.entity_type)) {
    case BICYCLE: {
      auto& bike = *(Bicycle*) editor.selection;

      // Restrict bikes to y-axis rotations only
      if (axis == Y_UP) {
        bike.spawn_facing_direction = Quaternion::fromAxisAngle(axis, angle).toMatrix4f() * bike.spawn_facing_direction;
        bike.spawn_facing_direction = bike.spawn_facing_direction.unit();
        bike.facing_direction = bike.spawn_facing_direction;
        bike.flat_rotation = Quaternion::FromDirection(bike.spawn_facing_direction, Y_UP);
      }

      break;
    }
    case STATIC_ENTITY: {
      auto& entity = *(StaticEntity*) editor.selection;

      entity.rotation *= Quaternion::fromAxisAngle(axis, angle);
      entity.needs_update = true;

      break;
    }
    case INTERACTIVE_ENTITY: // @todo
    default:
      break;
  }
}

// ------------------

static HighlightBox GetSelectionHighlightBox() {
  switch (GetEntityCategory(editor.entity_type)) {
    case BICYCLE: {
      auto& bike = *(Bicycle*) editor.selection;

      return {
        .position = UnitBikeToWorldPosition(bike, tVec3f(0, 0.3f, 0)),
        .scale = tVec3f(500.f, 1375.f, 2050.f),
        .rotation = bike.flat_rotation
      };
    }
    case STATIC_ENTITY: {
      auto& entity = *(StaticEntity*) editor.selection;

      return {
        .position = entity.position,
        .scale = entity.scale + tVec3f(250.f),
        .rotation = entity.rotation
      };
    };
    case INTERACTIVE_ENTITY:
      // @todo
      return {};
    default:
      return {};
  }
}

static void ShowSelectionDetails(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  tVec3f selection_position = GetSelectionPosition();
  Quaternion selection_rotation = GetSelectionRotation();
  tVec3f selection_scale = GetSelectionScale();
  tVec3f selection_direction = (selection_position - camera.position).unit();
  tVec3f gizmo_position = camera.position + selection_direction * 2000.f;

  // Highlight box
  {
    auto box = GetSelectionHighlightBox();

    Debug::ShowDebugBox(tachyon, {
      .position = box.position,
      .scale = box.scale,
      .rotation = box.rotation,
      .color = editor.highlight_color
    });
  }

  // Gizmo
  {
    if (editor.transform_type == POSITION) {
      EditorUtilities::ShowPositionGizmo(tachyon, gizmo_position, selection_rotation);
    } else if (editor.transform_type == SCALE) {
      EditorUtilities::ShowScaleGizmo(tachyon, gizmo_position, selection_rotation);
    } else {
      EditorUtilities::ShowRotationGizmo(tachyon, gizmo_position, selection_rotation);
    }
  }

  // Labels
  {
    auto label = GetSelectionLabel();

    Debug::ShowDebugLabel(tachyon, selection_position, { .y = 50.f }, label);
    Debug::ShowDebugLabel(tachyon, selection_position, { .y = 72.f }, Format(selection_position));
    Debug::ShowDebugLabel(tachyon, selection_position, { .y = 94.f }, Format(selection_rotation));
    Debug::ShowDebugLabel(tachyon, selection_position, { .y = 116.f }, Format(selection_scale));
  }
}

static inline bool IsSelectable(const tVec3f& position, const tVec3f& scale, const tVec3f& camera_position, const tVec3f& camera_forward) {
  tVec3f camera_to_selectable = position - camera_position;
  float distance = camera_to_selectable.magnitude();
  tVec3f direction = camera_to_selectable / distance;
  float distance_threshold = scale.magnitude() * 8.f;
  float dot = tVec3f::dot(direction, camera_forward);

  return distance < distance_threshold && dot > 0.98f;
}

static void MaybeMakeSelection(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  tVec3f camera_forward = camera.orientation.getDirection();

  // Track potential selections by distance so we can pick the closest one
  float closest_distance = FLT_MAX;

  // Bike selection
  for (auto& bike : state.bicycles) {
    if (IsSelectable(bike.position, tVec3f(2000.f), camera.position, camera_forward)) {
      float distance = tVec3f::distance(bike.position, camera.position);

      if (distance < closest_distance) {
        editor.selection = &bike;
        editor.entity_type = bike.type;
        editor.transform_type = POSITION;

        closest_distance = distance;
      }
    }
  }

  // Static entity selection
  for_static_entity_containers() {
    for_entities() {
      if (IsSelectable(entity.position, entity.scale, camera.position, camera_forward)) {
        float distance = tVec3f::distance(entity.position, camera.position);

        if (distance < closest_distance) {
          editor.selection = &entity;
          editor.entity_type = entity.type;
          editor.transform_type = POSITION;

          closest_distance = distance;
        }
      }
    }
  }
}

static inline void Deselect() {
  editor.selection = nullptr;
}

static void DeleteSelection(Tachyon* tachyon, State& state) {
  switch (GetEntityCategory(editor.entity_type)) {
    case BICYCLE: {
      auto& bike = *(Bicycle*) editor.selection;

      BackgroundBicycles::DestroyBicycle(tachyon, state, bike);

      break;
    }
    case STATIC_ENTITY:
      ((StaticEntity*) editor.selection)->needs_deletion = true;
      break;
    case INTERACTIVE_ENTITY:
      // @todo
      break;
    default:
      break;
  }
}

static void PlaceNewBicycle(Tachyon* tachyon, State& state, const tVec3f& position) {
  Bicycle bike;
  bike.type          = COMMON_BIKE;
  bike.id            = CreateUniqueId();
  bike.position      = position;
  // @temporary
  bike.frame_color   = tVec3f(1.f, 0.2f, 0.4f);
  bike.grips_color   = tVec3f(0.1f);
  bike.saddle_color  = tVec3f(0.1f, 0, 0);
  bike.wheel_color   = tVec3f(1.f, 0.9f, 0.7f);
  bike.facing_direction = Z_BACKWARD;

  bike.spawn_position = bike.position;
  bike.spawn_facing_direction = bike.facing_direction;

  BackgroundBicycles::SpawnBicycle(tachyon, state, bike);

  // @temporary
  // @todo return from SpawnBicycle()
  editor.selection = &state.bicycles.back();
}

static void PlaceNewEntity(Tachyon* tachyon, State& state, const tVec3f& position) {
  switch (GetEntityCategory(editor.entity_type)) {
    case BICYCLE:
      PlaceNewBicycle(tachyon, state, position);

      break;
    case STATIC_ENTITY: {
      auto& entity = CreateStaticEntity(state.entities, editor.entity_type);

      entity.position = position;
      entity.scale = tVec3f(2000.f);
      entity.color = tVec3f(1.f);

      editor.selection = &entity;

      break;
    }
    case INTERACTIVE_ENTITY:
      // @todo
      break;
    default:
      break;
  }

  editor.transform_type = POSITION;
  editor.is_placing_new_entity = false;
}

static void CloneSelection(Tachyon* tachyon, State& state, CloneDirection direction) {
  auto& camera = tachyon->scene.camera;

  tVec3f camera_direction = direction == LEFT
    ? camera.orientation.getLeftDirection()
    : camera.orientation.getRightDirection();

  tVec3f selection_position = GetSelectionPosition();
  Quaternion selection_rotation = GetSelectionRotation();
  tVec3f selection_scale = GetSelectionScale();

  tVec3f basis_x = EditorUtilities::GetClosestBasisAxis(selection_rotation, camera_direction);
  tVec3f spawn_position = selection_position + basis_x * 5000.f;

  PlaceNewEntity(tachyon, state, spawn_position);

  // Copy rotation + scale to the new entity
  SetSelectionRotation(selection_rotation);
  SetSelectionScale(selection_scale);
}

static void ShowPlacementPreview(Tachyon* tachyon, State& state) {
  const float ray_length = 30000.f;

  auto& camera = tachyon->scene.camera;
  tVec3f ray = camera.orientation.getDirection() * ray_length;

  // @temporary
  // @todo define default scales per entity type
  const tVec3f scale = 2000.f;

  HighlightBox box;
  box.position = camera.position + ray;
  box.scale = scale;

  // Track ray hits by distance so we can place entities at the closest one
  float closest_distance = FLT_MAX;

  for_static_entity_containers() {
    for_entities() {
      for (auto& plane : entity.collision_planes) {
        auto ray_test = Collision::TestRayHit(camera.position, ray, plane);

        if (ray_test.has_collision) {
          float distance = tVec3f::distance(ray_test.collision_point, camera.position);

          if (distance < closest_distance) {
            // Position the preview box fully above the collision point,
            // with a small additional buffer to avoid clipping through floors
            box.position = ray_test.collision_point + tVec3f(0, scale.y + 25.f, 0);

            closest_distance = distance;
          }
        }
      }
    }
  }

  Debug::ShowDebugBox(tachyon, {
    .position = box.position,
    .scale = box.scale,
    .rotation = box.rotation,
    .color = tVec3f(1.f, 0, 1.f)
  });

  if (did_left_click_down()) {
    PlaceNewEntity(tachyon, state, box.position);
  }
}

static void HandleCameraControls(Tachyon* tachyon, State& state) {
  const float movement_speed = 15000.f;
  const float mouse_sensivity = 0.2f;

  auto& camera = tachyon->scene.camera;

  // WASD movement
  EditorUtilities::UseWASDCameraMovement(tachyon, state, movement_speed);

  // Mouse look-around
  if (!is_key_held(tKey::SHIFT) && !is_left_mouse_held_down()) {
    EditorUtilities::UseMouseCameraLookaround(tachyon, state, mouse_sensivity);
  }

  // Swiveling around selected objects with SHIFT
  if (is_key_held(tKey::SHIFT) && IsAnythingSelected()) {
    tVec3f pivot_position = GetSelectionPosition();

    EditorUtilities::SwivelAroundPosition(tachyon, pivot_position);
  }
}

static void HandleClickActions(Tachyon* tachyon, State& state) {
  if (did_left_click_down() && !IsAnythingSelected() && !editor.is_placing_new_entity) {
    MaybeMakeSelection(tachyon, state);
  }

  if (did_right_click_down()) {
    if (IsAnythingSelected()) {
      Deselect();

      Serialization::SaveWorldData(state);
    } else if (editor.is_placing_new_entity) {
      editor.is_placing_new_entity = false;
    } else {
      editor.is_placing_new_entity = true;
    }
  }

  // Alternate between highlight colors depending on mouse state
  {
    const tVec3f highlight_color = tVec3f(1.f, 0, 1.f);
    const tVec3f mouse_down_highlight_color = tVec3f(1.f);

    editor.highlight_color = tVec3f::lerp(
      editor.highlight_color,
      is_mouse_held_down() ? mouse_down_highlight_color : highlight_color,
      10.f * state.dt
    );
  }
}

static void HandleTransformTypeCycleActions(Tachyon* tachyon, State& state) {
  if (did_wheel_down()) {
    if (editor.transform_type == POSITION) {
      editor.transform_type = SCALE;
    } else if (editor.transform_type == SCALE) {
      editor.transform_type = ROTATION;
    } else {
      editor.transform_type = POSITION;
    }
  }

  if (did_wheel_up()) {
    if (editor.transform_type == POSITION) {
      editor.transform_type = ROTATION;
    } else if (editor.transform_type == SCALE) {
      editor.transform_type = POSITION;
    } else {
      editor.transform_type = SCALE;
    }
  }
}

static void HandleSelectionManipulationActions(Tachyon* tachyon, State& state) {
  if (is_left_mouse_held_down()) {
    auto& camera = tachyon->scene.camera;
    tVec3f camera_left = camera.orientation.getLeftDirection();
    Quaternion basis_rotation = GetSelectionRotation();

    tVec3f basis_x = EditorUtilities::GetClosestBasisAxis(basis_rotation, camera_left);
    tVec3f basis_y = tVec3f(0, 1.f, 0);

    // Position actions
    if (editor.transform_type == POSITION) {
      tVec3f delta_x = basis_x * 4.f * (float) -tachyon->mouse_delta_x;
      tVec3f delta_y = basis_y * 4.f * (float) -tachyon->mouse_delta_y;

      MoveSelection(delta_x + delta_y);

    // Scale actions
    } else if (editor.transform_type == SCALE) {
      bool is_horizontal_action = abs(tachyon->mouse_delta_x) > abs(tachyon->mouse_delta_y);

      if (is_horizontal_action) {
        tVec3f scale_axis = EditorUtilities::GetScalingAxis(basis_x, basis_rotation);
        tVec3f scale_change = scale_axis * 4.f * (float) tachyon->mouse_delta_x;

        ScaleSelection(scale_change);
      } else {
        tVec3f scale_axis = EditorUtilities::GetScalingAxis(basis_y, basis_rotation);
        tVec3f scale_change = scale_axis * 4.f * (float) -tachyon->mouse_delta_y;

        ScaleSelection(scale_change);
      }
    }

    // Rotation actions
    else if (editor.transform_type == ROTATION) {
      bool is_horizontal_action = abs(tachyon->mouse_delta_x) > abs(tachyon->mouse_delta_y);

      if (is_horizontal_action) {
        tVec3f rotation_axis = EditorUtilities::GetClosestBasisAxis(basis_rotation, tVec3f(0, 1.f, 0));
        float angle = 0.002f * (float) tachyon->mouse_delta_x;

        RotateSelection(rotation_axis, angle);
      } else {
        tVec3f rotation_axis = EditorUtilities::GetClosestBasisAxis(basis_rotation, camera_left);
        float angle = 0.002f * (float) -tachyon->mouse_delta_y;

        RotateSelection(rotation_axis, angle);
      }
    }
  }
}

static void HandleSelectionHotkeys(Tachyon* tachyon, State& state) {
  if (did_press_key(tKey::BACKSPACE)) {
    DeleteSelection(tachyon, state);
    Deselect();

    Serialization::SaveWorldData(state);
  }

  if (did_press_key(tKey::ARROW_LEFT)) {
    CloneSelection(tachyon, state, LEFT);
  }

  if (did_press_key(tKey::ARROW_RIGHT)) {
    CloneSelection(tachyon, state, RIGHT);
  }
}

void WorldEditor::Open(Tachyon* tachyon, State& state) {
  state.is_editor_open = true;

  // Sync bicycles to their spawn positions and dismount any active bike
  // @todo remove/hide dynamically-spawned bikes, once those exist
  {
    for (auto& bike : state.bicycles) {
      // @todo factor
      bike.leaning_angle = 0.f;
      bike.steering_angle = 0.f;
      bike.pedal_speed = 0.f;
      bike.speed = 0.f;
      bike.pedal_revolution = 0.f;
      bike.wheel_revolution = 0.f;
      bike.position = bike.spawn_position;
      bike.facing_direction = bike.spawn_facing_direction;
      bike.flat_rotation = Quaternion::FromDirection(bike.spawn_facing_direction, Y_UP);
    }

    state.player_bike_id = -1;
  }

  show_overlay_message("Entering editor");
}

void WorldEditor::Update(Tachyon* tachyon, State& state) {
  profile("WorldEditor::Update()");

  auto& camera = tachyon->scene.camera;

  if (is_window_focused()) {
    HandleCameraControls(tachyon, state);
    HandleClickActions(tachyon, state);

    if (editor.is_placing_new_entity) {
      ShowPlacementPreview(tachyon, state);
    }

    if (IsAnythingSelected()) {
      HandleTransformTypeCycleActions(tachyon, state);
      HandleSelectionManipulationActions(tachyon, state);
      ShowSelectionDetails(tachyon, state);
      HandleSelectionHotkeys(tachyon, state);
    }
  }
}

void WorldEditor::Close(Tachyon* tachyon, State& state) {
  Deselect();

  Serialization::SaveWorldData(state);

  state.is_editor_open = false;

  show_overlay_message("Leaving editor");
}