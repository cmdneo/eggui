#ifndef THEME_H_INCLUDED
#define THEME_H_INCLUDED

#include "raylib/raylib.h"

// Font, loaded by window.cxx: main_loop
extern Font EG_MONO_FONT;
static constexpr int EG_FONT_SIZE = 32;

// Dark theme color palette
static constexpr Color BACKGROUND_COLOR = {31, 32, 33, 255};
static constexpr Color BUTTON_COLOR = {61, 62, 63, 255};
static constexpr Color BUTTON_HOVER_COLOR = {71, 72, 73, 255};
static constexpr Color BUTTON_CLICK_COLOR = {85, 86, 85, 255};

static constexpr Color BORDER_COLOR = {192, 192, 192, 255};
static constexpr Color TEXT_COLOR = {230, 230, 230, 255};

// static constexpr Color PRIMARY_COLOR = {255, 46, 99, 255};
// static constexpr Color SECONDARY_COLOR = {8, 192, 196, 255};

// Debug colors
static constexpr Color DEBUG_BORDER_COLOR = {116, 238, 21, 255};

#endif
