#include <array>

#include "metro/world_editor.h"
#include "metro/editor_utilities.h"
#include "metro/utilities.h"

using namespace metro;

struct HighlightBox {
  tVec3f position;
  tVec3f scale;
  Quaternion rotation;
};

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
  tVec3f highlight_color = tVec3f(1.f, 0, 1.f);
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

static tVec3f GetSelectionScale() {
  switch (editor.selection_type) {
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

static HighlightBox GetSelectionHighlightBox() {
  switch (editor.selection_type) {
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

static void ScaleSelection(const tVec3f& scale_change) {
  switch (editor.selection_type) {
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
  switch (editor.selection_type) {
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
      ShowSelectionDetails(tachyon, state);
    }
  }
}

void WorldEditor::Close(Tachyon* tachyon, State& state) {
  Deselect();

  state.is_editor_open = false;

  show_overlay_message("Leaving editor");
}