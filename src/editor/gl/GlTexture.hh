// Copyright © 2023 Luis Michaelis <me@lmichaelis.de>
// SPDX-License-Identifier: MIT-Modern-Variant
#pragma once
#include <glad/gl.h>

#include <phoenix/Texture.hh>

#include <vector>
#include <cstdint>

namespace studio {
	class GlTexture {
	public:
		GlTexture(GlTexture const&) = delete;
		GlTexture(GlTexture&&)noexcept ;
		GlTexture& operator=(GlTexture const&) = delete;
		GlTexture& operator=(GlTexture&&)noexcept ;
		~GlTexture() noexcept;

		void bind() const;
		void unbind() const;

		static GlTexture create(phoenix::Texture const& zen);

	protected:
		GlTexture(GLuint id, GLenum type);

	private:
		GLuint _m_id;
		GLenum _m_type;
	};
}