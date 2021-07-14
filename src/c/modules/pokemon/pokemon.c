#include "pokemon.h"
#include "world.h"
#include "sprites.h"
#include "objects.h"
#include "enums.h"
#include "animations.h"
#include "items.h"

// Player variables
static PlayerDirection s_player_direction = D_DOWN;
static PlayerMode s_player_mode;
static uint16_t s_player_x, s_player_y;
static uint8_t s_player_sprite_x, s_player_sprite_y;
static uint8_t *s_player_palette;
static uint8_t s_route_num = 3;
static uint16_t s_player_step;
static uint16_t s_atomic_player_x, s_atomic_player_y, s_atomic_player_step;

// Frame counters
static uint8_t s_walk_frame, s_anim_frame, s_step_frame;

// Movement
static bool s_flip_walk, s_can_move;
static uint16_t s_target_x, s_target_y;
static uint8_t s_warp_route;
static uint16_t s_warp_x, s_warp_y;

// Options
static uint8_t s_player_sprite = 0;

// Misc
static uint8_t *s_world_map;
static uint8_t s_cur_bg_palettes[PALETTE_BANK_SIZE];
static bool s_game_started;

static GPoint direction_to_point(PlayerDirection dir) {
    switch (dir) {
        case D_UP:  return GPoint(0, -1);
        case D_LEFT:  return GPoint(-1, 0);
        case D_DOWN:  return GPoint(0, 1);
        case D_RIGHT:  return GPoint(1, 0);
        default: return GPoint(0, 0);
    }
}

static uint8_t get_tile_id_from_map(uint8_t *map, uint16_t tile_x, uint16_t tile_y) {
  uint8_t chunk = map[(tile_x >> 2) + (tile_y >> 2) * route_dims[s_route_num << 1]];
  uint8_t block = chunks[s_route_num][chunk * 4 + ((tile_x >> 1) & 1) + ((tile_y >> 1) & 1) * 2];
  return blocks[s_route_num][block * 4 + (tile_x & 1) + (tile_y & 1) * 2];
}

static void load_tiles(GBC_Graphics *graphics, uint8_t bg_root_x, uint8_t bg_root_y, uint16_t tile_root_x, uint16_t tile_root_y, uint8_t num_x_tiles, uint8_t num_y_tiles) {
  for (uint16_t tile_y = tile_root_y; tile_y < tile_root_y + num_y_tiles; tile_y++) {
    for (uint16_t tile_x = tile_root_x; tile_x < tile_root_x + num_x_tiles; tile_x++) {
      uint8_t tile = get_tile_id_from_map(s_world_map, tile_x, tile_y);
    #if defined(PBL_COLOR)
      GBC_Graphics_bg_set_tile_and_attrs(graphics, bg_root_x + (tile_x - tile_root_x), bg_root_y + (tile_y - tile_root_y), tile, tile_palettes[s_route_num][tile]);
    #else
      GBC_Graphics_bg_set_tile_and_attrs(graphics, bg_root_x + (tile_x - tile_root_x), bg_root_y + (tile_y - tile_root_y), tile, 0);
    #endif
    }
  }
}

static void load_screen(GBC_Graphics *graphics) {
  for (uint8_t i = 0; i < 8; i++) {
  #if defined(PBL_COLOR)
    GBC_Graphics_set_bg_palette_array(graphics, i, &palettes[s_route_num][i*PALETTE_SIZE]);
  #else
    GBC_Graphics_set_bg_palette_array(graphics, i, bw_palette);
  #endif
  }
#if defined(PBL_COLOR)
  GBC_Graphics_set_sprite_palette_array(graphics, 4, &palettes[s_route_num][2*PALETTE_SIZE]);
#endif
  GBC_Graphics_copy_all_bg_palettes(graphics, s_cur_bg_palettes);
  GBC_Graphics_bg_set_scroll_pos(graphics, 0, 0);
  uint8_t bg_root_x = GBC_Graphics_bg_get_scroll_x(graphics) >> 3;
  uint8_t bg_root_y = GBC_Graphics_bg_get_scroll_y(graphics) >> 3;
  uint16_t tile_root_x = (s_player_x >> 3) - 8;
  uint16_t tile_root_y = (s_player_y >> 3) - 8;
  load_tiles(graphics, bg_root_x & 31, bg_root_y & 31, tile_root_x, tile_root_y, 18, 18);
}

