// Copyright © 2023 Luis Michaelis <me@lmichaelis.de>
// SPDX-License-Identifier: MIT-Modern-Variant
#include "GlShader.hh"
#include "glad/gl.h"
#include <cstdio>

namespace studio {
	GlShader::Builder& GlShader::Builder::withVertexShader(std::string_view source) {
		return this->withShader(GL_VERTEX_SHADER, source);
	}

	GlShader::Builder& GlShader::Builder::withFragmentShader(std::string_view source) {
		return this->withShader(GL_FRAGMENT_SHADER, source);
	}

	GlShader::Builder& GlShader::Builder::withGeometryShader(std::string_view source) {
		return this->withShader(GL_GEOMETRY_SHADER, source);
	}

	GlShader::Builder& GlShader::Builder::withShader(GLenum type, std::string_view source) {
		// TODO: Error handling
		GLuint shader = glCreateShader(type);

		auto* data = source.data();
		auto size = static_cast<GLint>(source.size());
		glShaderSource(shader, 1, &data, &size);
		glCompileShader(shader);

		_m_shaders.push_back(shader);
		return *this;
	}

	GlShader GlShader::Builder::build() {
		// TODO: Error handling
		GLuint program = glCreateProgram();
		for (auto shader : _m_shaders)
			glAttachShader(program, shader);

		glLinkProgram(program);
		glValidateProgram(program);

		for (auto shader : _m_shaders)
			glDeleteShader(shader);

		return GlShader {program};
	}

	GlShader::GlShader(GLuint program) : _m_id(program) {}

	GlShader::GlShader() noexcept {
		if (_m_id != 0) {
			glDeleteProgram(_m_id);
		}
	}

	GlShader::GlShader(GlShader&& o) noexcept : _m_id(o._m_id) {
		o._m_id = 0;
	}
	GlShader& GlShader::operator=(GlShader&& o) noexcept {
		_m_id = o._m_id;
		o._m_id = 0;
		return *this;
	}

	GlShader::Builder GlShader::create() {
		return {};
	}

	void GlShader::activate() const {
		glUseProgram(_m_id);
	}

	void GlShader::deactivate() const {
		glUseProgram(0);
	}

	template <>
	GlUniform<glm::mat4> GlShader::get_uniform(std::string const& name) const {
		// TODO: Check type
		auto location = glGetUniformLocation(_m_id, name.c_str());
		return GlUniform<glm::mat4>(location);
	}
} // namespace studio