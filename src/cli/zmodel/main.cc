// Copyright © 2022 Luis Michaelis <lmichaelis.all+dev@gmail.com>
// SPDX-License-Identifier: MIT
#include <phoenix/model.hh>
#include <phoenix/morph_mesh.hh>
#include <phoenix/proto_mesh.hh>
#include <phoenix/vdfs.hh>
#include <phoenix/world.hh>

#include <CLI/App.hpp>
#include <fmt/format.h>

#include <fstream>
#include <iostream>

#include "config.hh"

namespace px = phoenix;

static void dump_material(std::ostream& mtl, std::string_view name, const std::vector<phoenix::material>& materials) {
	for (const auto& mat : materials) {
		if (!mat.texture.empty()) {
			mtl << "newmtl " << mat.name << "\n";
			mtl << "Kd 1.00 1.00 1.00\n";
			mtl << "map_Kd " << mat.texture << "\n";
		}
	}
}

static void dump_wavefront(std::ostream& out,
                           std::ostream* material_out,
                           std::string_view mtllib_name,
                           const phoenix::proto_mesh& mesh) {
	out << "# zmodel exported mesh\n";
	if (material_out != nullptr)
		out << "mtllib " << mtllib_name << ".mtl\n\n";

	out << "# vertices\n";

	for (const auto& item : mesh.positions) {
		out << "v " << item.x << " " << item.y << " " << item.z << "\n";
	}

	unsigned wedge_offset = 0;
	int i = 0;
	for (const auto& msh : mesh.sub_meshes) {
		out << "g sub" << ++i << "\n"
		    << "usemtl " << msh.mat.name << "\n";

		for (const auto& item : msh.wedges) {
			out << "vn " << item.normal.x << " " << item.normal.y << " " << item.normal.z << "\n";
			out << "vt " << item.texture.x << " " << item.texture.y << "\n";
		}

		for (const auto& item : msh.triangles) {
			auto wedge0 = msh.wedges[item.wedges[0]];
			auto wedge1 = msh.wedges[item.wedges[1]];
			auto wedge2 = msh.wedges[item.wedges[2]];

			out << "f " << wedge0.index + 1 << "/" << wedge_offset + item.wedges[0] + 1 << "/"
			    << wedge_offset + item.wedges[0] + 1 << " " << wedge1.index + 1 << "/"
			    << wedge_offset + item.wedges[1] + 1 << "/" << wedge_offset + item.wedges[1] + 1 << " "
			    << wedge2.index + 1 << "/" << wedge_offset + item.wedges[2] + 1 << "/"
			    << wedge_offset + item.wedges[2] + 1 << "\n";
		}

		wedge_offset += msh.wedges.size();
	}

	if (material_out != nullptr) {
		dump_material(*material_out, mtllib_name, mesh.materials);
	}
}

static void
dump_wavefront(std::ostream& out, std::ostream* material_out, std::string_view mtllib_name, const phoenix::mesh& mesh) {
	out << "# zmodel exported mesh\n";
	if (material_out != nullptr)
		out << "mtllib " << mtllib_name << ".mtl\n\n";

	out << "# vertices\n";

	for (const auto& item : mesh.vertices) {
		out << "v " << item.z << " " << item.y << " " << item.x << "\n";
	}

	auto& mats = mesh.materials;
	auto& feats = mesh.features;

	out << "\n# normals\n";
	for (const auto& feat : feats) {
		out << "vn " << feat.normal.x << " " << feat.normal.y << " " << feat.normal.z << "\n";
	}

	out << "\n# textures\n";
	for (const auto& feat : feats) {
		out << "vt " << feat.texture.x << " " << feat.texture.y << "\n";
	}

	long old_material = -1;
	auto& polys = mesh.polygons;
	for (unsigned i = 0; i < polys.vertex_indices.size() / 3; ++i) {
		auto material = polys.material_indices[i];

		if (old_material != material) {
			auto& mat = mats[material];
			out << "usemtl " << mat.name << "\n";
			out << "g " << mat.name << "\n";
			old_material = material;
		}

		out << "f ";
		for (unsigned v = 0; v < 3; ++v) {
			auto feature = polys.feature_indices[i * 3 + v] + 1;
			out << polys.vertex_indices[i * 3 + v] + 1 << "/" << feature << "/" << feature;

			if (v != 2) {
				out << " ";
			}
		}

		out << "\n";
	}

	if (material_out != nullptr) {
		dump_material(*material_out, mtllib_name, mesh.materials);
	}
}