static void reset_player_palette(GBC_Graphics *graphics) {
#if defined(PBL_COLOR)
  s_player_palette = pokemon_trainer_palettes[s_player_sprite];
  GBC_Graphics_set_sprite_palette(graphics, 0, s_player_palette[0], s_player_palette[1], s_player_palette[2], s_player_palette[3]);
#else
  GBC_Graphics_set_sprite_palette(graphics, 0, 1, 1, 0, 0);
#endif 
}

static void load_player_sprites(GBC_Graphics *graphics) {
  reset_player_palette(graphics);

  uint16_t spritesheet_offset = s_player_sprite * 6 * POKEMON_TILES_PER_SPRITE;
  GBC_Graphics_load_from_tilesheet_into_vram(graphics, RESOURCE_ID_DATA_SPRITESHEET,
    spritesheet_offset, POKEMON_TILES_PER_SPRITE * POKEMON_SPRITES_PER_TRAINER, TILE_OFFSET_SPRITES + 10, TILE_BANK_SPRITES); // offset 10 for effects and items
}

static void new_player_sprites(GBC_Graphics *graphics) {
  s_player_sprite = rand()%22;
  load_player_sprites(graphics);
}

static void move_player_sprites(GBC_Graphics *graphics, short delta_x, short delta_y) {
  for (uint8_t i = 0; i < 4; i++) {
    GBC_Graphics_oam_move_sprite(graphics, 2+i, delta_x, delta_y);
  }
}

static void set_player_sprites(GBC_Graphics *graphics, bool walk_sprite, bool x_flip) {
  uint8_t new_tile = pokemon_trainer_sprite_offsets[s_player_direction + walk_sprite * 4];
  for (uint8_t i = 0; i < 4; i++) {
    GBC_Graphics_oam_set_sprite_tile(graphics, 2 + i, TILE_OFFSET_SPRITES + (new_tile * POKEMON_TILES_PER_SPRITE) + i + 10);
    GBC_Graphics_oam_set_sprite_x_flip(graphics, 2 + i, x_flip);
  }

  if (x_flip) {
    GBC_Graphics_oam_swap_sprite_tiles(graphics, 2, 3);
    GBC_Graphics_oam_swap_sprite_tiles(graphics, 4, 5);
  }
}

static void render_items(GBC_Graphics *graphics) {
  for (uint8_t i = 0; i < 8; i++) {
    const int16_t *item = items[i];
    int item_x = item[1] << 4;
    int item_y = item[2] << 4;
    if (s_route_num == item[0]) {
      if (!item[4]) {
        int x_diff = item_x - s_player_x;
        int y_diff = item_y - s_player_y;
        if (abs(x_diff) <= 80 && abs(y_diff) <= 80) {
          GBC_Graphics_oam_set_sprite_pos(graphics, 20 + 0, x_diff + 72, y_diff + 76);
          GBC_Graphics_oam_set_sprite_pos(graphics, 20 + 1, x_diff + 72 + 8, y_diff + 76);
          GBC_Graphics_oam_set_sprite_pos(graphics, 20 + 2, x_diff + 72, y_diff + 76 + 8);
          GBC_Graphics_oam_set_sprite_pos(graphics, 20 + 3, x_diff + 72 + 8, y_diff + 76 + 8);
        }
      }
    }
  }
}

static bool load(PokemonSaveData *data) {
  return persist_read_data(SAVE_KEY, data, sizeof(PokemonSaveData)) != E_DOES_NOT_EXIST;
}

static uint8_t get_block_type(uint8_t *map, uint16_t x, uint16_t y) {
  uint16_t tile_x = (x >> 3);
  uint16_t tile_y = (y >> 3);
  uint8_t chunk = map[(tile_x >> 2) + (tile_y >> 2) * route_dims[s_route_num << 1]];
  uint8_t block = chunks[s_route_num][chunk * 4 + ((tile_x >> 1) & 1) + ((tile_y >> 1) & 1) * 2];
  return block_types[s_route_num][block];
}

