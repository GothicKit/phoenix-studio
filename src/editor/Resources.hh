// Copyright © 2023 Luis Michaelis <me@lmichaelis.de>
// SPDX-License-Identifier: MIT-Modern-Variant
#pragma once
#include "gl/GlTexture.hh"
#include "phoenix/Texture.hh"
#include <memory>
#include <phoenix/Vfs.hh>
#include <phoenix/World.hh>

#include <filesystem>

namespace studio {

	class Resources {
	public:
		explicit Resources(std::filesystem::path const& root);

		GlTexture const& load_texture(std::string const& name);
		phoenix::World load_world(std::string const& name);

	private:
		phoenix::Vfs _m_vfs;

		std::unordered_map<std::string, std::unique_ptr<GlTexture>> _m_cache_textures {};

	};

} // namespace studio
