#include "main_window.h"
#include "pebble-gbc-graphics/pebble-gbc-graphics.h"
#include "pokemon/pokemon.h"

static Window *s_window;
static GBC_Graphics *s_graphics;
static AppTimer *s_frame_timer;
static Layer *s_foreground_layer;
static uint8_t s_min, s_hour;
static int32_t s_minute_angle;
static int32_t s_hour_angle;
static GRect s_screen_rect, s_adjusted_screen_rect;
static GRect s_hour_rect;
static GRect s_inner_rect;
static GRect s_inner_rect_white;
static GPoint s_hour_endpoint, s_min_endpoint;

ClaySettings settings;

/* Game loading handlers */
static void load_game() {
  Pokemon_initialize(s_graphics);
}

static void unload_game() {
  Pokemon_deinitialize(s_graphics);
}

/* Input handlers*/
static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  if (settings.ShakeSprite) {
    Pokemon_handle_tap(s_graphics);
  }
}

/* timer callback */
static void frame_timer_handle(void* context) {
  if (Pokemon_step(s_graphics)) {
    s_frame_timer = app_timer_register(FRAME_DURATION, frame_timer_handle, NULL); 
  } else {
    s_frame_timer = NULL;
  }

  GBC_Graphics_render(s_graphics);
}

/* time callback */
static void time_handler(struct tm *tick_time, TimeUnits units_changed) {
  Pokemon_start_animation(s_graphics);
  s_frame_timer = app_timer_register(FRAME_DURATION, frame_timer_handle, NULL);
  layer_mark_dirty(s_foreground_layer);
  
  s_hour = tick_time->tm_hour % 12;
  s_min = tick_time->tm_min;
}

static void will_focus_handler(bool in_focus) {
  if (!in_focus) {
    // If a notification pops up while the timer is firing
    // very rapidly, it will crash the entire watch :)
    // Stopping the timer when a notification appears will
    // prevent this while also pausing the gameplay
    if (s_frame_timer != NULL) {
      app_timer_cancel(s_frame_timer);
    }
  } else {
    if (s_frame_timer != NULL) {
      s_frame_timer = app_timer_register(FRAME_DURATION, frame_timer_handle, NULL);
    }
  }
}

static void foreground_update_proc(Layer *layer, GContext *ctx) {
  // determine what direction the hands will go in
  s_minute_angle = TRIG_MAX_ANGLE * s_min / 60;
  s_hour_angle = TRIG_MAX_ANGLE * s_hour / 12 + TRIG_MAX_ANGLE / 12 * s_min / 60;

  // determine where they should end
  s_hour_endpoint = gpoint_from_polar(s_hour_rect, GOvalScaleModeFitCircle, s_hour_angle);
  s_min_endpoint = gpoint_from_polar(s_screen_rect, GOvalScaleModeFitCircle, s_minute_angle);

  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_antialiased(ctx, false);

  // Draw the top and bottom halves of the pokeball frame
  GRect radial_bounds = GRect(bounds.origin.x - 30, bounds.origin.y - 29, bounds.size.w + 61, bounds.size.h + 59);
  graphics_context_set_fill_color(ctx, settings.BackgroundTopColor);
  graphics_fill_radial(ctx, radial_bounds, GOvalScaleModeFillCircle, PBL_IF_ROUND_ELSE(49, 43), 
                        DEG_TO_TRIGANGLE(-90), DEG_TO_TRIGANGLE(90));
  graphics_context_set_fill_color(ctx, settings.BackgroundBottomcolor);
  graphics_fill_radial(ctx, radial_bounds, GOvalScaleModeFillCircle, PBL_IF_ROUND_ELSE(49, 43), 
                        DEG_TO_TRIGANGLE(90), DEG_TO_TRIGANGLE(270));

  GPoint center = GPoint(bounds.origin.x + bounds.size.w / 2, bounds.origin.y + bounds.size.h / 2);

  // On round, draw the black bar that connects the halves (not visible on rectangular watches)
#if defined(PBL_ROUND)
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 4);
  graphics_draw_line(ctx, GPoint(0, center.y), GPoint(18, center.y));
  graphics_draw_line(ctx, GPoint(180, center.y), GPoint(180-18, center.y));
