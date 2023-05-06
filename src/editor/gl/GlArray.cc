// Copyright © 2023 Luis Michaelis <me@lmichaelis.de>
// SPDX-License-Identifier: MIT-Modern-Variant
#include "GlArray.hh"

namespace studio {
	GlArray::GlArray() : _m_id(0) {
		glGenVertexArrays(1, &_m_id);
	}

	GlArray::GlArray(GlArray&& o) noexcept : _m_id(o._m_id) {
		o._m_id = 0;
		o._m_buffers.clear();
	}

	GlArray& GlArray::operator=(GlArray&& o) noexcept {
		_m_id = o._m_id;
		o._m_id = 0;
		o._m_buffers.clear();
		return *this;
	}

	GlArray::~GlArray() {
		glDeleteBuffers(_m_buffers.size(), _m_buffers.data());
		glDeleteVertexArrays(1, &_m_id);
	}

	void GlArray::attrib(const std::vector<glm::vec3>& d3, GLuint no) {
		GLuint buf;
		glGenBuffers(1, &buf);
		glBindBuffer(GL_ARRAY_BUFFER, buf);
		glBufferData(GL_ARRAY_BUFFER,
					 d3.size() * sizeof(glm::vec3),
					 d3.data(),
					 GL_STATIC_DRAW);
		glEnableVertexArrayAttrib(_m_id, no);
		glVertexAttribPointer(no, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		_m_buffers.push_back(buf);
	}

	void GlArray::attrib(const std::vector<glm::vec2>& d2, GLuint no) {
		GLuint buf;
		glGenBuffers(1, &buf);
		glBindBuffer(GL_ARRAY_BUFFER, buf);
		glBufferData(GL_ARRAY_BUFFER,
					 d2.size() * sizeof(glm::vec2),
					 d2.data(),
					 GL_STATIC_DRAW);
		glEnableVertexArrayAttrib(_m_id, no);
		glVertexAttribPointer(no, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		_m_buffers.push_back(buf);
	}

	void GlArray::bind() const {
		glBindVertexArray(_m_id);
	}

	void GlArray::unbind() const {
		glBindVertexArray(0);
	}
} // namespace studio