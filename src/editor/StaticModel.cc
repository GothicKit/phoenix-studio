// Copyright © 2023 Luis Michaelis <me@lmichaelis.de>
// SPDX-License-Identifier: MIT-Modern-Variant
#include "StaticModel.hh"

const char* MESH_VERTEX_SHADER = R"(
#version 330 core
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexture;

uniform mat4 camera;
uniform mat4 transformation;

out vec2 iTexture;

void main() {
	gl_Position = camera * transformation * vec4(vPos, 1.0f);
	iTexture = vTexture;
}
)";

const char* MESH_FRAGMENT_SHADER = R"(
#version 330 core

in vec2 iTexture;
out vec4 fColor;

uniform sampler2D sampler;

void main() {
	vec4 color = texture(sampler, iTexture);
	if (color.a < 0.9f) discard;
	fColor = color;
}
)";

namespace studio {
	StaticModel::StaticModel(Resources& resources, std::string const& name) {
		auto mrm = resources.load_mesh(name);

		std::vector<glm::vec3> vertexPositions {};
		std::vector<glm::vec3> vertexNormals {};
		std::vector<glm::vec2> vertexTextures {};

		auto index = 0u;

		for (auto& mesh : mrm.sub_meshes) {
			StaticModelSubMesh s {};
			s.texture = &resources.load_texture(mesh.mat.texture);

			for (auto& triangle : mesh.triangles) {
				auto w0 = mesh.wedges[triangle.wedges[0]];
				auto w1 = mesh.wedges[triangle.wedges[1]];
				auto w2 = mesh.wedges[triangle.wedges[2]];

				vertexPositions.push_back(mrm.positions[w0.index]);
				vertexPositions.push_back(mrm.positions[w1.index]);
				vertexPositions.push_back(mrm.positions[w2.index]);
				vertexNormals.push_back(w0.normal);
				vertexNormals.push_back(w1.normal);
				vertexNormals.push_back(w2.normal);
				vertexTextures.push_back(w0.texture);
				vertexTextures.push_back(w1.texture);
				vertexTextures.push_back(w2.texture);

				s.indices.push_back(index++);
				s.indices.push_back(index++);
				s.indices.push_back(index++);
			}

			_m_sub_meshes.push_back(std::move(s));
		}

		_m_gpu_data.bind();
		_m_gpu_data.attrib(vertexPositions, 0);
		_m_gpu_data.attrib(vertexNormals, 1);
		_m_gpu_data.attrib(vertexTextures, 2);
		_m_gpu_data.unbind();

		_m_gpu_shader = studio::GlShader::create()
			.withVertexShader(MESH_VERTEX_SHADER)
			.withFragmentShader(MESH_FRAGMENT_SHADER)
			.build();
		_m_gpu_shader_camera = _m_gpu_shader.get_uniform<glm::mat4>("camera");
		_m_gpu_shader_transformation = _m_gpu_shader.get_uniform<glm::mat4>("transformation");
	}

	void StaticModel::draw(const glm::mat4& camera, const glm::mat4& transformation) {
		_m_gpu_shader.activate();
		_m_gpu_data.bind();
		_m_gpu_shader_camera.set(camera);
		_m_gpu_shader_transformation.set(transformation);

		for (auto& subm : _m_sub_meshes) {
			subm.texture->bind();
			glDrawElements(GL_TRIANGLES, subm.indices.size(), GL_UNSIGNED_INT, subm.indices.data());
			subm.texture->unbind();
		}

		_m_gpu_data.unbind();
		_m_gpu_shader.deactivate();
	}

} // namespace studio
