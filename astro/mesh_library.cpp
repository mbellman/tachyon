#include "astro/mesh_library.h"
#include "astro/entity_dispatcher.h"

#define CUBE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreateCubeMesh(), total)
#define SPHERE_MESH(total, divisions) Tachyon_AddMesh(tachyon, Tachyon_CreateSphereMesh(divisions), total)
#define PLANE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreatePlaneMesh(), total)

#define MODEL_MESH(path, total) Tachyon_AddMesh(tachyon, Tachyon_LoadMesh(path), total)
#define MODEL_MESH_LOD_2(lod_1_path, lod_2_path, total) Tachyon_AddMesh(tachyon, Tachyon_LoadMesh(lod_1_path), Tachyon_LoadMesh(lod_2_path), total)

using namespace astro;

static void AddHUDMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  meshes.astrolabe_rear        = MODEL_MESH("./astro/3d_models/astrolabe_rear.obj", 1);
  meshes.astrolabe_base        = MODEL_MESH("./astro/3d_models/astrolabe_base.obj", 1);
  meshes.astrolabe_plate       = MODEL_MESH("./astro/3d_models/astrolabe_plate.obj", 1);
  meshes.astrolabe_plate2      = MODEL_MESH("./astro/3d_models/astrolabe_plate2.obj", 1);
  meshes.astrolabe_plate3      = MODEL_MESH("./astro/3d_models/astrolabe_plate3.obj", 1);
  meshes.astrolabe_plate4      = MODEL_MESH("./astro/3d_models/astrolabe_plate4.obj", 1);
  meshes.astrolabe_ring        = MODEL_MESH("./astro/3d_models/astrolabe_ring.obj", 1);
  meshes.astrolabe_hand        = MODEL_MESH("./astro/3d_models/astrolabe_hand.obj", 1);
  meshes.target_reticle        = MODEL_MESH("./astro/3d_models/target_reticle.obj", 1);

  mesh(meshes.astrolabe_rear).shadow_cascade_ceiling = 0;
  mesh(meshes.astrolabe_base).shadow_cascade_ceiling = 0;
  mesh(meshes.astrolabe_plate).shadow_cascade_ceiling = 0;
  mesh(meshes.astrolabe_plate2).shadow_cascade_ceiling = 0;
  mesh(meshes.astrolabe_plate3).shadow_cascade_ceiling = 0;
  mesh(meshes.astrolabe_plate4).shadow_cascade_ceiling = 0;
  mesh(meshes.astrolabe_ring).shadow_cascade_ceiling = 0;
  mesh(meshes.astrolabe_hand).shadow_cascade_ceiling = 0;
  mesh(meshes.target_reticle).shadow_cascade_ceiling = 0;
}

static void AddDecorativeMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  meshes.flat_ground   = PLANE_MESH(1000);
  meshes.rock_1        = MODEL_MESH("./astro/3d_models/rock_1.obj", 5000);
  meshes.rock_2        = MODEL_MESH("./astro/3d_models/rock_2.obj", 5000);
  meshes.rock_step     = MODEL_MESH("./astro/3d_models/rock_step.obj", 1000);
  meshes.river_edge    = MODEL_MESH("./astro/3d_models/river_edge.obj", 5000);
  meshes.ground_1      = MODEL_MESH("./astro/3d_models/ground_1.obj", 5000);
  meshes.lookout_tower = MODEL_MESH("./astro/3d_models/decoratives/lookout_tower.obj", 100);
  meshes.stairs_floor  = MODEL_MESH("./astro/3d_models/stairs_floor.obj", 1000);

  mesh(meshes.flat_ground).shadow_cascade_ceiling = 0;
  mesh(meshes.rock_1).shadow_cascade_ceiling = 2;
  mesh(meshes.rock_1).texture = "./astro/textures/rock_1.png";
  mesh(meshes.rock_2).shadow_cascade_ceiling = 2;
  mesh(meshes.rock_2).texture = "./astro/textures/rock_2.png";
  mesh(meshes.rock_step).shadow_cascade_ceiling = 1;
  mesh(meshes.rock_step).texture = "./astro/textures/rock_2.png";
  mesh(meshes.river_edge).shadow_cascade_ceiling = 2;
  mesh(meshes.ground_1).shadow_cascade_ceiling = 2;
  mesh(meshes.stairs_floor).shadow_cascade_ceiling = 2;
}

