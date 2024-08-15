#pragma once

#include "engine/tachyon_types.h"

#define objects(__index) tachyon->mesh_pack.mesh_records[__index - 1].group
#define commit(__object) Tachyon_CommitObject(tachyon, __object);

tMesh Tachyon_LoadMesh(const char* path);
uint32 Tachyon_AddMesh(Tachyon* tachyon, const tMesh& mesh);
void Tachyon_InitializeObjects(Tachyon* tachyon);
void Tachyon_CommitObject(Tachyon* tachyon, const tObject& object);