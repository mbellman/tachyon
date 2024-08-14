#include <stdio.h>

#include "engine/tachyon.h"
#include "cosmodrone/game.h"

int main(int argc, char* argv[]) {
  auto* tachyon = Tachyon_Init();

  Tachyon_SpawnWindow(tachyon, "Tachyon", 1536, 850);

  Cosmodrone::StartGame(tachyon);

  Tachyon_UseRenderBackend(tachyon, TachyonRenderBackend::OPENGL);

  Tachyon_Loop({
    Cosmodrone::RunGame(tachyon);
  });

  Tachyon_Exit(tachyon);

  return 0;
}