static void AddFacadeMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  meshes.facade_sg_castle = MODEL_MESH("./astro/3d_models/facades/sg_castle.obj", 1);

  // @todo shadow_cascade_floor
  mesh(meshes.facade_sg_castle).shadow_cascade_ceiling = 3;
}

static void AddEntityMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  for_all_entity_types() {
    EntityDispatcher::AddMeshes(tachyon, state, type);
  }

  mesh(meshes.flower_bush_placeholder).type = GRASS_MESH;
  mesh(meshes.flower_bush_placeholder).shadow_cascade_ceiling = 2;
  mesh(meshes.flower_bush_leaves).type = GRASS_MESH;
  mesh(meshes.flower_bush_leaves).shadow_cascade_ceiling = 2;
}

static void AddItemMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  meshes.item_astro_part = CUBE_MESH(3);
  meshes.item_gate_key = MODEL_MESH("./astro/3d_models/gate_key.obj", 1);
}

static void AddProceduralMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // grass
  {
    meshes.grass = MODEL_MESH("./astro/3d_models/ground_1_grass.obj", 10000);

    mesh(meshes.grass).type = GRASS_MESH;
    mesh(meshes.grass).shadow_cascade_ceiling = 2;
  }

  // small_grass
  {
    meshes.small_grass = MODEL_MESH_LOD_2("./astro/3d_models/small_grass.obj", "./astro/3d_models/small_grass_lod.obj", 50000);

    mesh(meshes.small_grass).type = GRASS_MESH;
    mesh(meshes.small_grass).shadow_cascade_ceiling = 2;
    mesh(meshes.small_grass).use_lowest_lod_for_shadows = true;
  }

  // ground_flower
  {
    meshes.ground_flower = MODEL_MESH("./astro/3d_models/flower.obj", 30000);

    mesh(meshes.ground_flower).type = GRASS_MESH;
    mesh(meshes.ground_flower).shadow_cascade_ceiling = 2;
  }

  // tiny_ground_flowers
  {
    meshes.tiny_ground_flower = MODEL_MESH("./astro/3d_models/tiny_flower.obj", 30000);

    mesh(meshes.tiny_ground_flower).type = GRASS_MESH;
    mesh(meshes.tiny_ground_flower).shadow_cascade_ceiling = 0;
  }

  // ground_1_flower
  {
    meshes.ground_1_flower = MODEL_MESH("./astro/3d_models/ground_1_flower.obj", 1000);

    mesh(meshes.ground_1_flower).type = GRASS_MESH;
    mesh(meshes.ground_1_flower).shadow_cascade_ceiling = 0;
  }

  // bush_flower
  {
    meshes.bush_flower = MODEL_MESH("./astro/3d_models/flower.obj", 500);
    meshes.bush_flower_2 = MODEL_MESH("./astro/3d_models/flower2.obj", 500);
    meshes.flower_middle = MODEL_MESH("./astro/3d_models/flower_middle.obj", 500);

    mesh(meshes.bush_flower).type = GRASS_MESH;
    mesh(meshes.bush_flower).shadow_cascade_ceiling = 2;
    mesh(meshes.bush_flower_2).type = GRASS_MESH;
    mesh(meshes.bush_flower_2).shadow_cascade_ceiling = 2;
    mesh(meshes.flower_middle).type = GRASS_MESH;
    mesh(meshes.flower_middle).shadow_cascade_ceiling = 0;
  }

  // tree_mushroom
  {
    meshes.tree_mushroom = MODEL_MESH("./astro/3d_models/tree_mushroom.obj", 500);

    mesh(meshes.tree_mushroom).shadow_cascade_ceiling = 2;
  }

  // vine_leaf + vine_flower
  {
    meshes.vine_leaf = MODEL_MESH("./astro/3d_models/vine_leaf.obj", 1000);
    meshes.vine_flower = MODEL_MESH("./astro/3d_models/vine_flower.obj", 300);

    mesh(meshes.vine_leaf).shadow_cascade_ceiling = 1;
    mesh(meshes.vine_leaf).type = FOLIAGE_MESH;

    mesh(meshes.vine_flower).shadow_cascade_ceiling = 1;
    mesh(meshes.vine_flower).type = FOLIAGE_MESH;
  }

  // tree_flower
  {
    meshes.tree_flower = MODEL_MESH("./astro/3d_models/tree_flower.obj", 5000);

    mesh(meshes.tree_flower).shadow_cascade_ceiling = 1;
    mesh(meshes.tree_flower).type = FOLIAGE_MESH;
    mesh(meshes.tree_flower).use_disocclusion = true;
  }

  // Dirt paths
  {
    meshes.dirt_path = MODEL_MESH("./astro/3d_models/dirt_path.obj", 10000);
    meshes.rock_dirt = MODEL_MESH("./astro/3d_models/rock_dirt.obj", 400);

    mesh(meshes.dirt_path).shadow_cascade_ceiling = 0;
    mesh(meshes.rock_dirt).shadow_cascade_ceiling = 0;

    mesh(meshes.dirt_path).texture = "./astro/textures/dirt.png";
  }

  // stone_path
  {
    // @todo update model
    meshes.stone_path = MODEL_MESH("./astro/3d_models/dirt_path.obj", 10000);
    meshes.path_stone = MODEL_MESH("./astro/3d_models/path_stone.obj", 1000);

    mesh(meshes.stone_path).shadow_cascade_ceiling = 0;
    mesh(meshes.path_stone).shadow_cascade_ceiling = 0;
  }
}