px::buffer open_buffer(const std::optional<std::string>& input, const std::optional<std::string>& vdf) {
	if (input) {
		if (vdf) {
			const auto container = px::vdf_file::open(*vdf);
			if (auto* entry = container.find_entry(*input); entry != nullptr) {
				return entry->open();
			} else {
				fmt::print(stderr, "the file named {} was not found in the VDF {}", *input, *vdf);
				return px::buffer::empty();
			}
		} else {
			return px::buffer::mmap(*input);
		}
	} else {
		std::vector<std::byte> data {};

		while (!std::cin.eof()) {
			data.push_back(static_cast<std::byte>(std::cin.get()));
		}

		if (data.empty()) {
			fmt::print(stderr, "no data provided via stdin");
			return px::buffer::empty();
		}

		// remove the EOF byte
		data.pop_back();
		return px::buffer::of(std::move(data));
	}
}

int main(int argc, char** argv) {
	px::logging::use_default_logger();

	CLI::App app {"Dump ZenGin files to JSON."};

	bool display_version {false};
	app.add_flag("-v,--version", display_version, "Print version information");

	std::optional<std::string> file {};
	app.add_option("-f,--file", file, "Operate on this file from disk or a VDF if -e is specified");

	std::optional<std::string> vdf {};
	app.add_option("-e,--vdf", vdf, "Open the given file from this VDF");

	std::optional<std::string> output {};
	app.add_option("-o,--output", output, "Write data to the given path instead of stdout.");

	std::optional<std::string> material {};
	app.add_option("-m,--material", material, "Also write a material file to the given path");

	CLI11_PARSE(app, argc, argv);

	if (display_version) {
		fmt::print("zmodel v{}\n", ZMODEL_VERSION);
	} else {
		try {
			if (!file) {
				fmt::print(stderr, "no input file given\n");
				return EXIT_FAILURE;
			}

			auto in = open_buffer(file, vdf);
			auto extension = file->substr(file->find('.') + 1);

			std::ostream* model_out = &std::cout;
			if (output)
				model_out = new std::ofstream {*output};

			std::ostream* material_out = nullptr;
			if (material)
				material_out = new std::ofstream {*material};

			if (phoenix::iequals(extension, "MRM")) {
				auto mesh = phoenix::proto_mesh::parse(in);
				dump_wavefront(*model_out, material_out, material.value_or(""), mesh);
			} else if (phoenix::iequals(extension, "ZEN")) {
				auto wld = phoenix::world::parse(in);
				dump_wavefront(*model_out, material_out, material.value_or(""), wld.world_mesh);
			} else if (phoenix::iequals(extension, "MSH")) {
				auto msh = phoenix::mesh::parse(in, {});
				dump_wavefront(*model_out, material_out, material.value_or(""), msh);
			} else if (phoenix::iequals(extension, "MMB")) {
				auto msh = phoenix::morph_mesh::parse(in);
				dump_wavefront(*model_out, material_out, material.value_or(""), msh.mesh);
			} else if (phoenix::iequals(extension, "MDL")) {
				auto msh = phoenix::model::parse(in);
				dump_wavefront(*model_out,
				               material_out,
				               material.value_or(""),
				               msh.mesh.meshes[0].mesh); // FIXME: support dumping multiple meshes
			} else if (phoenix::iequals(extension, "MDM")) {
				auto msh = phoenix::model_mesh::parse(in);
				dump_wavefront(*model_out,
				               material_out,
				               material.value_or(""),
				               msh.meshes[0].mesh); // FIXME: support dumping multiple meshes
			} else {
				fmt::print(stderr, "format not supported: {}", extension);
				return EXIT_FAILURE;
			}

			model_out->flush();

			if (material_out != nullptr) {
				material_out->flush();
				delete material_out;
			}
		} catch (const std::exception& e) {
			fmt::print(stderr, "cannot convert model: {}", e.what());
			return EXIT_FAILURE;
		}
	}

	return 0;
}