static void set_block(uint8_t *map, uint16_t x, uint16_t y, uint8_t replacement) {
  uint16_t tile_x = (x >> 3);
  uint16_t tile_y = (y >> 3);
  uint8_t chunk = map[(tile_x >> 2) + (tile_y >> 2) * route_dims[s_route_num << 1]];
  chunks[s_route_num][chunk * 4 + ((tile_x >> 1) & 1) + ((tile_y >> 1) & 1) * 2] = replacement;
}

static void load_overworld(GBC_Graphics *graphics) {
  load_screen(graphics);
  
  for (uint8_t i = 0; i < 40; i++) {
      GBC_Graphics_oam_hide_sprite(graphics, i);
  }

  load_player_sprites(graphics);

  // Create grass and shadow effect sprites
  uint16_t spritesheet_offset = POKEMON_SPRITELET_GRASS * POKEMON_TILES_PER_SPRITE;
  GBC_Graphics_load_from_tilesheet_into_vram(graphics, RESOURCE_ID_DATA_SPRITESHEET, spritesheet_offset, 1, TILE_OFFSET_SPRITES + 0, TILE_BANK_SPRITES);
  GBC_Graphics_load_from_tilesheet_into_vram(graphics, RESOURCE_ID_DATA_SPRITESHEET, spritesheet_offset+2, 1, TILE_OFFSET_SPRITES + 1, TILE_BANK_SPRITES);
  GBC_Graphics_oam_set_sprite(graphics, 0, 0, 0, TILE_OFFSET_SPRITES + 0, GBC_Graphics_attr_make(6, TILE_BANK_SPRITES, false, false, false));
  GBC_Graphics_oam_set_sprite(graphics, 1, 0, 0, TILE_OFFSET_SPRITES + 0, GBC_Graphics_attr_make(6, TILE_BANK_SPRITES, true, false, false));
  GBC_Graphics_oam_set_sprite(graphics, 6, 0, 0, TILE_OFFSET_SPRITES + 1, GBC_Graphics_attr_make(0, TILE_BANK_SPRITES, false, false, false));
  GBC_Graphics_oam_set_sprite(graphics, 7, 0, 0, TILE_OFFSET_SPRITES + 1, GBC_Graphics_attr_make(0, TILE_BANK_SPRITES, true, false, false));
  #if defined(PBL_COLOR)
    GBC_Graphics_set_sprite_palette(graphics, 6, 0b11101101, 0b11011000, 0b11000100, 0b11000000);
  #else
    GBC_Graphics_set_sprite_palette(graphics, 6, 1, 1, 0, 0);
  #endif

  // Create item sprites
  spritesheet_offset = ITEM_SPRITE_POKEBALL * POKEMON_TILES_PER_SPRITE;
  GBC_Graphics_load_from_tilesheet_into_vram(graphics, RESOURCE_ID_DATA_SPRITESHEET, spritesheet_offset, 8, TILE_OFFSET_SPRITES + 2, TILE_BANK_SPRITES);
  for (uint8_t i = 0; i < 4; i++) {
    GBC_Graphics_oam_set_sprite(graphics, 20+i, 0, 0, TILE_OFFSET_SPRITES + 2+i, GBC_Graphics_attr_make(5, TILE_BANK_SPRITES, false, false, false));
  }
  for (uint8_t i = 0; i < 4; i++) {
    GBC_Graphics_oam_set_sprite(graphics, 24+i, 0, 0, TILE_OFFSET_SPRITES + 6+i, GBC_Graphics_attr_make(4, TILE_BANK_SPRITES, false, false, false));
  }
  #if defined(PBL_COLOR)
    GBC_Graphics_set_sprite_palette(graphics, 5, 0b11111111, 0b11111001, 0b11110000, 0b11000000);
    // To make sure the palete is correct for forest vs route, so it's set to background palette in load_screen instead
  #else
    GBC_Graphics_set_sprite_palette(graphics, 5, 1, 1, 0, 0);
    GBC_Graphics_set_sprite_palette(graphics, 4, 1, 1, 0, 0);
  #endif
  render_items(graphics);

  // Create player sprites
  for (uint8_t y = 0; y < 2; y++) {
    for (uint8_t x = 0; x < 2; x++) {
      GBC_Graphics_oam_set_sprite_pos(graphics, 2 + x + 2 * y, s_player_sprite_x + x * 8, s_player_sprite_y + y * 8 - 4);
      GBC_Graphics_oam_set_sprite_attrs(graphics, 2 + x + 2 * y, GBC_Graphics_attr_make(0, TILE_BANK_SPRITES, false, false, false));
    }
  }
  set_player_sprites(graphics, false, s_player_direction == D_RIGHT);

  if(get_block_type(s_world_map, s_player_x+8, s_player_y) == GRASS) {
#if defined(PBL_COLOR)
    GBC_Graphics_oam_set_sprite_priority(graphics, 4, true);
    GBC_Graphics_oam_set_sprite_priority(graphics, 5, true);
#else
    GBC_Graphics_oam_set_sprite_pos(graphics, 0, s_player_sprite_x, s_player_sprite_y + 4);
    GBC_Graphics_oam_set_sprite_pos(graphics, 1, s_player_sprite_x + 8, s_player_sprite_y + 4);
#endif
  }
}

