#include "Resources.hh"
#include "Window.hh"
#include "gl/GlArray.hh"
#include "gl/GlShader.hh"
#include "gl/GlTexture.hh"
#include "World.hh"
#include "StaticModel.hh"

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

#define ROOT R"(C:\Users\Luis Michaelis\Documents\Gothic\Gothic108kDE)"

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

		wnd.resize.bind([&projection](auto e) {
			projection = glm::perspective(glm::radians(80.f), (float) e.width / (float) e.height, 0.01f, 10000.f);
		});

		// studio::World world {resources, "WORLD.ZEN"};
		// _WORK/DATA/MESHES/_COMPILED/jeej
		studio::StaticModel model {resources, "OW_LOB_TREE_V9.MRM"};

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

			model.draw(projection * view, glm::rotate(glm::identity<glm::mat4>(), (float) glfwGetTime(), {0.f, 1.f, 0.f}));
			wnd.update();
		}
	});
	return 0;
}
