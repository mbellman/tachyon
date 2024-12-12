#pragma once

#include "engine/tachyon_linear_algebra.h"
#include "engine/tachyon_types.h"

#define add_mesh(__mesh, __total) Tachyon_AddMesh(tachyon, __mesh, __total)
#define mesh(__mesh_index) tachyon->mesh_pack.mesh_records[__mesh_index]
#define objects(__mesh_index) tachyon->mesh_pack.mesh_records[__mesh_index].group
#define create(__mesh_index) Tachyon_CreateObject(tachyon, __mesh_index)
#define remove(__object) Tachyon_RemoveObject(tachyon, __object)
#define remove_all(__mesh_index) Tachyon_RemoveAllObjects(tachyon, __mesh_index);
#define commit(__object) Tachyon_CommitObject(tachyon, __object)
#define get_original_object(__object) Tachyon_GetOriginalObject(tachyon, __object)

tMesh Tachyon_LoadMesh(const char* path, const tVec3f& axis_factors = tVec3f(1.f));
tMesh Tachyon_CreatePlaneMesh();
tMesh Tachyon_CreateCubeMesh();
tMesh Tachyon_CreateSphereMesh(uint8 divisions);
uint16 Tachyon_AddMesh(Tachyon* tachyon, const tMesh& mesh, uint16 total);
void Tachyon_InitializeObjects(Tachyon* tachyon);
tObject& Tachyon_CreateObject(Tachyon* tachyon, uint16 mesh_index);
void Tachyon_RemoveObject(Tachyon* tachyon, tObject& object);
void Tachyon_RemoveAllObjects(Tachyon* tachyon, uint16 mesh_index);
void Tachyon_CommitObject(Tachyon* tachyon, const tObject& object);
tObject* Tachyon_GetOriginalObject(Tachyon* tachyon, const tObject& object);
uint16 Tachyon_PartitionObjectsByDistance(Tachyon* tachyon, tObjectGroup& group, const uint16 start, const tCamera& camera, const float distance);