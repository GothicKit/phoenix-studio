// Copyright © 2023 Luis Michaelis <me@lmichaelis.de>
// SPDX-License-Identifier: MIT-Modern-Variant
#include "GlTexture.hh"

#include <iostream>

namespace studio {
	GlTexture::GlTexture(GLuint id, GLenum type) : _m_id(id), _m_type(type) {}

	GlTexture::GlTexture(GlTexture&& o) noexcept : _m_id(o._m_id), _m_type(o._m_type) {
		o._m_id = 0;
	}

	GlTexture& GlTexture::operator=(GlTexture&& o) noexcept {
		_m_id = o._m_id;
		_m_type = o._m_type;
		o._m_id = 0;
		return *this;
	}

	GlTexture::~GlTexture() noexcept {
		if (_m_id != 0) {
			glDeleteTextures(1, &_m_id);
			_m_id = 0;
		}
	}

	void GlTexture::bind() const {
		glBindTexture(_m_type, _m_id);
	}

	void GlTexture::unbind() const {
		glBindTexture(_m_type, 0);
	}

	GlTexture GlTexture::create(const phoenix::Texture& zen) {

		GLuint id;
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, zen.mipmaps() - 1);

		for (auto i = 0 ; i < zen.mipmaps(); ++i) {
			auto rgba = zen.as_rgba8(i);
			glTexImage2D(GL_TEXTURE_2D,
						 i,
						 GL_RGBA,
						 zen.mipmap_width(i),
						 zen.mipmap_height(i),
						 0,
						 GL_RGBA,
						 GL_UNSIGNED_BYTE,
						 rgba.data());
		}
		glBindTexture(GL_TEXTURE_2D, 0);
		return {id, GL_TEXTURE_2D};
	}
} // namespace studio