#endif

  // Draw the outer circles
  if (settings.BatteryBar) {
    BatteryChargeState battery_state = battery_state_service_peek();
    GColor battery_color;
    if (battery_state.is_charging) {
      battery_color = PBL_IF_COLOR_ELSE(GColorBlueMoon, GColorLightGray);
    } else if (battery_state.charge_percent <= 10) {
      battery_color = PBL_IF_COLOR_ELSE(GColorRed, GColorLightGray);
    } else if (battery_state.charge_percent <= 20) {
      battery_color = PBL_IF_COLOR_ELSE(GColorChromeYellow, GColorLightGray);
    } else {
      battery_color = PBL_IF_COLOR_ELSE(GColorIslamicGreen, GColorLightGray);
    }

    int length = (battery_state.charge_percent * TRIG_MAX_ANGLE) / 100;
    GRect battery_bounds = SCREEN_BOUNDS_SQUARE;
    battery_bounds.origin.x -= 1;
    battery_bounds.origin.y -= 1;
    battery_bounds.size.w += 3;
    battery_bounds.size.h += 3;
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_radial(ctx, battery_bounds, GOvalScaleModeFitCircle, 3, 
                          0, TRIG_MAX_ANGLE);
    graphics_context_set_fill_color(ctx, battery_color);
    graphics_fill_radial(ctx, battery_bounds, GOvalScaleModeFitCircle, 3, 
                          TRIG_MAX_ANGLE-length, TRIG_MAX_ANGLE);
  } else {
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_stroke_width(ctx, 2);
    graphics_draw_circle(ctx, center, 72);
  }
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_circle(ctx, center, 70);
  graphics_draw_circle(ctx, center, 74);

  // Draw circles for the hours
  if (settings.HourTickmarks) {
    GPoint point;
    GRect rect;
    for (uint8_t i = 0; i < 12; i++) {
      graphics_context_set_fill_color(ctx, GColorWhite);
      point = gpoint_from_polar(s_adjusted_screen_rect, GOvalScaleModeFillCircle, DEG_TO_TRIGANGLE(i*30));
      graphics_fill_circle(ctx, point, i % 3 == 0 ? 6 : 4);
      
      // The cardinal hours get pokeballs!
      if (i % 3 == 0) {
        graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorRed, GColorLightGray));
        rect = grect_centered_from_polar(s_adjusted_screen_rect, GOvalScaleModeFillCircle, DEG_TO_TRIGANGLE(i*30), GSize(13, 13));
        rect.origin.y += 1;
        graphics_fill_radial(ctx, rect, GOvalScaleModeFillCircle, 6, DEG_TO_TRIGANGLE(-90), DEG_TO_TRIGANGLE(90));

        graphics_context_set_stroke_width(ctx, 1);
        graphics_context_set_stroke_color(ctx, GColorBlack);
        graphics_draw_line(ctx, GPoint(point.x-5, point.y), GPoint(point.x+5, point.y));
        graphics_context_set_stroke_color(ctx, GColorWhite);
        graphics_draw_line(ctx, GPoint(point.x-5, point.y+1), GPoint(point.x+5, point.y+1));
        
        graphics_context_set_fill_color(ctx, GColorWhite);
        graphics_fill_circle(ctx, point, 2);
        graphics_context_set_stroke_color(ctx, GColorBlack);
        graphics_draw_circle(ctx, point, 2);
      }

      graphics_context_set_stroke_width(ctx, 1);
      graphics_context_set_stroke_color(ctx, GColorBlack);
      graphics_draw_circle(ctx, point, i % 3 == 0 ? 6 : 4);
    }
  }

  // Draw the white outline for the minute hand
  GPoint min_startpoint;
  graphics_context_set_stroke_width(ctx, 4);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  min_startpoint = gpoint_from_polar(s_inner_rect_white, GOvalScaleModeFitCircle, s_minute_angle + DEG_TO_TRIGANGLE(-48));
  graphics_draw_line(ctx, min_startpoint, s_min_endpoint);
  min_startpoint = gpoint_from_polar(s_inner_rect_white, GOvalScaleModeFitCircle, s_minute_angle + DEG_TO_TRIGANGLE(48));
  graphics_draw_line(ctx, min_startpoint, s_min_endpoint);
  // Then draw the actual minute hand
  graphics_context_set_stroke_color(ctx, settings.MinuteColor);
  graphics_context_set_stroke_width(ctx, 2);
  for (int8_t i = -45; i < 46; i+=9) {
    min_startpoint = gpoint_from_polar(s_inner_rect, GOvalScaleModeFitCircle, s_minute_angle + DEG_TO_TRIGANGLE(i));
    graphics_draw_line(ctx, min_startpoint, s_min_endpoint);
  }
  
  // Draw the white outline for the hour hand
  GPoint hour_startpoint;
  graphics_context_set_stroke_width(ctx, 4);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  hour_startpoint = gpoint_from_polar(s_inner_rect_white, GOvalScaleModeFitCircle, s_hour_angle + DEG_TO_TRIGANGLE(-48));
  graphics_draw_line(ctx, hour_startpoint, s_hour_endpoint);
  hour_startpoint = gpoint_from_polar(s_inner_rect_white, GOvalScaleModeFitCircle, s_hour_angle + DEG_TO_TRIGANGLE(48));
  graphics_draw_line(ctx, hour_startpoint, s_hour_endpoint);
  // And draw the actual hour hand
  graphics_context_set_stroke_color(ctx, settings.HourColor);
  graphics_context_set_stroke_width(ctx, 2);
  for (int8_t i = -45; i < 46; i+=9) {
    hour_startpoint = gpoint_from_polar(s_inner_rect, GOvalScaleModeFitCircle, s_hour_angle + DEG_TO_TRIGANGLE(i));
    graphics_draw_line(ctx, hour_startpoint, s_hour_endpoint);
  }

  // Draw the inner circles 
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_circle(ctx, center, 17);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_circle(ctx, center, 15);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_circle(ctx, center, 19);
}


