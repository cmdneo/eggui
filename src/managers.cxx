#include <cassert>
#include <memory>
#include <ranges>

#include "raylib/raylib.h"

#include "point.hxx"
#include "managers.hxx"

using std::unique_ptr;

// Texture manager members
//---------------------------------------------------------
TextureManager &TextureManager::instance()
{
	static unique_ptr<TextureManager> ptr(new TextureManager());
	return *ptr;
}

int TextureManager::create_texture(eggui::Point size)
{
	int id = get_free_slot();

	if (does_gl_context_exist) {
		auto tex = LoadRenderTexture(size.x, size.y);
		textures[id] = {tex, SlotState::InUse};
	} else {
		// Store the info needed for initialization in the texture itself.
		Texture tmp = {.width = size.x, .height = size.y};
		textures[id] = {RenderTexture{.texture = tmp}, SlotState::NeedsInit};
	}

	return id;
}

void TextureManager::destroy_texture(int id)
{
	assert(id < static_cast<int>(textures.size()));
	assert(textures[id].second != SlotState::Free);

	if (textures[id].second == SlotState::InUse)
		UnloadRenderTexture(textures[id].first);

	// After we remove a texture we mark its slot free and track it,
	// we never shrink the texture vector as that would mean moving rest of the
	// textures, which would change their ID(which is the index).
	textures[id].second = SlotState::Free;
	free_list.push_back(id);
}

void TextureManager::load_textures()
{
	assert(does_gl_context_exist);

	for (auto &[tex, state] : textures) {
		if (state != SlotState::NeedsInit)
			continue;

		tex = LoadRenderTexture(tex.texture.width, tex.texture.height);
		state = SlotState::InUse;
	}
}

void TextureManager::unload_textures()
{
	assert(does_gl_context_exist);

	for (auto &[tex, state] : textures) {
		if (state != SlotState::InUse)
			continue;

		UnloadRenderTexture(tex);
		state = SlotState::NeedsInit;
	}
}

int TextureManager::get_free_slot()
{
	int id = -1;
	if (!free_list.empty()) {
		id = free_list.back();
		free_list.pop_back();
	} else {
		id = textures.size();
		textures.push_back(std::pair(RenderTexture{}, SlotState::Free));
	}

	assert(textures[id].second == SlotState::Free);

	return id;
}

// Font manager members
//---------------------------------------------------------
FontManager &FontManager::instance()
{
	static unique_ptr<FontManager> ptr(new FontManager());
	return *ptr;
}