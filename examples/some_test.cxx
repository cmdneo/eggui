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
#include "eggui.hxx"

using namespace eggui;

using std::make_unique;

int main()
{

	auto root = make_unique<HorizontalContainer>();
	auto pane_left = make_unique<VerticalContainer>();
	auto pane_right = make_unique<VerticalContainer>();

	pane_left->add_widget(
		Anchor::BottomRight, make_unique<Button>(100, 60, "Main")
	);
	pane_right->add_widget(
		Anchor::BottomLeft, make_unique<Button>(200, 100, "BigX")
	);
	pane_right->add_widget(
		Anchor::BottomLeft, make_unique<Button>(200, 100, "BigY")
	);

	root->add_widget(Anchor::TopLeft, std::move(pane_left));
	root->add_widget(Anchor::BottomRight, std::move(pane_right));

	auto window = Window(std::move(root));
	window.set_title("Event demo...!");

	window.main_loop(640, 480);

	return 0;
}
