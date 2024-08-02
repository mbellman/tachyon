#include <stdio.h>

#include "engine/tachyon.h"
#include "cosmodrone/start.h"

int main(int argc, char* argv[]) {
  auto* tachyon = Tachyon_Init();

  Tachyon_CreateWindow(tachyon);
  Tachyon_UseRenderBackend(tachyon, TachyonRenderBackend::OPENGL);

  Cosmodrone::StartGame(tachyon);

  while (Tachyon_IsRunning(tachyon)) {
    Tachyon_HandleEvents(tachyon);
    Tachyon_RenderScene(tachyon);
  }

  Tachyon_Exit(tachyon);

  return 0;
}