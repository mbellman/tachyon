#pragma once

#include "engine/tachyon_linear_algebra.h"
#include "engine/tachyon_types.h"

#define objects(__index) tachyon->mesh_pack.mesh_records[__index].group
#define create(__index) Tachyon_CreateObject(tachyon, __index);
#define commit(__object) Tachyon_CommitObject(tachyon, __object);

tMesh Tachyon_LoadMesh(const char* path, const tVec3f& axis_factors = tVec3f(1.f));
tMesh Tachyon_CreatePlaneMesh();
tMesh Tachyon_CreateSphereMesh(uint8 divisions);
uint32 Tachyon_AddMesh(Tachyon* tachyon, const tMesh& mesh, uint16 total);
void Tachyon_InitializeObjects(Tachyon* tachyon);
tObject& Tachyon_CreateObject(Tachyon* tachyon, uint16 mesh_index);
void Tachyon_CommitObject(Tachyon* tachyon, const tObject& object);