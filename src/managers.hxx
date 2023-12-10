#ifndef MANAGERS_HXX_INCLUDED
#define MANAGERS_HXX_INCLUDED

#include <vector>
#include <utility>

#include "raylib/raylib.h"
#include "point.hxx"

/// @brief Manages textures, is a singleton!
class TextureManager
{
public:
	/// Tracks if the OpenGL context is available, if available then the
	// texture can be loaded requested, otherwise defer it and can be loaded
	// using `load_textures` when the OpenGL context becomes available.
	bool does_gl_context_exist = false;

	static TextureManager &instance();

	int create_texture(eggui::Point size);
	void destroy_texture(int id);
	void load_textures();
	void unload_textures();

	/// @brief Start textured drawing mode for the texture provided.
	/// @param texture_id Texture id
	void push_texture(int texture_id);
	/// @brief End textured drawing mode for the last texture pushed.
	/// @return The texture_id popped.
	int pop_texture();

	RenderTexture &get(int id) { return textures[id].first; }

private:
	TextureManager();

	/// @brief Gets a free slot for the new texture, if none exists then
	///        allocates a new free slot.
	/// @return Texture ID
	int get_free_slot();

	enum class SlotState { Free, InUse, NeedsInit };

	/// Textures for every widget created, indexed by texture_id.
	/// The SlotState stores info about the state of the slot.
	std::vector<std::pair<RenderTexture, SlotState>> textures;
	std::vector<int> free_list;

	// Since only one texture can be active at a time we maintain a list
	// of textures that are currently used by pens.
	// We only keep the last pushed texture active.
	std::vector<int> active_textures;
};

// TODO complete this
class FontManager
{
public:
	static FontManager &instance();

	int load_font(
		const char *file_type, unsigned char *data, int size, int font_size
	);
	void unload_fonts();

private:
	FontManager() { fonts.reserve(8); }

	std::vector<Font> fonts;
};

#endif