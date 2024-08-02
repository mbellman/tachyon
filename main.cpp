#include <stdio.h>

#include <SDL.h>

#include "engine/tachyon.h"
#include "cosmodrone/start.h"

int main(int argc, char* argv[]) {
  auto* tachyon = Tachyon_Init();

  Tachyon_CreateWindow(tachyon);

  Cosmodrone::StartGame(tachyon);

  while (Tachyon_IsRunning(tachyon)) {
    Tachyon_HandleEvents(tachyon);

    SDL_Delay(16);
  }

  Tachyon_Exit(tachyon);

  return 0;
}