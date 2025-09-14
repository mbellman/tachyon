#include <stdio.h>

#include "engine/tachyon.h"

// #include "cosmodrone/game.h"

// int main(int argc, char* argv[]) {
//   auto* tachyon = Tachyon_Init();

//   Tachyon_SpawnWindow(tachyon, "Cosmodrone", 1536, 850);

//   Cosmodrone::StartGame(tachyon);

//   Tachyon_UseRenderBackend(tachyon, TachyonRenderBackend::OPENGL);

//   Tachyon_RunMainLoop({
//     Cosmodrone::UpdateGame(tachyon, dt);
//   });

//   Tachyon_Exit(tachyon);

//   return 0;
// }

#include "astro/game.h"

int main(int argc, char* argv[]) {
  auto* tachyon = Tachyon_Init();

  Tachyon_SpawnWindow(tachyon, "Astrolabe", 1536, 850);

  astro::State state;
  astro::InitGame(tachyon, state);

  Tachyon_UseRenderBackend(tachyon, TachyonRenderBackend::OPENGL);

  Tachyon_RunMainLoop({
    astro::UpdateGame(tachyon, state, dt);
  });

  Tachyon_Exit(tachyon);

  return 0;
}