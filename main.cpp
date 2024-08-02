#include <stdio.h>

#include "engine/tachyon.h"
#include "cosmodrone/start.h"

int main(int argc, char* argv[]) {
  auto* tachyon = Tachyon_Init();

  Cosmodrone::StartGame(tachyon);

  printf("Tachyon!\n");

  return 0;
}