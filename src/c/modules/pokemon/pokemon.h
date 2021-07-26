#pragma once

#include <pebble.h>
#include "../pebble-gbc-graphics/pebble-gbc-graphics.h"

#define SAVE_KEY 1

#if defined(PBL_PLATFORM_DIORITE)
    #define FRAME_SKIP 8
#else
    #define FRAME_SKIP 1
#endif

#define PLAYER_ORIGIN_X 16*51
#define PLAYER_ORIGIN_Y 16*13

#define TILE_BANK_WORLD 0
#define TILE_BANK_SPRITES 0
#define TILE_OFFSET_SPRITES 180
#define TILE_BANK_ANIMS 0
#define TILE_OFFSET_ANIMS 220

#define DEMO_MODE false

typedef struct {
    uint16_t player_step;
} PokemonSaveData;

void Pokemon_initialize(GBC_Graphics *graphics);
void Pokemon_deinitialize(GBC_Graphics *graphics);
bool Pokemon_step(GBC_Graphics *graphics);
void Pokemon_handle_tap(GBC_Graphics *graphics);
void Pokemon_handle_focus_lost(GBC_Graphics *graphics);
void Pokemon_start_animation(GBC_Graphics *graphics);