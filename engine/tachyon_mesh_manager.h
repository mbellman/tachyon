#pragma once

#include "engine/tachyon_types.h"

tMesh Tachyon_LoadMesh(const char* path);
uint32 Tachyon_AddMesh(Tachyon* tachyon, const tMesh& mesh);
void Tachyon_InitializeObjects(Tachyon* tachyon);
void Tachyon_CommitObject(Tachyon* tachyon, const tObject& object);