static void AddFaunaMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  meshes.butterfly_left_wing = MODEL_MESH("./astro/3d_models/fauna/butterfly_left_wing.obj", 10);
  meshes.butterfly_right_wing = MODEL_MESH("./astro/3d_models/fauna/butterfly_right_wing.obj", 10);

  meshes.tiny_bird_head = MODEL_MESH("./astro/3d_models/small_bird/head.obj", 50);
  meshes.tiny_bird_body = MODEL_MESH("./astro/3d_models/small_bird/body.obj", 50);
  meshes.tiny_bird_wings = MODEL_MESH("./astro/3d_models/small_bird/wings.obj", 50);
  meshes.tiny_bird_left_wing = MODEL_MESH("./astro/3d_models/small_bird/left_wing.obj", 50);
  meshes.tiny_bird_right_wing = MODEL_MESH("./astro/3d_models/small_bird/right_wing.obj", 50);

  mesh(meshes.butterfly_left_wing).shadow_cascade_ceiling = 1;
  mesh(meshes.butterfly_right_wing).shadow_cascade_ceiling = 1;

  mesh(meshes.tiny_bird_body).shadow_cascade_ceiling = 1;
  mesh(meshes.tiny_bird_head).shadow_cascade_ceiling = 1;
  mesh(meshes.tiny_bird_wings).shadow_cascade_ceiling = 1;
  mesh(meshes.tiny_bird_left_wing).shadow_cascade_ceiling = 1;
  mesh(meshes.tiny_bird_right_wing).shadow_cascade_ceiling = 1;
}

