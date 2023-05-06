// Copyright © 2023 Luis Michaelis <me@lmichaelis.de>
// SPDX-License-Identifier: MIT-Modern-Variant
#include "Resources.hh"
#include "gl/GlTexture.hh"
#include "phoenix/Phoenix.hh"
#include "phoenix/World.hh"
#include <exception>

namespace studio {
	Resources::Resources(const std::filesystem::path& root) {
		for (auto& disk : std::filesystem::directory_iterator {root / "Data"}) {
			auto& path = disk.path();

			if (!phoenix::iequals(path.extension().string(), ".VDF"))
				continue;

			_m_vfs.mount_disk(path);
		}

		_m_vfs.mount_host(root / "_work", "/");
	}

	GlTexture const& Resources::load_texture(const std::string& name) {
		auto it = _m_cache_textures.find(name);
		if (it != _m_cache_textures.end())
			return *it->second;

		// TODO: TGA loading!

		auto* node = _m_vfs.find(name);
		if (node == nullptr)
			node = _m_vfs.find(name.substr(0, name.rfind('.')) + "-C.TEX");
		if (node == nullptr)
			return load_texture("DEFAULT.TGA");

		try {
			auto texture = phoenix::Texture::parse(node->open());
			auto generated = studio::GlTexture::create(texture);
			auto [rv, _] = _m_cache_textures.insert(std::make_pair(name, std::make_unique<GlTexture>(std::move(generated))));
			return *rv->second;
		} catch (std::exception const& e) {
			return load_texture("DEFAULT.TGA");
		}
	}

	phoenix::World Resources::load_world(const std::string& name) {
		auto* node = _m_vfs.find(name);
		if (node == nullptr)
			throw std::runtime_error {"World not found!"};
		return phoenix::World::parse(node->open());
	}
} // namespace studio