#include <array>

#include "metro/world_editor.h"
#include "metro/editor_utilities.h"

using namespace metro;

enum TransformType {
  POSITION,
  SCALE,
  ROTATION
};

enum SelectionType {
  NOTHING_SELECTED = -1,
  BICYCLE,
  STATIC_ENTITY,
  INTERACTIVE_ENTITY
};

static struct EditorState {
  TransformType transform_type = POSITION;
  SelectionType selection_type = NOTHING_SELECTED;
  void* selection = nullptr;
} editor;

static inline bool IsAnythingSelected() {
  return editor.selection_type != NOTHING_SELECTED;
}

static tVec3f GetSelectionPosition() {
  switch (editor.selection_type) {
    case BICYCLE:
      return ((Bicycle*)editor.selection)->position;
    case STATIC_ENTITY:
      return ((StaticEntity*)editor.selection)->position;
    case INTERACTIVE_ENTITY: // @todo
    default:
      return tVec3f(0.f);
  }
}

static Quaternion GetSelectionRotation() {
  switch (editor.selection_type) {
    case BICYCLE:
      return ((Bicycle*)editor.selection)->flat_rotation;
    case STATIC_ENTITY:
      return ((StaticEntity*)editor.selection)->rotation;
    case INTERACTIVE_ENTITY: // @todo
    default:
      return Quaternion(1.f, 0, 0, 0);
  }
}

static void MoveSelection(const tVec3f& offset) {
  switch (editor.selection_type) {
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

static void ShowSelectionGizmo(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  tVec3f selection_position = GetSelectionPosition();
  Quaternion selection_rotation = GetSelectionRotation();
  tVec3f selection_direction = (selection_position - camera.position).unit();
  tVec3f gizmo_position = camera.position + selection_direction * 2000.f;

  if (editor.transform_type == POSITION) {
    EditorUtilities::ShowPositionGizmo(tachyon, state, gizmo_position, selection_rotation);
  } else if (editor.transform_type == SCALE) {
    EditorUtilities::ShowScaleGizmo(tachyon, state, gizmo_position, selection_rotation);
  } else {
    EditorUtilities::ShowRotationGizmo(tachyon, state, gizmo_position, selection_rotation);
  }
}

static inline bool IsSelectable(const tVec3f& position, const tVec3f& scale, const tVec3f& camera_position, const tVec3f& camera_forward) {
  tVec3f camera_to_selectable = position - camera_position;
  float distance = camera_to_selectable.magnitude();
  tVec3f direction = camera_to_selectable / distance;
  float distance_threshold = scale.magnitude() * 5.f;
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
        editor.selection_type = BICYCLE;
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
        editor.selection_type = STATIC_ENTITY;
        editor.transform_type = POSITION;

        return;
      }
    }
  }
}

static inline void Deselect() {
  editor.selection_type = NOTHING_SELECTED;
  editor.selection = nullptr;
}

static void HandleCameraControls(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  tVec3f camera_forward = camera.orientation.getDirection();
  tVec3f camera_left = camera.orientation.getLeftDirection();

  // WASD movement
  {
    if (is_key_held(tKey::W)) {
      camera.position += camera_forward * 15000.f * state.dt;
    }

    if (is_key_held(tKey::A)) {
      camera.position += camera_left * 15000.f * state.dt;
    }

    if (is_key_held(tKey::S)) {
      camera.position -= camera_forward * 15000.f * state.dt;
    }

    if (is_key_held(tKey::D)) {
      camera.position -= camera_left * 15000.f * state.dt;
    }
  }

  // Looking around with the mouse
  {
    const float camera_mouse_speed = 0.2f;

    if (!is_key_held(tKey::SHIFT) && !is_left_mouse_held_down()) {
      camera.orientation.yaw += tachyon->mouse_delta_x * camera_mouse_speed * state.dt;
      camera.orientation.pitch += tachyon->mouse_delta_y * camera_mouse_speed * state.dt;

      camera.rotation = camera.orientation.toQuaternion();
    }
  }

  // Swiveling around selected objects with SHIFT
  {
    if (
      is_key_held(tKey::SHIFT) &&
      IsAnythingSelected()
    ) {
      tVec3f selection_position = GetSelectionPosition();

      EditorUtilities::SwivelAroundPosition(tachyon, state, selection_position);
    }
  }
}

static void HandleClickActions(Tachyon* tachyon, State& state) {
  if (did_left_click_down() && !IsAnythingSelected()) {
    MaybeMakeSelection(tachyon, state);
  }

  if (did_right_click_down()) {
    Deselect();
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
    Quaternion rotation = GetSelectionRotation();

    tVec3f x_axis = EditorUtilities::GetClosestBasisAxis(rotation, camera_left);
    tVec3f y_axis = tVec3f(0, 1.f, 0);

    tVec3f move_x = x_axis * 4.f * (float) -tachyon->mouse_delta_x;
    tVec3f move_y = y_axis * 4.f * (float) -tachyon->mouse_delta_y;

    MoveSelection(move_x + move_y);
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

    if (IsAnythingSelected()) {
      HandleTransformTypeCycleActions(tachyon, state);
      HandleSelectionManipulationActions(tachyon, state);
      ShowSelectionGizmo(tachyon, state);
    }
  }
}

void WorldEditor::Close(Tachyon* tachyon, State& state) {
  Deselect();

  state.is_editor_open = false;

  show_overlay_message("Leaving editor");
}