#pragma once

#include "engine/tachyon_types.h"

tMesh Tachyon_LoadMesh(const char* path);
void Tachyon_AddMesh(Tachyon* tachyon, const tMesh& mesh);