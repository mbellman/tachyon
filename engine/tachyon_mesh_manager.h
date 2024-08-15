#pragma once

#include "engine/tachyon_types.h"

#define objects(__index) tachyon->mesh_pack.mesh_records[__index].group
#define create(__index) Tachyon_CreateObject(tachyon, __index);
#define commit(__object) Tachyon_CommitObject(tachyon, __object);

tMesh Tachyon_LoadMesh(const char* path);
uint32 Tachyon_AddMesh(Tachyon* tachyon, const tMesh& mesh, uint16 total);
void Tachyon_InitializeObjects(Tachyon* tachyon);
tObject& Tachyon_CreateObject(Tachyon* tachyon, uint16 meshIndex);
void Tachyon_CommitObject(Tachyon* tachyon, const tObject& object);