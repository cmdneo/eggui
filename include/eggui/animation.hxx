#ifndef ANIMATION_HXX_INCLUDED
#define ANIMATION_HXX_INCLUDED

#include <cassert>
#include <functional>

/// @brief Animation object, can be attached to a widget by requesting the window.
class Animation
{
public:
	Animation(
		int duration_, int delay_, bool loop,
		std::function<bool(int tick)> next_frame_cb
	)
		: delay(delay_)
		, duration(duration_)
		, is_looping(loop)
		, next_frame_callback(next_frame_cb)
	{
	}

	bool update()
	{
		assert(tick < duration);

		elapsed++;
		if (elapsed <= delay)
			return false;

		auto changed = next_frame_callback(tick);
		tick++;

		if (tick == duration && is_looping)
			tick = 0;

		return changed;
	}

	bool has_ended() const { return tick == duration; }

private:
	// Time in ticks, 1 / TICKS_PER_SECOND = UPDATE_DELTA_TIME
	// is the duration of a tick.
	// Start animation after this many ticks.
	int delay;
	// The length of an animation.
	int duration;
	// Ticks elapsed since the animation was initiated.
	int elapsed;
	// Current tick of the animation ranges from: 0 to duration-1.
	int tick;
	// Enable looping.
	bool is_looping;
	// Function object to call for next frame,
	// if it returns true then the widget should be re-drawn.
	std::function<bool(int tick)> next_frame_callback;
};

#endif
