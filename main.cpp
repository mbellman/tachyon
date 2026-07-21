#include <stdio.h>

#include "engine/tachyon.h"

// -----
// COSMO

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

// -----
// ASTRO

// #include "astro/game.h"

// int main(int argc, char* argv[]) {
//   auto* tachyon = Tachyon_Init();

//   astro::State state;
//   astro::InitGame(tachyon, state);

//   Tachyon_SpawnWindow(tachyon, "Birdy Adventure", 1536, 850);
//   Tachyon_UseRenderBackend(tachyon, TachyonRenderBackend::OPENGL);

//   Tachyon_RunMainLoop({
//     astro::UpdateGame(tachyon, state, dt);
//   });

//   Tachyon_Exit(tachyon);

//   return 0;
// }

// -----
// METRO

#include "metro/game.h"

int main(int argc, char* argv[]) {
  auto* tachyon = Tachyon_Init();

  metro::State state;
  metro::Init(tachyon, state);

  Tachyon_SpawnWindow(tachyon, "Bicycle Adventure", 1536, 850);
  Tachyon_UseRenderBackend(tachyon, TachyonRenderBackend::OPENGL);

  Tachyon_RunMainLoop({
    metro::Update(tachyon, state, dt);
  });

  Tachyon_Exit(tachyon);

  return 0;
}