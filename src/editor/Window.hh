// Copyright © 2023 Luis Michaelis <me@lmichaelis.de>
// SPDX-License-Identifier: MIT-Modern-Variant
#pragma once
#include "util/Signal.hh"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cstdint>
#include <string_view>

namespace studio {

	struct WindowSize {
		std::int32_t width;
		std::int32_t height;
	};

	class Window {
	public:
		~Window() noexcept;

		void with(std::function<void(Window&)> const& handler);

		[[nodiscard]] bool closed() const;
		void update();

		void show();
		void hide();

		[[nodiscard]] static Window windowed(std::uint32_t w, std::uint32_t h, std::string_view title);

		inline GLFWwindow* handle() const {
			return _m_window;
		}
	public:
		Signal<WindowSize> resize {};

	protected:
		explicit Window(GLFWwindow* wnd);

	private:
		static std::uint32_t _s_open_windows;

		GLFWwindow* _m_window;
		std::int32_t _m_width, _m_height;
	};

} // namespace studio
