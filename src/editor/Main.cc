#include "Resources.hh"
#include "Window.hh"
#include "gl/GlArray.hh"
#include "gl/GlShader.hh"
#include "gl/GlTexture.hh"

#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <phoenix/Material.hh>
#include <phoenix/Texture.hh>
#include <phoenix/Vfs.hh>
#include <phoenix/World.hh>
#include <phoenix/Phoenix.hh>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
const char* VERTEX_SHADER = R"(
#version 330 core
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexture;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 iTexture;

void main() {
	gl_Position = projection * view * model * vec4(vPos, 1.0f);
	iTexture = vTexture;
}
)";

const char* FRAGMENT_SHADER = R"(
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

struct SubMesh {
	std::vector<std::uint32_t> indices;
	phoenix::Material const* material;
	studio::GlTexture const* textureId = nullptr;
};

struct BasicObject {
	glm::mat4 transform;
};

#define ROOT "/mnt/games/Gothic/Gothic108kDE"

int main() {
	phoenix::Logging::use_default_logger();
	auto window = studio::Window::windowed(720, 480, "Testing");
	window.show();

	studio::Resources resources {ROOT};

	window.resize.bind([](auto e) { glViewport(0, 0, e.width, e.height); });
	window.with([&resources](studio::Window& wnd) {
		gladLoadGL((GLADloadfunc) glfwGetProcAddress);

		auto projection = glm::perspective(glm::radians(80.f), 720.f / 480.f, 0.01f, 10000.f);
		auto view = glm::translate(glm::identity<glm::mat4>(), {0., 0., 0.});
		auto model = glm::translate(glm::identity<glm::mat4>(), {0., 0., -100.});
		model = glm::scale(model, {.01, .01, .01});

		wnd.resize.bind([&projection](auto e) {
			projection = glm::perspective(glm::radians(80.f), (float) e.width / (float) e.height, 0.01f, 10000.f);
		});

		auto world = resources.load_world("WORLD.ZEN");
		auto mesh = world.world_mesh;

		std::vector<glm::vec3> vertexPositions {};
		std::vector<glm::vec3> vertexNormals {};
		std::vector<glm::vec2> vertexTextures {};
		std::vector<SubMesh> subMeshes {};
		subMeshes.resize(mesh.materials.size());

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
			subMeshes[m].indices.push_back(i * 3);
			subMeshes[m].indices.push_back(i * 3 + 1);
			subMeshes[m].indices.push_back(i * 3 + 2);
			subMeshes[m].material = &mesh.materials[m];

			if (subMeshes[m].textureId == nullptr) {
				subMeshes[m].textureId = &resources.load_texture(subMeshes[m].material->texture);
			}
		}

		auto shader =
		    studio::GlShader::create().withVertexShader(VERTEX_SHADER).withFragmentShader(FRAGMENT_SHADER).build();

		auto uniformProject = shader.get_uniform<glm::mat4>("projection");
		auto uniformView = shader.get_uniform<glm::mat4>("view");
		auto uniformModel = shader.get_uniform<glm::mat4>("model");

		studio::GlArray obj {};
		obj.bind();
		obj.attrib(vertexPositions, 0);
		obj.attrib(vertexNormals, 1);
		obj.attrib(vertexTextures, 2);
		obj.unbind();

		glEnable(GL_DEPTH_TEST);

		std::cout << "Ready.\n";

		while (!wnd.closed()) {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glClearColor(0, 0, 0, 0);

			if (glfwGetKey(wnd.handle(), GLFW_KEY_W)) {
				view = glm::translate(view, {0, 0, 1});
			}
			if (glfwGetKey(wnd.handle(), GLFW_KEY_S)) {
				view = glm::translate(view, {0, 0, -1});
			}
			if (glfwGetKey(wnd.handle(), GLFW_KEY_A)) {
				view = glm::translate(view, {1, 0, 0});
			}
			if (glfwGetKey(wnd.handle(), GLFW_KEY_D)) {
				view = glm::translate(view, {-1, 0, 0});
			}
			if (glfwGetKey(wnd.handle(), GLFW_KEY_SPACE)) {
				view = glm::translate(view, {0, -1, 0});
			}
			if (glfwGetKey(wnd.handle(), GLFW_KEY_LEFT_SHIFT)) {
				view = glm::translate(view, {0, 1, 0});
			}

			shader.activate();
			obj.bind();

			uniformProject.set(projection);
			uniformView.set(view);
			uniformModel.set(model);

			for (auto& subm : subMeshes) {
				if (subm.textureId == nullptr)
					continue ;
				subm.textureId->bind();
				glDrawElements(GL_TRIANGLES, subm.indices.size(), GL_UNSIGNED_INT, subm.indices.data());
				subm.textureId->unbind();
			}

			obj.unbind();
			shader.deactivate();

			wnd.update();
		}
	});
	return 0;
}
