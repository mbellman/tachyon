#pragma once

#include "engine/tachyon_linear_algebra.h"
#include "engine/tachyon_types.h"

#define add_mesh(__mesh, __total) Tachyon_AddMesh(tachyon, __mesh, __total)
#define mesh(__mesh_index) tachyon->mesh_pack.mesh_records[__mesh_index]
#define skinned_mesh(__mesh_index) tachyon->skinned_meshes[__mesh_index]
#define objects(__mesh_index) tachyon->mesh_pack.mesh_records[__mesh_index].group
#define create(__mesh_index) Tachyon_CreateObject(tachyon, __mesh_index)
#define remove_object(...) Tachyon_RemoveObject(tachyon, __VA_ARGS__)
#define remove_all(__mesh_index) Tachyon_RemoveAllObjects(tachyon, __mesh_index)
#define commit(__object_or_skinned_mesh) Tachyon_Commit(tachyon, __object_or_skinned_mesh)
#define get_live_object(__object) Tachyon_GetLiveObject(tachyon, __object)

#define reset_instances(__mesh_index)\
  mesh(__mesh_index).lod_1.instance_count = 0;\
  mesh(__mesh_index).lod_2.instance_count = 0;\
  mesh(__mesh_index).lod_3.instance_count = 0

#define use_instance(__mesh_index)\
  objects(__mesh_index)[mesh(__mesh_index).lod_1.instance_count++]

#define create_point_light() Tachyon_CreatePointLight(tachyon)
#define get_point_light(__light_id) Tachyon_GetPointLight(tachyon, __light_id)
#define remove_point_light(__light) Tachyon_RemovePointLight(tachyon, __light)

tMesh Tachyon_LoadMesh(const char* path, const tVec3f& axis_factors = tVec3f(1.f));
tSkinnedMesh Tachyon_LoadSkinnedMesh(const char* obj_path, const char* skin_path, const tSkeleton& skeleton);
tMesh Tachyon_CreatePlaneMesh();
tMesh Tachyon_CreateCubeMesh();
tMesh Tachyon_CreateSphereMesh(uint8 divisions);
uint16 Tachyon_AddMesh(Tachyon* tachyon, const tMesh& mesh, uint16 total);
uint16 Tachyon_AddMesh(Tachyon* tachyon, const tMesh& mesh, const tMesh& mesh2, uint16 total);
uint16 Tachyon_AddMesh(Tachyon* tachyon, const tMesh& mesh, const tMesh& mesh2, const tMesh& mesh3, uint16 total);
int32 Tachyon_AddSkinnedMesh(Tachyon* tachyon, const tSkinnedMesh& skinned_mesh);
void Tachyon_InitializeObjects(Tachyon* tachyon);
tObject& Tachyon_CreateObject(Tachyon* tachyon, uint16 mesh_index);
void Tachyon_RemoveObject(Tachyon* tachyon, uint16 mesh_index, uint16 object_id);
void Tachyon_RemoveObject(Tachyon* tachyon, tObject& object);
void Tachyon_RemoveAllObjects(Tachyon* tachyon, uint16 mesh_index);
void Tachyon_Commit(Tachyon* tachyon, const tObject& object);
void Tachyon_Commit(Tachyon* tachyon, tSkinnedMesh& skinned_mesh);
tObject* Tachyon_GetLiveObject(Tachyon* tachyon, const tObject& object);
uint16 Tachyon_PartitionObjectsByDistance(Tachyon* tachyon, tObjectGroup& group, const uint16 start, const float distance);
void Tachyon_UseLodByDistance(Tachyon* tachyon, const uint16 mesh_index, const float distance);
void Tachyon_UseLodByDistance(Tachyon* tachyon, const uint16 mesh_index, const float distance, const float distance2);
void Tachyon_ShowHighestLevelsOfDetail(Tachyon* tachyon, uint16 mesh_index);

int32 Tachyon_CreatePointLight(Tachyon* tachyon);
tPointLight* Tachyon_GetPointLight(Tachyon* tachyon, int32 light_id);
void Tachyon_RemovePointLight(Tachyon* tachyon, int32 light_id);
void Tachyon_RemovePointLight(Tachyon* tachyon, tPointLight& light);