static int check_for_object(uint16_t target_x, uint16_t target_y) {
  uint16_t block_x = target_x >> 4;
  uint16_t block_y = target_y >> 4;
  for (uint16_t i = 0; i < sizeof(objects) >> 2; i++) {
    if (s_route_num == objects[i][0] && block_x == objects[i][1] && block_y == objects[i][2]) {
      return i;
    }
  }
  return -1;
}

static void load_resources(GBC_Graphics *graphics) {
  ResHandle handle = resource_get_handle(map_files[s_route_num]);
  size_t res_size = resource_size(handle);
  if (s_world_map != NULL) {
    free(s_world_map);
    s_world_map = NULL;
  }
  s_world_map = (uint8_t*)malloc(res_size);
  resource_load(handle, s_world_map, res_size);
    
  // replace trees so that we can walk anywhere
  for (uint16_t i = 0; i < sizeof(objects) >> 2; i++) {
    if (s_route_num == objects[i][0] && objects[i][3] == PO_TREE) {
      set_block(s_world_map, objects[i][1] << 4, objects[i][2] << 4, replacement_blocks[s_route_num]);
    }
  }
  
  load_overworld(graphics);
}

static void load_game(GBC_Graphics *graphics) {
  s_player_mode = P_STAND;
  s_walk_frame = 0;
  s_player_sprite_x = 16 * 4 + SPRITE_OFFSET_X;
  s_player_sprite_y = 16 * 4 + SPRITE_OFFSET_Y;

  load_resources(graphics);
}

#if defined(PBL_COLOR)
static uint8_t lerp_uint8_t(uint8_t start, uint8_t end, float t) {
  return start + (end - start) * t;
}

static uint8_t lerp_color(uint8_t start, uint8_t end, uint8_t index) {
  float t = index / 4.0;
  uint8_t r_1 = ((start >> 4) & 0b11), r_2 = ((end >> 4) & 0b11);
  uint8_t g_1 = ((start >> 2) & 0b11), g_2 = ((end >> 2) & 0b11);
  uint8_t b_1 = ((start) & 0b11), b_2 = ((end) & 0b11);
  uint8_t r = lerp_uint8_t(r_1, r_2, t);
  uint8_t g = lerp_uint8_t(g_1, g_2, t);
  uint8_t b = lerp_uint8_t(b_1, b_2, t);
  return (0b11 << 6) | (r << 4) | (g << 2) | (b);
}

static void lerp_palette(uint8_t *start, uint8_t *end, uint8_t index, uint8_t *output) {
  for (uint8_t c = 0; c < 4; c++) {
    output[c] = lerp_color(start[c], end[c], index);
  }
}
#endif

