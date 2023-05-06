// Copyright © 2023 Luis Michaelis <me@lmichaelis.de>
// SPDX-License-Identifier: MIT-Modern-Variant
#pragma once
#include <glad/gl.h>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <vector>


namespace studio {

	class GlArray {
	public:
		GlArray();
		GlArray(GlArray const&) = delete;
		GlArray(GlArray&&) noexcept;
		GlArray& operator=(GlArray const&) = delete;
		GlArray& operator=(GlArray&&) noexcept;
		~GlArray();

		void attrib(std::vector<glm::vec3> const& d3, GLuint no);
		void attrib(std::vector<glm::vec2> const& d2, GLuint no);

		void bind() const;
		void unbind() const;

	private:
		std::vector<GLuint> _m_buffers;
		GLuint _m_id;

	};

} // namespace studio
