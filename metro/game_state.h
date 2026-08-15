#pragma once

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_types.h"

#include "metro/debug.h"

#include "metro/collision.h"
#include "metro/constants.h"
#include "metro/entities.h"

// @todo move to engine
#define for_range(__low, __high) for (int i = __low; i <= __high; i++)
#define for_reversed(array) for (int32 i = (int32) array.size() - 1; i >= 0; i--)

#define get_scene_time() tachyon->scene.scene_time
#define time_since(t) (tachyon->scene.scene_time - (t))

#define is_moving_left_stick() (tachyon->left_stick.x != 0.f || tachyon->left_stick.y != 0.f)
#define is_moving_right_stick() (tachyon->right_stick.x != 0.f || tachyon->right_stick.y != 0.f)

namespace metro {
  // @todo move to entities.h
  struct Bicycle {
    EntityType type = UNSPECIFIED;
    int32 id = -1;

    tColor frame_color;
    tColor wheel_color;
    tColor grips_color;
    tColor saddle_color;

    tVec3f spawn_position;
    tVec3f spawn_facing_direction;

    tVec3f position;
    tVec3f visual_position;
    float pedal_speed = 0.f;
    float speed = 0.f;
    float drifting_factor = 0.f;
    tVec3f facing_direction;
    tVec3f momentum;

    bool drifting = false;
    bool jumping_off_ramp = false;
    bool in_freefall = false;

    tVec3f front_wheel_position;
    tVec3f front_wheel_slope;
    float front_wheel_downward_force = 0.f;
    float front_wheel_recoil_force = 0.f;
    float front_wheel_recoil_timer = 0.f;
    float recoil = 0.f;

    tVec3f back_wheel_position;
    tVec3f back_wheel_slope;
    float back_wheel_downward_force = 0.f;

    float steering_angle = 0.f;
    float leaning_angle = 0.f;
    float rocking_factor = 0.f;
    float pitch = 0.f;
    float pedal_revolution = 0.f;
    float wheel_revolution = 0.f;

    tVec3f movement_vector;

    Quaternion flat_rotation;
    Quaternion directional_rotation;
    Quaternion visual_rotation;
  };

  struct MeshIds {
    uint16
      // @temporary
      dev_cube,

      // @temporary
      dev_mannequin,

      // Common bike
      common_frame,
      common_fork,
      common_handlebars,
      common_grips,
      common_seatpost,
      common_saddle,
      common_crank,
      common_pedal,
      common_spokes,
      common_wheel,

      // Static entities
      platform,
      ramp,
      walkway_segment,
      walkway_plane,

      // Interactive entities
      vending_machine

      ;

    int32 walkway_stream;
  };

  struct State {
    float dt;

    MeshIds meshes;

    tVec3f player_position;
    tVec3f previous_player_position;
    tVec3f player_velocity = tVec3f(0.f);
    float recorded_player_speed = 0.f;
    float target_camera_azimuth = 0.f;
    float target_camera_azimuth_blend_rate = 0.f;
    float last_manual_camera_control_time = 0.f;

    int32 player_bike_id = -1;

    // Refers to the iterated index of the bike the player is riding.
    // This is an index in the set of bikes of a given type, which is
    // also used for the object indexes of the bike's parts.
    //
    // We store this on the initial bike update, so that when we then
    // run physics on the player bike, we can do a final secondary update
    // on the same bike instance as a physics post-step.
    int32 player_bike_index = -1;

    std::vector<Bicycle> bicycles;
    Entities entities;

    std::string world_level_name = "test_world.lvl";

    // @todo dev only
    bool is_editor_open = false;
    bool use_slow_motion = false;
    bool use_frame_stepping = false;
    bool allow_frame_step = false;
  };
}