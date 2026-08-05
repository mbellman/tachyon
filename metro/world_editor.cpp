#include <format>

#include "metro/world_editor.h"
#include "metro/background_bicycles.h"
#include "metro/collision.h"
#include "metro/constants.h"
#include "metro/editor_utilities.h"
#include "metro/utilities.h"

using namespace metro;

struct HighlightBox {
  tVec3f position;
  tVec3f scale;
  Quaternion rotation = Quaternion(1.f, 0, 0, 0);
};

enum TransformType {
  POSITION,
  SCALE,
  ROTATION
};

enum EntityCategory {
  NOTHING_SELECTED = -1,
  BICYCLE,
  STATIC_ENTITY,
  INTERACTIVE_ENTITY
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

static EntityCategory GetEntityCategory() {
  switch (editor.entity_type) {
    case COMMON_BIKE:
      return BICYCLE;
    case PLATFORM:
    case RAMP:
    case WALKWAY_SEGMENT:
      return STATIC_ENTITY;
    default:
      return NOTHING_SELECTED;
  }
}

static std::string GetEntityDisplayName() {
  switch (editor.entity_type) {
    case COMMON_BIKE    : return "Common Bike";
    case PLATFORM       : return "Platform";
    case RAMP           : return "Ramp";
    case WALKWAY_SEGMENT: return "Walkway Segment";
    default:
      return "Entity";
  }
}

static tVec3f GetSelectionPosition() {
  switch (GetEntityCategory()) {
    case BICYCLE:
      return ((Bicycle*)editor.selection)->position;
    case STATIC_ENTITY:
      return ((StaticEntity*)editor.selection)->position;
    case INTERACTIVE_ENTITY: // @todo
    default:
      return tVec3f(0.f);
  }
}

static tVec3f GetSelectionScale() {
  switch (GetEntityCategory()) {
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

static Quaternion GetSelectionRotation() {
  switch (GetEntityCategory()) {
    case BICYCLE:
      return ((Bicycle*)editor.selection)->flat_rotation;
    case STATIC_ENTITY:
      return ((StaticEntity*)editor.selection)->rotation;
    case INTERACTIVE_ENTITY: // @todo
    default:
      return Quaternion(1.f, 0, 0, 0);
  }
}

static HighlightBox GetSelectionHighlightBox() {
  switch (GetEntityCategory()) {
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

static void MoveSelection(const tVec3f& offset) {
  switch (GetEntityCategory()) {
    case BICYCLE:
      ((Bicycle*)editor.selection)->position += offset;
      break;
    case STATIC_ENTITY:
      ((StaticEntity*)editor.selection)->position += offset;
      ((StaticEntity*)editor.selection)->needs_update = true;
      break;
    case INTERACTIVE_ENTITY: // @todo
    default:
      break;
  }
}

static void ScaleSelection(const tVec3f& scale_change) {
  switch (GetEntityCategory()) {
    case BICYCLE:
      break;
    case STATIC_ENTITY:
      ((StaticEntity*)editor.selection)->scale += scale_change;
      ((StaticEntity*)editor.selection)->needs_update = true;
      break;
    case INTERACTIVE_ENTITY: // @todo
    default:
      break;
  }
}

static void RotateSelection(const tVec3f& axis, const float angle) {
  switch (GetEntityCategory()) {
    case BICYCLE:
      // @todo
      break;
    case STATIC_ENTITY:
      ((StaticEntity*)editor.selection)->rotation *= Quaternion::fromAxisAngle(axis, angle);
      ((StaticEntity*)editor.selection)->needs_update = true;
      break;
    case INTERACTIVE_ENTITY: // @todo
    default:
      break;
  }
}

static void ShowSelectionDetails(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  tVec3f selection_position = GetSelectionPosition();
  Quaternion selection_rotation = GetSelectionRotation();
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
    auto name = GetEntityDisplayName();

    Debug::ShowDebugLabel(tachyon, selection_position, { .y = 50.f }, name);
    Debug::ShowDebugLabel(tachyon, selection_position, { .y = 72.f }, Format(selection_position));
    Debug::ShowDebugLabel(tachyon, selection_position, { .y = 94.f }, Format(selection_rotation));
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

  // Bike selection
  {
    for (auto& bike : state.bicycles) {
      if (IsSelectable(bike.position, tVec3f(2000.f), camera.position, camera_forward)) {
        editor.selection = &bike;
        editor.entity_type = bike.type;
        editor.transform_type = POSITION;

        return;
      }
    }
  }

  // Static entity selection
  for_static_entity_containers() {
    for_entities() {
      if (IsSelectable(entity.position, entity.scale, camera.position, camera_forward)) {
        editor.selection = &entity;
        editor.entity_type = entity.type;
        editor.transform_type = POSITION;

        return;
      }
    }
  }
}

static inline void Deselect() {
  editor.selection = nullptr;
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

  BackgroundBicycles::SpawnBicycle(tachyon, state, bike);
}

static void PlaceNewEntity(Tachyon* tachyon, State& state, const tVec3f& position) {
  switch (GetEntityCategory()) {
    case BICYCLE:
      PlaceNewBicycle(tachyon, state, position);
      break;
    case STATIC_ENTITY: {
      auto& entity = CreateStaticEntity(state.entities, editor.entity_type);
      entity.id = CreateUniqueId();

      entity.position = position;
      entity.scale = tVec3f(2000.f);
      entity.color = tVec3f(1.f);

      break;
    }
    case INTERACTIVE_ENTITY:
      // @todo
      break;
    default:
      break;
  }

  editor.is_placing_new_entity = false;
}

static void ShowPlacementPreview(Tachyon* tachyon, State& state) {
  const float ray_length = 30000.f;

  auto& camera = tachyon->scene.camera;
  tVec3f ray = camera.orientation.getDirection() * ray_length;

  // @temporary
  const float size = 2000.f;

  HighlightBox box;
  box.position = camera.position + ray;
  box.scale = tVec3f(size);

  for_static_entity_containers() {
    for_entities() {
      for (auto& plane : entity.collision_planes) {
        auto ray_test = Collision::TestRayHit(camera.position, ray, plane);

        if (ray_test.has_collision) {
          // Position the preview box fully above the collision point,
          // with a small additional buffer to avoid clipping through floors
          box.position = ray_test.collision_point + tVec3f(0, size + 25.f, 0);

          break;
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
    switch (GetEntityCategory()) {
      case BICYCLE:
        // @todo
        break;
      case STATIC_ENTITY:
        ((StaticEntity*) editor.selection)->needs_deletion = true;
        break;
      case INTERACTIVE_ENTITY:
        // @todo
        break;
      default:
        break;
    }

    Deselect();
  }
}

void WorldEditor::Open(Tachyon* tachyon, State& state) {
  state.is_editor_open = true;

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

  state.is_editor_open = false;

  show_overlay_message("Leaving editor");
}