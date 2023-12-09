#include <cassert>
#include <memory>
#include <vector>
#include <functional>
#include <utility>
#include <iostream>
#include <format>

#define println(...) (std::cout << std::format(__VA_ARGS__) << '\n')

#include "raylib/raylib.h"

#include "event.hxx"
#include "scrollable.hxx"
#include "eggui.hxx"

using namespace eggui;

using std::make_unique;

int main()
{

	auto grid = make_unique<Grid>();
	grid->set_row_gap(10);
	grid->set_col_gap(10);
	grid->add_widget(make_unique<Button>(400, 200, "DBL-DECKER"), 0, 0, 1, 2);
	auto b = grid->add_widget(make_unique<Button>(200, 70, "SIN-U"), 2, 0);

	grid->add_widget(make_unique<Button>(200, 70, "SIN-L"), 2, 2);

	auto sbar = make_unique<ScrollBar>(20, 400, ScrollBar::Axis::Y);
	sbar->set_on_scroll([](float) {});
	grid->add_widget_beside(std::move(sbar), b, Direction::Left, 1, 4);

	grid->add_widget_beside(
		make_unique<Button>(400, 100, "XPANDER"), b, Direction::BottomRight
	);

	auto window = Window(std::move(grid));
	window.set_title("Event demo...!");

	window.main_loop();

	return 0;
}
