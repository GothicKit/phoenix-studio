// Copyright © 2023 Luis Michaelis <me@lmichaelis.de>
// SPDX-License-Identifier: MIT-Modern-Variant
#pragma once
#include "gl/GlArray.hh"
#include "gl/GlShader.hh"
#include "Resources.hh"
#include "gl/GlTexture.hh"

#include <glm/mat4x4.hpp>

#include <phoenix/MultiResolutionMesh.hh>

namespace studio {
	struct StaticModelSubMesh {
		std::vector<uint32_t> indices {};
		GlTexture const* texture {nullptr};
	};

	class StaticModel {
	public:
		explicit StaticModel(Resources& resources, std::string const& mrm);

		void draw(glm::mat4 const& camera, glm::mat4 const& transformation);

	private:
		GlArray _m_gpu_data;
		GlShader _m_gpu_shader;
		GlUniform<glm::mat4> _m_gpu_shader_camera, _m_gpu_shader_transformation;
		std::vector<StaticModelSubMesh> _m_sub_meshes;
	};
} // namespace studio