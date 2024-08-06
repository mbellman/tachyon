#include "cosmodrone/game.h"

void Cosmodrone::StartGame(Tachyon* tachyon) {
  auto mesh = Tachyon_LoadMesh("./cosmodrone/assets/test.obj");

  Tachyon_AddMesh(tachyon, mesh);
}

void Cosmodrone::RunGame(Tachyon* tachyon) {

}