static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_set_background_color(window, GColorBlack);

  s_graphics = GBC_Graphics_ctor(window, 1);
  load_game();
  GBC_Graphics_render(s_graphics);

  s_foreground_layer = layer_create(bounds);
  layer_add_child(window_get_root_layer(window), s_foreground_layer);
  layer_set_update_proc(s_foreground_layer, foreground_update_proc);
  
  layer_mark_dirty(s_foreground_layer);
}

static void window_unload(Window *window) {
  GBC_Graphics_destroy(s_graphics);

  unload_game();

  layer_destroy(s_foreground_layer);

  window_destroy(s_window);
}

static void default_settings() {
  settings.BackgroundBottomcolor = GColorWhite;
  settings.BackgroundTopColor = PBL_IF_COLOR_ELSE(GColorRed, GColorLightGray);
  settings.HourColor = PBL_IF_COLOR_ELSE(GColorRed, GColorBlack);
  settings.MinuteColor = GColorBlack;
  settings.Analog = true;
  settings.HourTickmarks = true;
  settings.ShakeSprite = true;
  settings.BatteryBar = false;
}

static void load_settings() {
  default_settings();

  persist_read_data(CLAY_SAVE_KEY, &settings, sizeof(settings));
}

static void save_settings() {
  persist_write_data(CLAY_SAVE_KEY, &settings, sizeof(settings));

  layer_mark_dirty(s_foreground_layer);
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  // Read color preferences
  Tuple *bg_top_color_t = dict_find(iter, MESSAGE_KEY_BackgroundTopColor);
  if(bg_top_color_t) {
    settings.BackgroundTopColor = GColorFromHEX(bg_top_color_t->value->int32);
  }

  Tuple *bg_bot_color_t = dict_find(iter, MESSAGE_KEY_BackgroundBottomColor);
  if(bg_bot_color_t) {
    settings.BackgroundBottomcolor = GColorFromHEX(bg_bot_color_t->value->int32);
  }

  Tuple *hour_color_t = dict_find(iter, MESSAGE_KEY_HourColor);
  if(hour_color_t) {
    settings.HourColor = GColorFromHEX(hour_color_t->value->int32);
  }

  Tuple *minute_color_t = dict_find(iter, MESSAGE_KEY_MinuteColor);
  if(minute_color_t) {
    settings.MinuteColor = GColorFromHEX(minute_color_t->value->int32);
  }

  Tuple *time_style_t = dict_find(iter, MESSAGE_KEY_TimeStyle);
  if(time_style_t) {
    char * time_style = time_style_t->value->cstring;
    settings.Analog = strcmp(time_style, "analog") == 0;
  }

  Tuple *hour_tickmarks_t = dict_find(iter, MESSAGE_KEY_HourTickmarks);
  if(hour_tickmarks_t) {
    settings.HourTickmarks = hour_tickmarks_t->value->int32 == 1;
  }

  Tuple *shake_sprite_change_t = dict_find(iter, MESSAGE_KEY_ShakeSpriteChange);
  if(shake_sprite_change_t) {
    settings.ShakeSprite = shake_sprite_change_t->value->int32 == 1;
  }

  Tuple *battery_bar_t = dict_find(iter, MESSAGE_KEY_BatteryBar);
  if(battery_bar_t) {
    settings.BatteryBar = battery_bar_t->value->int32 == 1;
  }

  save_settings();
}

static void battery_callback(BatteryChargeState state) {
  layer_mark_dirty(s_foreground_layer);
}

static void init() {
  // go ahead and calculate these ahead of time, so we don't waste processor time later
  s_screen_rect = SCREEN_BOUNDS_SQUARE;
  s_adjusted_screen_rect = GRect(s_screen_rect.origin.x, s_screen_rect.origin.y, s_screen_rect.size.w+1, s_screen_rect.size.h+1);
  s_hour_rect = GRect(s_screen_rect.origin.x + 22, s_screen_rect.origin.y + 22, 100, 100);
  s_inner_rect = GRect(s_screen_rect.origin.x + 56, s_screen_rect.origin.y + 57, 34, 34);
  s_inner_rect_white = GRect(s_screen_rect.origin.x + 55, s_screen_rect.origin.y + 56, 36, 36);

  // Grab the current time so that there's no loading
  time_t unadjusted_time = time(NULL);
  struct tm *cur_time = localtime(&unadjusted_time);
  s_hour = cur_time->tm_hour % 12;
  s_min = cur_time->tm_min;

  battery_state_service_subscribe(battery_callback);

  // Load any previous Clay settings
  load_settings();

  // Open AppMessage connection
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(256, 256);
}

void main_window_push() {
  if(!s_window) {
    s_window = window_create();
    init();
    tick_timer_service_subscribe(MINUTE_UNIT, time_handler);
    accel_tap_service_subscribe(accel_tap_handler);
    app_focus_service_subscribe(will_focus_handler);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);
}