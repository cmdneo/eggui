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

class ColorBlock : public InteractiveWidget
{
public:
	ColorBlock(int width, int height, Color col)
		: InteractiveWidget(width, height)
		, color(col)
	{
	}

	void draw() override
	{
		Rectangle rect{};
		rect.x = get_position().x;
		rect.y = get_position().y;
		rect.width = get_size().x;
		rect.height = get_size().y;

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
			saved_pos = get_position();
			break;
		case EventType::MouseReleased:
			set_position(saved_pos);
			break;

		case EventType::MouseDrag:
			set_position(get_position() + ev.delta);
			break;

		case EventType::MouseClick:
			// Ignore click if was dragged to a different position
			if (saved_pos != get_position())
				return false;
			{
				auto pos = get_position();
				println(
					"Clicked ID[{}] {} {}", uintptr_t(this) % 1024, pos.x, pos.y
				);
			}
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
		Anchor::BottomRight, make_unique<ColorBlock>(100, 100, PINK)
	);
	pane_right->add_widget(
		Anchor::BottomLeft, make_unique<ColorBlock>(100, 100, RED)
	);
	pane_right->add_widget(
		Anchor::TopLeft, make_unique<ColorBlock>(50, 100, GRAY)
	);

	root->add_widget(Anchor::TopLeft, std::move(pane_left));
	root->add_widget(Anchor::BottomRight, std::move(pane_right));

	auto window = Window(std::move(root));
	window.set_title("Event demo...!");

	TraceLog(LOG_INFO, "GUI debug enabled.");
	window.set_debug(true);
	window.main_loop(640, 480);

	return 0;
}
