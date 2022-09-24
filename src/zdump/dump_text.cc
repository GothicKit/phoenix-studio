// Copyright © 2022 Luis Michaelis <lmichaelis.all+dev@gmail.com>
// SPDX-License-Identifier: MIT
#include "glm/gtx/matrix_decompose.hpp"

#include "dump.hh"

static constexpr const char* TEXTURE_FORMAT_NAMES[] = {
    "B8G8R8A8",
    "R8G8B8A8",
    "A8B8G8R8",
    "A8R8G8B8",
    "B8G8R8",
    "R8G8B8",
    "A4R4G4B4",
    "A1R5G5B5",
    "R5G6B5",
    "PAL8",
    "DXT1",
    "DXT2",
    "DXT3",
    "DXT4",
    "DXT5",
};

template <>
void dump_text<px::font>(const px::font& fnt) {
	fmt::print("Type: Font\n");
	fmt::print("Name: {}\n", fnt.name());
	fmt::print("Glyph Count: {}\n", fnt.glyphs().size());
	fmt::print("Glyph Height: {}\n", fnt.height());
	fmt::print("Glyphs:\n");

	for (unsigned i = 0; i < fnt.glyphs().size(); ++i) {
		auto& glyph = fnt.glyphs()[i];
		fmt::print("  {: <4}: u=({}, {}), v=({}, {}), w={}\n",
		           escape<false>(static_cast<char>(i)),
		           glyph.uv[0].x,
		           glyph.uv[0].y,
		           glyph.uv[1].x,
		           glyph.uv[1].y,
		           glyph.width);
	}
}

template <>
void dump_text<px::messages>(const px::messages& messages) {
	fmt::print("Type: Message Database\n");
	fmt::print("Block Count: {}\n", messages.blocks().size());
	fmt::print("Blocks:\n");

	for (const auto& block : messages.blocks()) {
		fmt::print("- Name: {}\n", block.name);
		fmt::print("  Message: type={}, name={}, text={}\n",
		           block.message.type,
		           block.message.name,
		           block.message.text);
	}
}

template <>
void dump_text<px::animation>(const px::animation& animation) {
	fmt::print("Type: Animation\n");
	fmt::print("Name: {}\n", animation.name());
	fmt::print("Next: {}\n", animation.next());
	fmt::print("Layer: {}\n", animation.layer());
	fmt::print("Frames: {}\n", animation.frames());
	fmt::print("FPS: {}\n", animation.frames_per_second());
	fmt::print("FPS alt: {}\n", animation.frames_per_second_alt());
	fmt::print("Source Path: {}\n", animation.source_path());
	fmt::print("Source Script:\n  {}\n", animation.source_script());
	fmt::print("Checksum: {}\n", animation.checksum());

	auto bbox = animation.bbox();
	fmt::print("Bounding Box: min=vec3(x={}, y={}, z={}), max=vec3(x={}, y={}, z={})\n",
	           bbox.min.x,
	           bbox.min.y,
	           bbox.min.z,
	           bbox.max.x,
	           bbox.max.y,
	           bbox.max.z);

	fmt::print("Events:");
	for (const auto& event : animation.events()) {
		fmt::print("\n- Tag: {}\n", event.tag);
		fmt::print("  Type: {}\n", (unsigned) event.type);
		fmt::print("  Number: {}\n", event.no);
		fmt::print("  Probability: {}\n", event.probability);

		fmt::print("  Content: ");
		for (auto& v : event.content)
			fmt::print("{} ", v);

		fmt::print("\n  Values:");
		for (auto v : event.values)
			fmt::print("{} ", v);
	}

	fmt::print("\nSamples: {}\n", animation.samples().size());
}

template <>
void dump_text<px::model_hierarchy>(const px::model_hierarchy& hierachy) {
	fmt::print("Type: Model Hierarchy File\n");
	fmt::print("Root Translation: vec3(x={}, y={}, z={})\n",
	           hierachy.root_translation().x,
	           hierachy.root_translation().y,
	           hierachy.root_translation().z);

	auto [bbox_min, bbox_max] = hierachy.bbox();
	fmt::print("Bounding Box: min=vec3(x={}, y={}, z={}), max=vec3(x={}, y={}, z={})\n",
	           bbox_min.x,
	           bbox_min.y,
	           bbox_min.z,
	           bbox_max.x,
	           bbox_max.y,
	           bbox_max.z);

	auto [coll_min, coll_max] = hierachy.collision_bbox();
	fmt::print("Collision Box: min=vec3(x={}, y={}, z={}), max=vec3(x={}, y={}, z={})\n",
	           coll_min.x,
	           coll_min.y,
	           coll_min.z,
	           coll_max.x,
	           coll_max.y,
	           coll_max.z);

	auto& nodes = hierachy.nodes();
	fmt::print("Node Count: {}\n", nodes.size());
	fmt::print("Nodes:\n");

	auto& root = nodes[0];
	fmt::print("  Name: {}\n", root.name);

	std::function<void(unsigned short, unsigned)> print_tree = [&nodes, &print_tree](unsigned short parent,
	                                                                                 unsigned indent) {
		for (unsigned i = 0; i < nodes.size(); i++) {
			auto& node = nodes[i];
			if (node.parent_index == parent) {
				fmt::print("{:>{}}Name: {}\n", "", indent, node.name);
				glm::vec3 scale;
				glm::quat rotation;
				glm::vec3 translation;
				glm::vec3 skew;
				glm::vec4 perspective;
				glm::decompose(node.transform, scale, rotation, translation, skew, perspective);

				fmt::print("{:>{}}Translation: vec3(x={}, y={}, z={})\n",
				           "",
				           indent,
				           translation.x,
				           translation.y,
				           translation.z);

				fmt::print("{:>{}}Rotation: vec3(x={}, y={}, z={})\n", "", indent, rotation.x, rotation.y, rotation.z);
				fmt::print("{:>{}}Scale: vec3(x={}, y={}, z={})\n", "", indent, scale.x, scale.y, scale.z);
				print_tree(i, indent + 2);
			}
		}
	};

	print_tree(0, 4);
}

