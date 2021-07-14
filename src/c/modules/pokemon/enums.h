#pragma once

#include <pebble.h>

typedef enum {
  D_UP,
  D_LEFT,
  D_DOWN,
  D_RIGHT
} PlayerDirection;

typedef enum {
  P_STAND,
  P_WALK,
  P_RUN,
  P_JUMP,
  P_WARP,
  P_WARP_WALK
} PlayerMode;