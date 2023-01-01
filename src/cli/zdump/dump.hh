// Copyright © 2022 Luis Michaelis <lmichaelis.all+dev@gmail.com>
// SPDX-License-Identifier: MIT
#pragma once
#include <phoenix/animation.hh>
#include <phoenix/font.hh>
#include <phoenix/mesh.hh>
#include <phoenix/messages.hh>
#include <phoenix/model_hierarchy.hh>
#include <phoenix/model_script.hh>
#include <phoenix/texture.hh>
#include <phoenix/vdfs.hh>
#include <phoenix/world.hh>

#include <nlohmann/json.hpp>

namespace px = phoenix;

namespace phoenix {
	void to_json(nlohmann::json& j, const px::font& obj);

	void to_json(nlohmann::json& j, const px::messages& obj);

	void to_json(nlohmann::json& j, const px::animation& obj);

	void to_json(nlohmann::json& j, const px::model_hierarchy& obj);

	void to_json(nlohmann::json& j, const px::texture& obj);

	void to_json(nlohmann::json& j, const px::mesh& obj);

	void to_json(nlohmann::json& j, const px::model_script& obj);

	void to_json(nlohmann::json& j, const px::world& obj);
} // namespace phoenix
