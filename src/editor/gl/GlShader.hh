// Copyright © 2023 Luis Michaelis <me@lmichaelis.de>
// SPDX-License-Identifier: MIT-Modern-Variant
#pragma once
#include "fmt/core.h"
#include "glad/gl.h"

#include "glm/gtc/type_ptr.hpp"
#include "glm/mat4x4.hpp"

#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace studio {
	template <typename T>
	class GlUniform {
	public:
		GlUniform() : _m_location(-1) {}

		void set(T const& val) {
			if constexpr (std::is_same_v<T, glm::mat4>) {
				glUniformMatrix4fv(_m_location, 1, GL_FALSE, glm::value_ptr(val));
			} else {
				throw std::runtime_error {"unsupported operation"};
			}
		}

	protected:
		friend class GlShader;
		explicit GlUniform(GLint location) : _m_location(location) {}

	private:
		GLint _m_location;
	};

	class GlShader {
	public:
		GlShader();
		GlShader(GlShader const&) = delete;
		GlShader(GlShader&&) noexcept;
		GlShader& operator=(GlShader const&) = delete;
		GlShader& operator=(GlShader&&) noexcept;
		~GlShader() noexcept;

		class Builder {
		public:
			Builder& withVertexShader(std::string_view source);
			Builder& withFragmentShader(std::string_view source);
			Builder& withGeometryShader(std::string_view source);
			GlShader build();

		protected:
			Builder& withShader(GLenum type, std::string_view source);

		private:
			std::vector<GLuint> _m_shaders {};
		};

		void activate() const;
		void deactivate() const;

		template <typename T>
		GlUniform<T> get_uniform(std::string const& name) const {
			auto location = glGetUniformLocation(_m_id, name.c_str());
			return GlUniform<T>(location);
		}

		static Builder create();

	protected:
		explicit GlShader(GLuint program);

	private:
		GLuint _m_id;
	};
} // namespace studio