void Pokemon_initialize(GBC_Graphics *graphics) {
  GBC_Graphics_set_screen_bounds(graphics, SCREEN_BOUNDS_SQUARE);
  GBC_Graphics_window_set_offset_pos(graphics, 0, 168);
  GBC_Graphics_lcdc_set_8x16_sprite_mode_enabled(graphics, false);
  for (uint8_t i = 0; i < 40; i++) {
      GBC_Graphics_oam_hide_sprite(graphics, i);
  }

  for (uint8_t i = 0; i < 8; i++) {
  #if defined(PBL_COLOR)
    GBC_Graphics_set_bg_palette_array(graphics, i, &palettes[s_route_num][i*PALETTE_SIZE]);
  #else
    GBC_Graphics_set_bg_palette_array(graphics, i, bw_palette);
  #endif
  }

  init_anim_tiles(graphics, TILE_BANK_ANIMS, TILE_OFFSET_ANIMS);

  ResHandle handle = resource_get_handle(RESOURCE_ID_DATA_WORLD_TILESHEET);
  size_t res_size = resource_size(handle);
  uint16_t tiles_to_load = res_size / 16;
  GBC_Graphics_load_from_tilesheet_into_vram(graphics, RESOURCE_ID_DATA_WORLD_TILESHEET, 0, tiles_to_load, 0, TILE_BANK_WORLD);

  GBC_Graphics_stat_set_line_compare_interrupt_enabled(graphics, false);
  GBC_Graphics_lcdc_set_bg_layer_enabled(graphics, true);
  GBC_Graphics_lcdc_set_window_layer_enabled(graphics, true);
  GBC_Graphics_lcdc_set_sprite_layer_enabled(graphics, true);

  GBC_Graphics_bg_set_scroll_pos(graphics, 0, 0);

  PokemonSaveData data;
  if (load(&data)) {
    s_route_num = data.route_num;
    s_player_x = data.player_x;
    s_player_y = data.player_y;
    s_player_direction = data.player_direction;
    s_player_sprite = data.player_sprite;
    s_player_step = data.player_step;
  } else {
    s_player_x = PLAYER_ORIGIN_X;
    s_player_y = PLAYER_ORIGIN_Y;
    new_player_sprites(graphics);
  }
  s_atomic_player_step = s_player_step;
  s_atomic_player_x = s_player_x;
  s_atomic_player_y = s_player_y;
  load_game(graphics);
  s_game_started = true;
}

static void load_blocks_in_direction(GBC_Graphics *graphics, PlayerDirection direction) {
  uint8_t bg_root_x = (GBC_Graphics_bg_get_scroll_x(graphics) >> 3);
  uint8_t bg_root_y = (GBC_Graphics_bg_get_scroll_y(graphics) >> 3);
  uint16_t tile_root_x = (s_player_x >> 3) - 8;
  uint16_t tile_root_y = (s_player_y >> 3) - 8;
  uint16_t num_x_tiles = 0, num_y_tiles = 0;
  switch(direction) {
    case D_UP:
      tile_root_y -= 2;
      bg_root_y -= 2;
      num_x_tiles = 18;
      num_y_tiles = 2;
      break;
    case D_LEFT:
      tile_root_x -= 2;
      bg_root_x -= 2;
      num_x_tiles = 2;
      num_y_tiles = 18;
      break;
    case D_DOWN:
      tile_root_y += 18;
      bg_root_y += 18;
      num_x_tiles = 18;
      num_y_tiles = 2;
      break;
    case D_RIGHT:
      tile_root_x += 18;
      bg_root_x += 18;
      num_x_tiles = 2;
      num_y_tiles = 18;
      break;
  }
  load_tiles(graphics, bg_root_x & 31, bg_root_y & 31, tile_root_x, tile_root_y, num_x_tiles, num_y_tiles);
}

#if defined(PBL_COLOR)
static void lerp_player_palette_to_color(GBC_Graphics *graphics, uint8_t end, uint8_t index) {
  uint8_t palette_holder[4];
  uint8_t end_palette[4] = {end, end, end, end};
  lerp_palette(s_player_palette, end_palette, index, palette_holder);
  GBC_Graphics_set_sprite_palette_array(graphics, 0, palette_holder);
}

static void lerp_bg_palettes_to_color(GBC_Graphics *graphics, uint8_t end, uint8_t index) {
  uint8_t palette_holder[4];
  uint8_t end_palette[4] = {end, end, end, end};
  for (uint8_t i = 0; i < 8; i++) {
    lerp_palette(&s_cur_bg_palettes[i*PALETTE_SIZE], end_palette, index, palette_holder);
    GBC_Graphics_set_bg_palette_array(graphics, i, palette_holder);
  }
}

#else

static void set_player_palette_to_color(GBC_Graphics *graphics, uint8_t color) {
  uint8_t palette[4] = {color, color, color, color};
  GBC_Graphics_set_sprite_palette_array(graphics, 0, palette);
}

