#ifndef THEME_H_INCLUDED
#define THEME_H_INCLUDED

#include "graphics.hxx"

namespace eggui
{
constexpr float SCROLL_FACTOR = 24.;

constexpr float ELEMENT_ROUNDNESS = 0.2;
constexpr int SCROLL_BAR_WIDTH = 10;
constexpr int MIN_SLIDER_LENGTH = 30;
constexpr int TEXT_PADDING = 4;
constexpr int CURSOR_WIDTH = 2;

// Dark theme color palette
constexpr RGBA BACKGROUND_COLOR(31, 32, 33);
constexpr RGBA SCROLL_BAR_COLOR(44, 45, 46);
constexpr RGBA BUTTON_COLOR(61, 62, 63);
constexpr RGBA BUTTON_HOVER_COLOR(71, 72, 73);
constexpr RGBA BUTTON_CLICK_COLOR(85, 86, 85);

constexpr RGBA BORDER_COLOR(192, 192, 192);
constexpr RGBA TEXT_BG_COLOR(51, 52, 53);
constexpr RGBA TEXT_COLOR(230, 230, 230);

constexpr RGBA CURSOR_COLOR(216, 0, 101); // (221, 22, 115)

constexpr RGBA GAP_COLOR(255, 109, 192);
constexpr RGBA GAP_FILL_COLOR(255, 109, 192, 64);

//  constexpr Color PRIMARY_COLOR = {255, 46, 99, 255};
//  constexpr Color SECONDARY_COLOR = {8, 192, 196, 255};

// Debug colors
constexpr RGBA DEBUG_BORDER_COLOR(116, 238, 21);
} // namespace eggui

#endif
