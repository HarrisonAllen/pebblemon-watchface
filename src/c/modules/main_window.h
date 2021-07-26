#pragma once

#include <pebble.h>

// Very low frame duration, draw as fast as possible. 1ms leads
// to strange behavior, so I alwyas go with 2
#define FRAME_DURATION 2

#define CLAY_SAVE_KEY 2

typedef struct ClaySettings {
    GColor BackgroundTopColor;
    GColor BackgroundBottomcolor;
    GColor HourColor;
    GColor HourOutlineColor;
    GColor MinuteColor;
    GColor MinuteOutlineColor;
    GColor MiddleBarColor;
    bool Analog;
    bool HourTickmarks;
    bool ShakeSprite;
    bool BatteryBar;
} ClaySettings;

void main_window_mark_background_dirty();

void main_window_push();