static void set_bg_palettes_to_color(GBC_Graphics *graphics, uint8_t color) {
  uint8_t palette[4] = {color, color, color, color};
  for (uint8_t i = 0; i < 8; i++) {
    GBC_Graphics_set_bg_palette_array(graphics, i, palette);
  }
}
#endif

static void turn_player() {
  ResHandle path_handle = resource_get_handle(RESOURCE_ID_DATA_PATH);
  size_t res_size = resource_size(path_handle);
  uint16_t data_offset = s_player_step / 4;
  uint8_t buffer[1];
  resource_load_byte_range(path_handle, data_offset, buffer, 1);
  s_player_direction = (buffer[0] >> (2 * (s_player_step % 4))) & 0b11;
  s_player_step = (s_player_step + 1) % (res_size * 4);
}


void Pokemon_start_animation(GBC_Graphics *graphics) {
  if (s_player_mode == P_STAND) { 
    turn_player();
    s_step_frame = 0;
  }
}

static void play(GBC_Graphics *graphics) {
  s_anim_frame = (s_anim_frame + 1) % 8;
  if (s_anim_frame == 0) {
    animate_tiles(graphics, TILE_BANK_WORLD, s_route_num);
  }
  if (s_player_mode == P_STAND) {
    s_target_x = s_player_x + direction_to_point(s_player_direction).x * (TILE_WIDTH * 2);
    s_target_y = s_player_y + direction_to_point(s_player_direction).y * (TILE_HEIGHT * 2);
    
    s_player_mode = P_WALK;
    s_walk_frame = 0;
    if (s_player_sprite != 21) {
      s_flip_walk = !s_flip_walk;
    }
    GBC_Graphics_oam_set_sprite_priority(graphics, 4, false);
    GBC_Graphics_oam_set_sprite_priority(graphics, 5, false);

    
    PokemonSquareInfo current_block_type = get_block_type(s_world_map, s_player_x, s_player_y);
    PokemonSquareInfo target_block_type = get_block_type(s_world_map, s_target_x+8, s_target_y);
    if (target_block_type == OBJECT) {
      int object_num = check_for_object(s_target_x+8, s_target_y);
      if (object_num != -1) {
        PokemonObjectTypes object_type = objects[object_num][3];
        const int16_t *data = &objects[object_num][4];
        switch (object_type) {
          case PO_NONE:
            break;
          case PO_WARP:{
            s_warp_route = data[0];
            s_warp_x = data[1];
            s_warp_y = data[2];
            if (s_player_direction == D_UP) {
              s_player_mode = P_WARP_WALK;
              load_blocks_in_direction(graphics, s_player_direction);
            } else {
              s_player_mode = P_WARP;
            }
            s_can_move = true;
          } break;
          default:
            s_can_move = false;
            break;
        }
      } else {
        s_can_move = false;
      }
    } else {
      if ((target_block_type == CLIFF_S && s_player_direction == D_DOWN)
          || (target_block_type == CLIFF_W && s_player_direction == D_LEFT)
          || (target_block_type == CLIFF_E && s_player_direction == D_RIGHT)) {
        s_target_x += direction_to_point(s_player_direction).x * (TILE_WIDTH * 2);
        s_target_y += direction_to_point(s_player_direction).y * (TILE_HEIGHT * 2);
        s_player_mode = P_JUMP;
        s_can_move = true;
      } else {
        s_can_move = (target_block_type == WALK || target_block_type == GRASS
                      || (target_block_type == CLIFF_N && s_player_direction != D_DOWN));
        s_can_move &= !(current_block_type == CLIFF_N && s_player_direction == D_UP);
      }
      if (!s_can_move) {
        s_target_x = s_player_x;
        s_target_y = s_player_y;
      } else {
        load_blocks_in_direction(graphics, s_player_direction);
      #if defined(PBL_BW)
        GBC_Graphics_oam_hide_sprite(graphics, 0);
        GBC_Graphics_oam_hide_sprite(graphics, 1);
      #endif
      }
    }
  }
  
  switch(s_player_mode) {
    case P_STAND:
      break;
    case P_WALK:
      if (s_can_move) {
        s_player_x += direction_to_point(s_player_direction).x * 2;
        s_player_y += direction_to_point(s_player_direction).y * 2;
        GBC_Graphics_bg_move(graphics, direction_to_point(s_player_direction).x * 2, direction_to_point(s_player_direction).y * 2);
        render_items(graphics);
      }
      if (get_block_type(s_world_map, s_target_x+8, s_target_y) == GRASS) { // grass
        switch(s_walk_frame) {
          case 2:
            GBC_Graphics_oam_set_sprite_pos(graphics, 0, s_player_sprite_x, s_player_sprite_y + 4);
            GBC_Graphics_oam_set_sprite_pos(graphics, 1, s_player_sprite_x + 8, s_player_sprite_y + 4);
            break;
          case 5:
            GBC_Graphics_oam_set_sprite_pos(graphics, 0, s_player_sprite_x - 1, s_player_sprite_y + 5);
            GBC_Graphics_oam_set_sprite_pos(graphics, 1, s_player_sprite_x + 9, s_player_sprite_y + 5);
            break;
          case 7:
          #if defined(PBL_COLOR)
            GBC_Graphics_oam_hide_sprite(graphics, 0);
            GBC_Graphics_oam_hide_sprite(graphics, 1);
          #else
            GBC_Graphics_oam_set_sprite_pos(graphics, 0, s_player_sprite_x, s_player_sprite_y + 4);
            GBC_Graphics_oam_set_sprite_pos(graphics, 1, s_player_sprite_x + 8, s_player_sprite_y + 4);
          #endif
            break;
          default:
            break;
        }
      }
      switch(s_walk_frame) {
        case 7:
          s_player_mode = P_STAND;
        #if defined(PBL_COLOR)
          if(get_block_type(s_world_map, s_player_x+8, s_player_y) == GRASS) {
            GBC_Graphics_oam_set_sprite_priority(graphics, 4, true);
            GBC_Graphics_oam_set_sprite_priority(graphics, 5, true);
          }
        #endif
          break;
        case 0:
        case 6:
          set_player_sprites(graphics, false,  s_player_direction == D_RIGHT);
          break;
        case 2:
          set_player_sprites(graphics, true,  s_player_direction == D_RIGHT 
                             || ((s_player_direction == D_DOWN || s_player_direction == D_UP) && s_flip_walk));
          break;
        default:
          break;
      }
      s_walk_frame++;
      break;
    case P_JUMP:
      if (s_can_move) {
        s_player_x += direction_to_point(s_player_direction).x * 2;
        s_player_y += direction_to_point(s_player_direction).y * 2;
        GBC_Graphics_bg_move(graphics, direction_to_point(s_player_direction).x * 2, direction_to_point(s_player_direction).y * 2);
        render_items(graphics);
      }
      switch(s_walk_frame) {
        case 15:
          s_player_mode = P_STAND;
        #if defined(PBL_COLOR)
          if(get_block_type(s_world_map, s_player_x+8, s_player_y) == GRASS) {
            GBC_Graphics_oam_set_sprite_priority(graphics, 4, true);
            GBC_Graphics_oam_set_sprite_priority(graphics, 5, true);
          }
        #endif
          GBC_Graphics_oam_hide_sprite(graphics, 6);
          GBC_Graphics_oam_hide_sprite(graphics, 7);
          break;
        case 0:
          GBC_Graphics_oam_set_sprite_pos(graphics, 6, s_player_sprite_x, s_player_sprite_y + 8);
          GBC_Graphics_oam_set_sprite_pos(graphics, 7, s_player_sprite_x + 8, s_player_sprite_y + 8);
        case 6:
        case 8:
        case 14:
          set_player_sprites(graphics, false,  s_player_direction == D_RIGHT);
          break;
        case 10:
          set_player_sprites(graphics, true,  s_player_direction == D_RIGHT 
                             || ((s_player_direction == D_DOWN || s_player_direction == D_UP) && s_flip_walk));
          break;
        case 7:
          load_blocks_in_direction(graphics, s_player_direction);
          break;
        default:
          break;
      }
      move_player_sprites(graphics, 0, jump_offsets[s_walk_frame]);
      s_walk_frame++;
      break;
    case P_WARP_WALK:
      if (s_can_move) {
        s_player_x += direction_to_point(s_player_direction).x * 2;
        s_player_y += direction_to_point(s_player_direction).y * 2;
        GBC_Graphics_bg_move(graphics, direction_to_point(s_player_direction).x * 2, direction_to_point(s_player_direction).y * 2);
        render_items(graphics);
      }
      switch(s_walk_frame) {
        case 7:
          if (s_player_direction == D_DOWN) {
            s_player_mode = P_STAND;
          } else {
            s_player_mode = P_WARP;
            s_walk_frame = 0;
          }
          break;
        case 0:
        case 6:
          set_player_sprites(graphics, false,  s_player_direction == D_RIGHT);
          break;
        case 2:
          set_player_sprites(graphics, true,  s_player_direction == D_RIGHT 
                             || ((s_player_direction == D_DOWN || s_player_direction == D_UP) && s_flip_walk));
          break;
        default:
          break;
      }
      if (s_player_mode == P_WARP_WALK) {
        s_walk_frame++;
      }
      break;
    case P_WARP:
      if (s_walk_frame < 5) {
        if (s_walk_frame == 0) {
          GBC_Graphics_copy_all_bg_palettes(graphics, s_cur_bg_palettes);
        }
      #if defined(PBL_COLOR)
        lerp_bg_palettes_to_color(graphics, 0b11111111, s_walk_frame);
        lerp_player_palette_to_color(graphics, 0b11111111, s_walk_frame);
      #else
        if (s_walk_frame == 3) {
          set_bg_palettes_to_color(graphics, 1);      
          set_player_palette_to_color(graphics, 1);
        }
      #endif
      } else if (s_walk_frame == 5) {  
        s_route_num = s_warp_route;
        s_player_x = s_warp_x;
        s_player_y = s_warp_y;
        s_target_x = s_warp_x;
        s_target_y = s_warp_y;
        load_resources(graphics);
        load_blocks_in_direction(graphics, s_player_direction);
        GBC_Graphics_copy_all_bg_palettes(graphics, s_cur_bg_palettes);
      #if defined(PBL_BW)
        set_bg_palettes_to_color(graphics, 1);
        for (uint8_t i = 0; i < 8; i++) {
          GBC_Graphics_set_bg_palette_array(graphics, i, bw_palette);
        }
      #else 
        lerp_bg_palettes_to_color(graphics, 0b11111111, 4);
        lerp_player_palette_to_color(graphics, 0b11111111, 4);
      #endif
      } else {
      #if defined(PBL_COLOR)
        lerp_bg_palettes_to_color(graphics, 0b11111111, 10 - s_walk_frame);
        lerp_player_palette_to_color(graphics, 0b11111111, 10 - s_walk_frame);
      #endif
      }
      s_walk_frame++;
      if (s_walk_frame == 11) {
        if (s_player_direction == D_DOWN) {
          s_player_mode = P_WARP_WALK;
          s_walk_frame = 0;
        } else {
          s_player_mode = P_STAND;
        }
      }
      break;
    default:
      break;
  }
}

