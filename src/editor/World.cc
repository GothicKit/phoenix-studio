//
// Created by Luis Michaelis on 06/05/2023.
//

#include "World.hh"
#include <iostream>

const char* WORLD_VERTEX_SHADER = R"(
#version 330 core
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexture;

uniform mat4 camera;

out vec2 iTexture;

void main() {
	gl_Position = camera * mat4(0.1) * vec4(vPos, 1.0f);
	iTexture = vTexture;
}
)";

const char* WORLD_FRAGMENT_SHADER = R"(
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
	World::World(Resources& resources, const std::string& name) : _m_source(resources.load_world(name)) {
		auto& mesh = _m_source.world_mesh;

		std::vector<glm::vec3> vertexPositions {};
		std::vector<glm::vec3> vertexNormals {};
		std::vector<glm::vec2> vertexTextures {};
		_m_sub_meshes.resize(mesh.materials.size());

		for (auto i = 0; i < mesh.polygons.vertex_indices.size() / 3; ++i) {
			auto i0 = mesh.polygons.vertex_indices[i * 3];
			auto i1 = mesh.polygons.vertex_indices[i * 3 + 1];
			auto i2 = mesh.polygons.vertex_indices[i * 3 + 2];
			auto f0 = mesh.polygons.feature_indices[i * 3];
			auto f1 = mesh.polygons.feature_indices[i * 3 + 1];
			auto f2 = mesh.polygons.feature_indices[i * 3 + 2];

			vertexPositions.push_back(mesh.vertices[i0] * glm::vec3 {-1, 1, 1});
			vertexPositions.push_back(mesh.vertices[i1] * glm::vec3 {-1, 1, 1});
			vertexPositions.push_back(mesh.vertices[i2] * glm::vec3 {-1, 1, 1});
			vertexNormals.push_back(mesh.features[f0].normal);
			vertexNormals.push_back(mesh.features[f1].normal);
			vertexNormals.push_back(mesh.features[f2].normal);
			vertexTextures.push_back(mesh.features[f0].texture);
			vertexTextures.push_back(mesh.features[f1].texture);
			vertexTextures.push_back(mesh.features[f2].texture);

			auto m = mesh.polygons.material_indices[i];
			_m_sub_meshes[m].indices.push_back(i * 3);
			_m_sub_meshes[m].indices.push_back(i * 3 + 1);
			_m_sub_meshes[m].indices.push_back(i * 3 + 2);

			if (_m_sub_meshes[m].texture == nullptr) {
				_m_sub_meshes[m].texture = &resources.load_texture(mesh.materials[m].texture);
			}
		}

		_m_gpu_data.bind();
		_m_gpu_data.attrib(vertexPositions, 0);
		_m_gpu_data.attrib(vertexNormals, 1);
		_m_gpu_data.attrib(vertexTextures, 2);
		_m_gpu_data.unbind();

		_m_gpu_shader = studio::GlShader::create()
		                    .withVertexShader(WORLD_VERTEX_SHADER)
		                    .withFragmentShader(WORLD_FRAGMENT_SHADER)
		                    .build();
		_m_gpu_shader_camera = _m_gpu_shader.get_uniform<glm::mat4>("camera");

		for (auto& vob : _m_source.world_vobs) {
			std::cout << vob->vob_name << ": " << (int) vob->associated_visual_type << " (" << vob->visual_name
			          << "/" << vob->show_visual << ")\n";

			try {
				resources.load_mesh(vob->visual_name);
			} catch (std::runtime_error const&) {
				std::cout << "Not found!\n";
			}
		}
	}

	void World::update() {}

	void World::draw(const glm::mat4& camera) {
		_m_gpu_shader.activate();
		_m_gpu_data.bind();
		_m_gpu_shader_camera.set(camera);

		for (auto& subm : _m_sub_meshes) {
			if (subm.texture == nullptr)
				continue;
			subm.texture->bind();
			glDrawElements(GL_TRIANGLES, subm.indices.size(), GL_UNSIGNED_INT, subm.indices.data());
			subm.texture->unbind();
		}

		_m_gpu_data.unbind();
		_m_gpu_shader.deactivate();
	}
} // namespace studio