#include "Window.hh"
#include <GLFW/glfw3.h>
#include <glad/gl.h>

namespace studio {
	std::uint32_t Window::_s_open_windows = 0;

	Window::Window(GLFWwindow* wnd) : _m_window(wnd) {
		glfwSetWindowUserPointer(wnd, this);
		glfwGetWindowSize(wnd, &_m_width, &_m_height);

		glfwSetFramebufferSizeCallback(wnd, [](GLFWwindow* handle, std::int32_t w, std::int32_t h){
			auto* slf = static_cast<Window*>(glfwGetWindowUserPointer(handle));
			slf->_m_width = w;
			slf->_m_height = h;
			slf->resize.emit({w, h});
		});
	}

	Window::~Window() noexcept {
		glfwDestroyWindow(_m_window);
		_s_open_windows -= 1;

		if (_s_open_windows == 0) {
			glfwTerminate();
		}
	}

	Window Window::windowed(std::uint32_t w, std::uint32_t h, std::string_view title) {
		// TODO: Error handling
		if (_s_open_windows == 0) {
			glfwInit();
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		GLFWwindow* wnd = glfwCreateWindow(static_cast<std::int32_t>(w), static_cast<std::int32_t>(h), title.data(), nullptr, nullptr);
		_s_open_windows += 1;
		return Window(wnd);
	}

	bool Window::closed() const {
		return glfwWindowShouldClose(_m_window);
	}

	void Window::show() {
		glfwShowWindow(_m_window);
	}

	void Window::hide() {
		glfwHideWindow(_m_window);
	}

	void Window::with(const std::function<void(Window&)>& handler) {
		auto* oldContext = glfwGetCurrentContext();
		glfwMakeContextCurrent(_m_window);
		handler(*this);
		glfwMakeContextCurrent(oldContext);
	}

	void Window::update() {
		glfwSwapBuffers(_m_window);
		glfwPollEvents();
	}
} // namespace studio