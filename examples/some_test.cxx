#include <cctype>
#include <cstdio>
#include <array>
#include <memory>
#include <string>
#include <utility>
#include <format>

#include "eggui.hxx"

using std::make_shared;

using eggui::Alignment;
using eggui::Button;
using eggui::Direction;
using eggui::Label;
using eggui::LinearBox;
using eggui::Orientation;
using eggui::PaddedBox;
using eggui::RGBA;
using eggui::ScrollArea;
using eggui::Switch;
using eggui::Toast;
using eggui::Window;

constexpr std::array<const char *, 9> names = {
	"ABC", "DEF", "GHI", "JKL", "MNO", "PQT", "RST", "UVW", "WXY",
};

auto main() -> int
{
	auto linear = make_shared<LinearBox>(Orientation::Vertical);
	linear->set_gap(5);

	int num = 0;
	auto add_btn = make_shared<Button>(180, 60, "Add +1");
	add_btn->set_on_click([&num](Window &w, Button &) {
		auto msg = std::format("Count: {}", ++num);
		Toast(180, 40, 2, msg, Direction::Bottom).show_toast(w);
	});

	auto sub_btn = make_shared<Button>(180, 60, "Add -1");
	sub_btn->set_on_click([&num](Window &w, Button &) {
		auto msg = std::format("Count: {}", --num);
		Toast(180, 40, 2, msg, Direction::Bottom).show_toast(w);
	});

	auto button_box = make_shared<LinearBox>(Orientation::Horizontal);
	button_box->set_gap(20);
	button_box->add_widget_start(add_btn);
	button_box->add_widget_end(sub_btn);
	linear->add_widget_start(button_box);

	for (auto name : names) {
		auto label = make_shared<Label>(140, 40, name, RGBA(255, 255, 255));
		label->set_text_align(Alignment::Center, Alignment::Center);
		auto toggle = make_shared<Switch>(40, 22);
		toggle->set_on_toggle([name](auto &, auto &, bool s) {
			printf("%3s : %s\n", (s ? "ON" : "OFF"), name);
		});

		auto combo = make_shared<LinearBox>(Orientation::Horizontal);
		combo->add_widget_start(label);
		combo->add_widget_end(toggle);
		linear->add_widget_start(combo);
	}

	auto box = make_shared<PaddedBox>(std::move(linear));
	box->set_padding(10, 10, 10, 10);
	auto scroller = make_shared<ScrollArea>(200, 400, std::move(box));
	scroller->set_scroll(false, true);

	auto window = Window(std::move(scroller));
	window.set_title("Event demo...!");
	window.main_loop();

	return 0;
}