template <>
void dump_text<px::texture>(const px::texture& tex) {
	fmt::print("Type: Texture\n");
	fmt::print("Format: {}\n", TEXTURE_FORMAT_NAMES[tex.format()]);
	fmt::print("Width: {}\n", tex.width());
	fmt::print("Height: {}\n", tex.height());
	fmt::print("Reference Width: {}\n", tex.ref_width());
	fmt::print("Reference Height: {}\n", tex.ref_height());
	fmt::print("Mipmap Count: {}\n", tex.mipmaps());
	fmt::print("Average Color (ARGB): #{:0>6x}\n", tex.average_color());
}

template <>
void dump_text<px::mesh>(const px::mesh& msh) {
	fmt::print("Type: Mesh\n");
	fmt::print("Source File: {}\n", msh.name());

	auto date = msh.date();
	fmt::print("Date: {}-{}-{}T{}:{}:{}\n", date.year, date.month, date.day, date.hour, date.minute, date.second);

	auto bbox = msh.bbox();
	fmt::print("Bounding Box: min=vec3(x={}, y={}, z={}), max=vec3(x={}, y={}, z={})\n",
	           bbox.min.x,
	           bbox.min.y,
	           bbox.min.z,
	           bbox.max.x,
	           bbox.max.y,
	           bbox.max.z);

	fmt::print("Vertex Count: {}\n", msh.vertices().size());
	fmt::print("Polygon Count: {}\n", msh.polygons().feature_indices.size());
	fmt::print("Feature Count: {}\n", msh.features().size());
	fmt::print("Lightmaps:\n");

	for (auto& lightmap : msh.lightmaps()) {
		fmt::print("  - Name: {}\n", TEXTURE_FORMAT_NAMES[lightmap.image->format()]);
		fmt::print("    Origin: vec3(x={}, y={}, z={})\n", lightmap.origin.x, lightmap.origin.y, lightmap.origin.z);
		fmt::print("    Normals: a=vec3(x={}, y={}, z={}) b=vec3(x={}, y={}, z={})\n",
		           lightmap.normals[0].x,
		           lightmap.normals[0].y,
		           lightmap.normals[0].z,
		           lightmap.normals[1].x,
		           lightmap.normals[1].y,
		           lightmap.normals[1].z);
	}

	fmt::print("Materials:\n");
	for (auto& material : msh.materials()) {
		fmt::print("  - Name: {}\n", material.name);
		fmt::print("    Texture: {}\n", material.texture);
	}
}

template <>
void dump_text<px::model_script>(const px::model_script& msh) {
	fmt::print("Type: Model Script\n");
	fmt::print("Skeleton: {} {}\n", msh.skeleton.name, msh.skeleton.disable_mesh ? "(without mesh)" : "(with mesh)");
	fmt::print("Model Tags ({}):\n", msh.model_tags.size());

	for (auto& tag : msh.model_tags) {
		fmt::print("  - DEF_HIT_LIMB: {}\n", tag.bone);
	}

	fmt::print("Meshes ({}):\n", msh.meshes.size());

	for (auto& mesh : msh.meshes) {
		fmt::print("  - {}\n", mesh);
	}

	fmt::print("Animations ({}):\n", msh.animations.size());

	for (auto& ani : msh.animations) {
		fmt::print("  - {}, next={} (layer={})\n", ani.name, ani.next, ani.layer);
	}

	fmt::print("Animation Combinations ({}):\n", msh.combinations.size());

	for (auto& ani : msh.combinations) {
		fmt::print("  - {}, next={} (layer={})\n", ani.name, ani.next, ani.layer);
	}

	fmt::print("Animation Blends ({}):\n", msh.blends.size());

	for (auto& ani : msh.blends) {
		fmt::print("  - {}, next={}\n", ani.name, ani.next);
	}

	fmt::print("Animation Aliases ({}):\n", msh.aliases.size());

	for (auto& ani : msh.aliases) {
		fmt::print("  - {}, alias={}\n", ani.name, ani.alias);
	}

	fmt::print("Disabled Animations ({}):\n", msh.disabled_animations.size());

	for (auto& ani : msh.disabled_animations) {
		fmt::print("  - {}\n", ani);
	}
}
