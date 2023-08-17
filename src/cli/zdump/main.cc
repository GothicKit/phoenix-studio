// Copyright © 2022 Luis Michaelis <lmichaelis.all+dev@gmail.com>
// SPDX-License-Identifier: MIT
#include "phoenix/animation.hh"
#include "phoenix/font.hh"
#include "phoenix/mesh.hh"
#include "phoenix/messages.hh"
#include "phoenix/model_hierarchy.hh"
#include "phoenix/model_script.hh"
#include "phoenix/texture.hh"
#include "phoenix/vdfs.hh"

#include <CLI/App.hpp>
#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include <iostream>
#include <regex>

#include "config.hh"
#include "dump.hh"

namespace px = phoenix;

enum class file_format {
	mdh,
	man,
	csl,
	fnt,
	msh,
	tex,
	mds,
	msb,
	mrm,
	mdm,
	mmb,
	unknown,
};

int dump(file_format fmt, px::buffer& in, bool bson) {
	nlohmann::json output {};

	if (fmt == file_format::mdh) {
		output = phoenix::model_hierarchy::parse(in);
	} else if (fmt == file_format::man) {
		output = phoenix::animation::parse(in);
	} else if (fmt == file_format::csl) {
		output = phoenix::messages::parse(in);
	} else if (fmt == file_format::fnt) {
		output = phoenix::font::parse(in);
	} else if (fmt == file_format::msh) {
		output = phoenix::mesh::parse(in, {});
	} else if (fmt == file_format::tex) {
		output = phoenix::texture::parse(in);
	} else if (fmt == file_format::mds) {
		output = phoenix::model_script::parse(in);
	} else if (fmt == file_format::msb) {
		output = phoenix::model_script::parse(in);
	} else if (fmt == file_format::mrm) {
		output = phoenix::proto_mesh::parse(in);
	} else if (fmt == file_format::mdm) {
		output = phoenix::model_mesh::parse(in);
	} else if (fmt == file_format::mmb) {
		output = phoenix::morph_mesh::parse(in);
	} else {
		fmt::print(stderr, "format not supported");
		return EXIT_FAILURE;
	}

	if (bson) {
		auto bin = nlohmann::json::to_bson(output);
		std::fwrite(bin.data(), bin.size(), 1, stdout);
	} else {
		std::cout << output.dump(4, ' ', false, nlohmann::json::error_handler_t::replace);
	}

	return EXIT_SUCCESS;
}

file_format detect_file_format(px::buffer&& buf) {
	buf.mark();
	if (buf.get_string(4) == "ZTEX")
		return file_format::tex;

	buf.reset();
	if (buf.get_ushort() == 0xD000)
		return file_format::mdm;

	buf.reset();
	if (buf.get_ushort() == 0xE000)
		return file_format::mmb;

	buf.reset();
	if (buf.get_ushort() == 0xD100)
		return file_format::mdh;

	buf.reset();
	if (buf.get_ushort() == 0xB100)
		return file_format::mrm;

	buf.reset();
	if (buf.get_ushort() == 0xA000)
		return file_format::man;

	buf.reset();
	if (buf.get_ushort() == 0xB000)
		return file_format::msh;

	buf.reset();
	if (buf.get_ushort() == 0xF000)
		return file_format::msb;

	buf.reset();
	std::string line;
	do {
		line = buf.get_line();

		if (std::regex_search(line, std::regex(R"(\s*Model\s*\(\s*"[\w\d]+"\s*\))", std::regex::icase))) {
			buf.reset();
			return file_format::mds;
		}
	} while (line[0] == '/' || line.empty());

	buf.reset();
	if (buf.get_line() == "ZenGin Archive") {
		buf.reset();
		auto copy = buf.duplicate();
		auto reader = phoenix::archive_reader::open(copy);

		px::archive_object obj {};
		if (!reader->read_object_begin(obj))
			return file_format::unknown;

		if (obj.class_name == "zCCSLib")
			return file_format::csl;
	}

	buf.reset();
	if (buf.get_line() == "1") {
		return file_format::fnt;
	}

	return file_format::unknown;
}

int main(int argc, char** argv) {
	px::logging::use_default_logger();

	CLI::App app {"Dump ZenGin files to JSON."};

	bool display_version {false};
	app.add_flag("-v,--version", display_version, "print version information");

	std::optional<std::string> file {};
	app.add_option("-f,--file", file, "operate on this file from disk or a VDF if -e is specified");

	std::optional<std::string> vdf {};
	app.add_option("-e,--vdf", vdf, "open the given file from this VDF");

	bool bson {false};
	app.add_flag("-b,--bson", bson, "dump the contents of the file as BSON");

	CLI11_PARSE(app, argc, argv);

	if (display_version) {
		fmt::print("zdump v{}\n", ZDUMP_VERSION);
		return EXIT_SUCCESS;
	}

	if (!file) {
		fmt::print(stderr, "no input file specified\n");
		return EXIT_FAILURE;
	}

	try {
		auto in = px::buffer::empty();

		if (vdf) {
			const auto container = px::vdf_file::open(*vdf);
			if (const auto* entry = container.find_entry(*file); entry != nullptr) {
				in = entry->open();
			} else {
				fmt::print(stderr, "the file named {} was not found in the VDF {}", *file, *vdf);
				return EXIT_FAILURE;
			}
		} else {
			in = phoenix::buffer::mmap(*file);
		}

		return dump(detect_file_format(in.duplicate()), in, bson);
	} catch (const px::parser_error& e) {
		fmt::print(stderr, "failed to parse file: {}", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
