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

class ColorBlock : public Widget
{
public:
	ColorBlock(int width, int height, Color col)
		: Widget(width, height)
		, color(col)
	{
	}

	void draw() override
	{
		Rectangle rect{};
		rect.x = position.x;
		rect.y = position.y;
		rect.width = box_size.x;
		rect.height = box_size.y;

		DrawRectangleRec(rect, color);
		if (is_hovering)
			DrawRectangleLinesEx(rect, 5, BLUE);
	}

	bool notify(Event ev) override
	{
		switch (ev.type) {
		case EventType::MouseIn:
			is_hovering = true;
			break;
		case EventType::MouseOut:
			is_hovering = false;
			break;

		case EventType::MousePressed:
			saved_pos = position;
			break;
		case EventType::MouseReleased:
			position = saved_pos;
			break;

		case EventType::MouseDrag:
			position += ev.delta;
			break;

		case EventType::MouseClick:
			// Ignore click if was dragged to a different position
			if (saved_pos != position)
				return false;
			println("Clicked ID[{}]", uintptr_t(this) % 1024);
			break;

		default:
			return false;
		}

		return true;
	}

private:
	Color color{};
	Point saved_pos{};
	bool is_hovering = false;
};

int main()
{

	auto root = make_unique<HorizontalContainer>();
	auto pane_left = make_unique<VerticalContainer>();
	auto pane_right = make_unique<VerticalContainer>();

	pane_left->add_widget(
		Anchor::TopRight, make_unique<ColorBlock>(40, 100, PINK)
	);
	pane_right->add_widget(
		Anchor::BottomRight, make_unique<ColorBlock>(80, 40, RED)
	);
	pane_right->add_widget(
		Anchor::TopLeft, make_unique<ColorBlock>(60, 40, GRAY)
	);
	pane_right->add_widget(
		Anchor::BottomLeft, make_unique<ColorBlock>(60, 40, PURPLE)
	);

	pane_left->set_size(200, 200);
	pane_right->set_size(150, 300);

	root->add_widget(Anchor::TopLeft, std::move(pane_left));
	root->add_widget(Anchor::BottomRight, std::move(pane_right));

	root->set_size(GetScreenWidth(), GetScreenHeight());

	auto window = Window(std::move(root));
	window.set_title("Event demo...!");

	TraceLog(LOG_INFO, "GUI debug enabled.");
	window.set_debug(true);
	window.main_loop(640, 480);

	return 0;
}