static void save() {
  PokemonSaveData data = (PokemonSaveData) {
    .route_num = s_route_num,
    .player_x = s_atomic_player_x,
    .player_y = s_atomic_player_y,
    .player_direction = s_player_direction,
    .player_sprite = s_player_sprite,
    .last_save = time(NULL),
    .player_step = s_atomic_player_step,
  };
  persist_write_data(SAVE_KEY, &data, sizeof(PokemonSaveData));
}

bool Pokemon_step(GBC_Graphics *graphics) {
  if (s_step_frame == 0) {
    play(graphics);
  }
  s_step_frame = (s_step_frame + 1) % FRAME_SKIP;
  if (s_player_mode == P_STAND) {
    s_atomic_player_step = s_player_step;
    s_atomic_player_x = s_player_x;
    s_atomic_player_y = s_player_y;
  }
  return s_player_mode != P_STAND;
}

void Pokemon_handle_tap(GBC_Graphics *graphics) {
  new_player_sprites(graphics);
  set_player_sprites(graphics, false, s_player_direction == D_RIGHT);
  GBC_Graphics_render(graphics);
}

void Pokemon_deinitialize(GBC_Graphics *graphics) {
  if (s_game_started) {
    save();
  }
  if (s_world_map != NULL) {
    free(s_world_map);
    s_world_map = NULL;
  }
}