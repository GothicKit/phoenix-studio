// Copyright © 2022 Luis Michaelis <lmichaelis.all+dev@gmail.com>
// SPDX-License-Identifier: MIT
#pragma once
#include <phoenix/Animation.hh>
#include <phoenix/CutsceneLibrary.hh>
#include <phoenix/Font.hh>
#include <phoenix/Mesh.hh>
#include <phoenix/ModelHierarchy.hh>
#include <phoenix/ModelScript.hh>
#include <phoenix/Texture.hh>
#include <phoenix/World.hh>

#include <nlohmann/json.hpp>

namespace phoenix {
	void to_json(nlohmann::json& j, const phoenix::Font& obj);

	void to_json(nlohmann::json& j, const phoenix::CutsceneLibrary& obj);

	void to_json(nlohmann::json& j, const phoenix::Animation& obj);

	void to_json(nlohmann::json& j, const phoenix::ModelHierarchy& obj);

	void to_json(nlohmann::json& j, const phoenix::Texture& obj);

	void to_json(nlohmann::json& j, const phoenix::Mesh& obj);

	void to_json(nlohmann::json& j, const phoenix::ModelScript& obj);

	void to_json(nlohmann::json& j, const phoenix::World& obj);
} // namespace phoenix
