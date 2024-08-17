#include <SDL_ttf.h>
#include <SDL_image.h>

#include "engine/tachyon_developer_overlay.h"
#include "engine/opengl/tachyon_opengl_renderer.h"

void Tachyon_InitDeveloperOverlay(Tachyon* tachyon) {
  tachyon->developer_overlay_font = TTF_OpenFont("./fonts/OpenSans-Regular.ttf", 16);
}

void Tachyon_DestroyDeveloperOverlay(Tachyon* tachyon) {
  TTF_CloseFont(tachyon->developer_overlay_font);
}