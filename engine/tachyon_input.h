#pragma once

#include <SDL_events.h>

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_types.h"

#define did_press_key(key) (tachyon->pressed_key_state & (uint64)key)
#define is_key_held(key) (tachyon->held_key_state & (uint64)key)
#define did_release_key(key) (tachyon->released_key_state & (uint64)key)
#define get_text_input() (tachyon->text_input)
#define did_wheel_up() (tachyon->wheel_direction > 0)
#define did_wheel_down() (tachyon->wheel_direction < 0)
#define did_left_click_down() (tachyon->did_left_click_down)
#define did_left_click_up() (tachyon->did_left_click_up)
#define did_right_click_down() (tachyon->did_right_click_down)
#define did_right_click_up() (tachyon->did_right_click_up)
#define is_mouse_held_down() (tachyon->is_mouse_held_down)
#define is_left_mouse_held_down() (tachyon->is_left_mouse_held_down)
#define is_right_mouse_held_down() (tachyon->is_right_mouse_held_down)

enum class tKey : uint64 {
  A = 1ULL << 0,
  B = 1ULL << 1,
  C = 1ULL << 2,
  D = 1ULL << 3,
  E = 1ULL << 4,
  F = 1ULL << 5,
  G = 1ULL << 6,
  H = 1ULL << 7,
  I = 1ULL << 8,
  J = 1ULL << 9,
  K = 1ULL << 10,
  L = 1ULL << 11,
  M = 1ULL << 12,
  N = 1ULL << 13,
  O = 1ULL << 14,
  P = 1ULL << 15,
  Q = 1ULL << 16,
  R = 1ULL << 17,
  S = 1ULL << 18,
  T = 1ULL << 19,
  U = 1ULL << 20,
  V = 1ULL << 21,
  W = 1ULL << 22,
  X = 1ULL << 23,
  Y = 1ULL << 24,
  Z = 1ULL << 25,
  NUM_0 = 1ULL << 26,
  NUM_1 = 1ULL << 27,
  NUM_2 = 1ULL << 28,
  NUM_3 = 1ULL << 29,
  NUM_4 = 1ULL << 30,
  NUM_5 = 1ULL << 31,
  NUM_6 = 1ULL << 32,
  NUM_7 = 1ULL << 33,
  NUM_8 = 1ULL << 34,
  NUM_9 = 1ULL << 35,
  ARROW_LEFT = 1ULL << 36,
  ARROW_RIGHT = 1ULL << 37,
  ARROW_UP = 1ULL << 38,
  ARROW_DOWN = 1ULL << 39,
  SPACE = 1ULL << 40,
  SHIFT = 1ULL << 41,
  ESCAPE = 1ULL << 42,
  ENTER = 1ULL << 43,
  CONTROL = 1ULL << 44,
  BACKSPACE = 1ULL << 45,
  TAB = 1ULL << 46,
  ALT = 1ULL << 47,
  CONTROLLER_A = 1ULL << 48,
  CONTROLLER_B = 1ULL << 49,
  CONTROLLER_X = 1ULL << 50,
  CONTROLLER_Y = 1ULL << 51,
  CONTROLLER_L1 = 1ULL << 52,
  CONTROLLER_R1 = 1ULL << 53
};

void Tachyon_HandleInputEvent(Tachyon* tachyon, const SDL_Event& event);
void Tachyon_ResetPerFrameInputState(Tachyon* tachyon);