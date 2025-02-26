#include <map>

#include "engine/tachyon_input.h"
#include "engine/tachyon_types.h"

const static std::map<SDL_Keycode, tKey> key_map = {
  { SDLK_a, tKey::A },
  { SDLK_b, tKey::B },
  { SDLK_c, tKey::C },
  { SDLK_d, tKey::D },
  { SDLK_e, tKey::E },
  { SDLK_f, tKey::F },
  { SDLK_g, tKey::G },
  { SDLK_h, tKey::H },
  { SDLK_i, tKey::I },
  { SDLK_j, tKey::J },
  { SDLK_k, tKey::K },
  { SDLK_l, tKey::L },
  { SDLK_m, tKey::M },
  { SDLK_n, tKey::N },
  { SDLK_o, tKey::O },
  { SDLK_p, tKey::P },
  { SDLK_q, tKey::Q },
  { SDLK_r, tKey::R },
  { SDLK_s, tKey::S },
  { SDLK_t, tKey::T },
  { SDLK_u, tKey::U },
  { SDLK_v, tKey::V },
  { SDLK_w, tKey::W },
  { SDLK_x, tKey::X },
  { SDLK_y, tKey::Y },
  { SDLK_z, tKey::Z },
  { SDLK_0, tKey::NUM_0 },
  { SDLK_1, tKey::NUM_1 },
  { SDLK_2, tKey::NUM_2 },
  { SDLK_3, tKey::NUM_3 },
  { SDLK_4, tKey::NUM_4 },
  { SDLK_5, tKey::NUM_5 },
  { SDLK_6, tKey::NUM_6 },
  { SDLK_7, tKey::NUM_7 },
  { SDLK_8, tKey::NUM_8 },
  { SDLK_9, tKey::NUM_9 },
  { SDLK_LEFT, tKey::ARROW_LEFT },
  { SDLK_RIGHT, tKey::ARROW_RIGHT },
  { SDLK_UP, tKey::ARROW_UP },
  { SDLK_DOWN, tKey::ARROW_DOWN },
  { SDLK_SPACE, tKey::SPACE },
  { SDLK_LSHIFT, tKey::SHIFT },
  { SDLK_RSHIFT, tKey::SHIFT },
  { SDLK_ESCAPE, tKey::ESCAPE },
  { SDLK_RETURN, tKey::ENTER },
  { SDLK_LCTRL, tKey::CONTROL },
  { SDLK_BACKSPACE, tKey::BACKSPACE },
  { SDLK_TAB, tKey::TAB },
  { SDLK_LALT, tKey::ALT }
};

void Tachyon_HandleInputEvent(Tachyon* tachyon, const SDL_Event& event) {
  switch (event.type) {
    case SDL_KEYDOWN: {
      auto code = event.key.keysym.sym;

      if (key_map.find(code) != key_map.end()) {
        auto key = (uint64)key_map.at(code);

        if (!is_key_held(key)) {
          tachyon->pressed_key_state |= key;
        }

        tachyon->held_key_state |= key;
      }
      break;
    }
    case SDL_KEYUP: {
      auto code = event.key.keysym.sym;

      if (key_map.find(code) != key_map.end()) {
        auto key = (uint64)key_map.at(code);

        tachyon->held_key_state &= ~key;
        tachyon->pressed_key_state &= ~key;
        tachyon->released_key_state |= key;
      }
      break;
    }
    case SDL_MOUSEMOTION: {
      tachyon->mouse_delta_x = event.motion.xrel;
      tachyon->mouse_delta_y = event.motion.yrel;
      break;
    }
    case SDL_MOUSEBUTTONDOWN: {
      if (event.button.button == SDL_BUTTON_LEFT) {
        tachyon->did_left_click_down = true;
      } else if (event.button.button == SDL_BUTTON_RIGHT) {
        tachyon->did_right_click_down = true;
      }

      tachyon->is_mouse_held_down = true;

      break;
    }
    case SDL_MOUSEBUTTONUP: {
      if (event.button.button == SDL_BUTTON_LEFT) {
        tachyon->did_left_click_up = true;
      } else if (event.button.button == SDL_BUTTON_RIGHT) {
        tachyon->did_right_click_up = true;
      }

      tachyon->is_mouse_held_down = false;

      break;
    }
    case SDL_MOUSEWHEEL: {
      tachyon->wheel_direction = event.wheel.y;
      break;
    }
  }
}

void Tachyon_ResetPerFrameInputState(Tachyon* tachyon) {
  tachyon->pressed_key_state = 0;
  tachyon->released_key_state = 0;
  tachyon->mouse_delta_x = 0;
  tachyon->mouse_delta_y = 0;
  tachyon->wheel_direction = 0;
  tachyon->did_left_click_down = false;
  tachyon->did_left_click_up = false;
  tachyon->did_right_click_down = false;
  tachyon->did_right_click_up = false;
}