// Copyright © 2023 Luis Michaelis <me@lmichaelis.de>
// SPDX-License-Identifier: MIT-Modern-Variant
#pragma once
#include "Resources.hh"
#include "gl/GlArray.hh"
#include "gl/GlTexture.hh"
#include "gl/GlShader.hh"

#include <phoenix/World.hh>

#include <glm/mat4x4.hpp>

namespace studio {

	struct WorldSubMesh {
		std::vector<std::uint32_t> indices {};
		GlTexture const* texture {nullptr};
	};

	class World {

	public:
		explicit World(Resources& resources, std::string const& name);

		void update();
		void draw(glm::mat4 const& camera);

	private:
		phoenix::World _m_source;

		GlArray _m_gpu_data;
		GlShader _m_gpu_shader;
		GlUniform<glm::mat4> _m_gpu_shader_camera;
		std::vector<WorldSubMesh> _m_sub_meshes;
	};
} // namespace studio