// @todo move to engine
static void TransformBonesIntoMeshSpace(tSkeleton& skeleton) {
  for (auto& bone : skeleton.bones) {
    int32 parent_index = bone.parent_bone_index;

    while (parent_index != -1) {
      auto& parent = skeleton.bones[parent_index];

      bone.translation = parent.translation + parent.rotation.toMatrix4f() * bone.translation;
      bone.rotation = parent.rotation * bone.rotation;

      parent_index = parent.parent_bone_index;
    }
  }
}

static void AddSkinnedPlayerMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;
  auto& rest_pose = state.player_mesh_animation.rest_pose;

  rest_pose = GltfLoader("./astro/3d_skeleton_animations/player_skeleton.gltf").skeleton;

  TransformBonesIntoMeshSpace(rest_pose);

  // Compute inverse bind matrices
  {
    for (auto& bone : rest_pose.bones) {
      tMat4f inverse_bind_matrix = tMat4f::transformation(bone.translation, tVec3f(1.f), bone.rotation).inverse();

      rest_pose.bone_matrices.push_back(inverse_bind_matrix);
    }
  }

  tSkinnedMesh player_hood = Tachyon_LoadSkinnedMesh(
    "./astro/3d_models/characters/player_hood.skin",
    rest_pose
  );

  tSkinnedMesh player_robes = Tachyon_LoadSkinnedMesh(
    "./astro/3d_models/characters/player_robes.skin",
    rest_pose
  );

  tSkinnedMesh player_trim = Tachyon_LoadSkinnedMesh(
    "./astro/3d_models/characters/player_trim.skin",
    rest_pose
  );

  tSkinnedMesh player_shirt = Tachyon_LoadSkinnedMesh(
    "./astro/3d_models/characters/player_shirt.skin",
    rest_pose
  );

  tSkinnedMesh player_pants = Tachyon_LoadSkinnedMesh(
    "./astro/3d_models/characters/player_pants.skin",
    rest_pose
  );

  tSkinnedMesh player_boots = Tachyon_LoadSkinnedMesh(
    "./astro/3d_models/characters/player_boots.skin",
    rest_pose
  );

  tSkinnedMesh player_belt = Tachyon_LoadSkinnedMesh(
    "./astro/3d_models/characters/player_belt.skin",
    rest_pose
  );

  meshes.player_hood = Tachyon_AddSkinnedMesh(tachyon, player_hood);
  meshes.player_robes = Tachyon_AddSkinnedMesh(tachyon, player_robes);
  meshes.player_trim = Tachyon_AddSkinnedMesh(tachyon, player_trim);
  meshes.player_shirt = Tachyon_AddSkinnedMesh(tachyon, player_shirt);
  meshes.player_pants = Tachyon_AddSkinnedMesh(tachyon, player_pants);
  meshes.player_boots = Tachyon_AddSkinnedMesh(tachyon, player_boots);
  meshes.player_belt = Tachyon_AddSkinnedMesh(tachyon, player_belt);
}

static void AddSkinnedPersonMeshes(Tachyon* tachyon, State& state) {
  // Share the rest pose skeleton among the meshes
  tSkeleton rest_pose_skeleton = GltfLoader("./astro/3d_skeleton_animations/player_skeleton.gltf").skeleton;

  TransformBonesIntoMeshSpace(rest_pose_skeleton);

  // Compute inverse bind matrices
  for (auto& bone : rest_pose_skeleton.bones) {
    tMat4f inverse_bind_matrix = tMat4f::transformation(bone.translation, tVec3f(1.f), bone.rotation).inverse();

    rest_pose_skeleton.bone_matrices.push_back(inverse_bind_matrix);
  }

  for_range(0, MAX_ANIMATED_PEOPLE - 1) {
    auto& skin = state.person_skinned_meshes[i];

    skin.animation.rest_pose = rest_pose_skeleton;

    // Load and add the mesh
    {
      tSkinnedMesh person = Tachyon_LoadSkinnedMesh(
        "./astro/3d_models/characters/person.skin",
        skin.animation.rest_pose
      );

      skin.mesh_index = Tachyon_AddSkinnedMesh(tachyon, person);
    }
  }
}

