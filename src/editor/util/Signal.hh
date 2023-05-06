// Copyright © 2023 Luis Michaelis <me@lmichaelis.de>
// SPDX-License-Identifier: MIT-Modern-Variant
#pragma once
#include <functional>
#include <vector>

namespace studio {
	template<typename E>
	using SignalHandler = std::function<void(E const&)>;

	template<typename E>
	class Signal {
	public:
		void bind(SignalHandler<E> const& handler) {
			_m_handlers.push_back(handler);
		}

		void emit(E const& evt) const {
			for (auto& handler : _m_handlers) {
				handler(evt);
			}
		}

	private:
		std::vector<SignalHandler<E>> _m_handlers {};
	};

} // namespace studio