static void AddClothingAndArmorMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  meshes.lesser_helmet = MODEL_MESH("./astro/3d_models/clothing_and_armor/lesser_helmet.obj", 10);

  mesh(meshes.lesser_helmet).shadow_cascade_ceiling = 2;
}

static void AddEditorMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  meshes.gizmo_arrow   = MODEL_MESH("./astro/3d_models/editor/gizmo_arrow.obj", 3);
  meshes.gizmo_resizer = MODEL_MESH("./astro/3d_models/editor/gizmo_resizer.obj", 3);
  meshes.gizmo_rotator = MODEL_MESH("./astro/3d_models/editor/gizmo_rotator.obj", 3);
  meshes.editor_placer = MODEL_MESH("./astro/3d_models/editor/placer.obj", 1);

  mesh(meshes.gizmo_arrow).shadow_cascade_ceiling = 0;
  mesh(meshes.gizmo_resizer).shadow_cascade_ceiling = 0;
  mesh(meshes.gizmo_rotator).shadow_cascade_ceiling = 0;

  mesh(meshes.editor_placer).type = ION_THRUSTER_MESH;
  mesh(meshes.editor_placer).shadow_cascade_ceiling = 0;
}

static void AddDebugMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  meshes.debug_skeleton_bone = CUBE_MESH(1000);
}

void MeshLibrary::AddMeshes(Tachyon* tachyon, State& state) {
  log_time("AddMeshes()");

  auto& meshes = state.meshes;

  // @todo factor
  {
    meshes.player_head =    MODEL_MESH("./astro/3d_models/characters/player_head.obj", 1);
    meshes.player_wand =    MODEL_MESH("./astro/3d_models/characters/player_wand.obj", 1);
    meshes.player_satchel = MODEL_MESH("./astro/3d_models/characters/player_satchel.obj", 1);
    meshes.player_lantern = SPHERE_MESH(1, 8);

    mesh(meshes.player_head).shadow_cascade_ceiling = 2;
    mesh(meshes.player_wand).shadow_cascade_ceiling = 2;
    mesh(meshes.player_satchel).shadow_cascade_ceiling = 1;
    mesh(meshes.player_lantern).shadow_cascade_ceiling = 0;
  }

  meshes.water_plane = PLANE_MESH(1);
  // @temporary
  meshes.snow_particle = MODEL_MESH("./astro/3d_models/snow.obj", 100);
  meshes.stray_leaf = MODEL_MESH("./astro/3d_models/stray_leaf.obj", 30);
  meshes.dust_mote = SPHERE_MESH(1000, 6);

  mesh(meshes.water_plane).type = WATER_MESH;
  mesh(meshes.water_plane).shadow_cascade_ceiling = 0;

  mesh(meshes.snow_particle).shadow_cascade_ceiling = 0;
  mesh(meshes.stray_leaf).shadow_cascade_ceiling = 0;

  // @temporary @todo use GLOW_PARTICLE_MESH once it exists
  mesh(meshes.dust_mote).type = VOLUMETRIC_MESH;
  mesh(meshes.dust_mote).shadow_cascade_ceiling = 0;

  AddHUDMeshes(tachyon, state);
  AddDecorativeMeshes(tachyon, state);
  AddFacadeMeshes(tachyon, state);
  AddEntityMeshes(tachyon, state);
  AddItemMeshes(tachyon, state);
  AddProceduralMeshes(tachyon, state);
  AddFaunaMeshes(tachyon, state);
  AddSkinnedPlayerMeshes(tachyon, state);
  AddSkinnedPersonMeshes(tachyon, state);
  AddClothingAndArmorMeshes(tachyon, state);

  // @todo dev mode only
  {
    AddEditorMeshes(tachyon, state);
    AddDebugMeshes(tachyon, state);
  }

  Tachyon_InitializeObjects(tachyon);
}