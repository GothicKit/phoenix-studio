// Copyright © 2022 Luis Michaelis <lmichaelis.all+dev@gmail.com>
// SPDX-License-Identifier: MIT
#include <phoenix/vdfs.hh>

#include <CLI/App.hpp>
#include <fmt/format.h>

#include <filesystem>
#include <fstream>
#include <iostream>

#include "config.hh"

namespace fs = std::filesystem;
namespace px = phoenix;

static void do_extract(const fs::path& base,
                       const fs::path& self,
                       const std::set<phoenix::vdf_entry, phoenix::vdf_entry_comparator>& entries) {
	for (const auto& entry : entries) {
		auto new_self = self / entry.name;
		auto output = base / new_self;

		if (entry.is_directory()) {
			fs::create_directory(output);
			do_extract(base, new_self, entry.children);
		} else {
			auto content = entry.open();

			std::ofstream out {output};
			out.write((const char*) content.array(), content.limit());
			out.close();
		}
	}
}

static void do_list(const fs::path& self, const std::set<phoenix::vdf_entry, phoenix::vdf_entry_comparator>& entries) {
	for (const phoenix::vdf_entry& entry : entries) {
		auto path = self / entry.name;

		fmt::print("{}\n", path.string<char>());

		if (entry.is_directory()) {
			do_list(path, entry.children);
		}
	}
}

int main(int argc, char** argv) {
	px::logging::use_default_logger();

	CLI::App app {"Extracts or lists files from VDF archives."};

	bool display_version {false};
	app.add_flag("-v,--version", display_version, "Print version information");

	std::optional<std::string> file {};
	app.add_option("-f,--file", file, "Read the VDF from FILE instead of stdin.");

	bool action_list {false};
	app.add_flag("-l,--list", action_list, "Print a list of all files in the VDF.");

	std::optional<std::string> extract {};
	app.add_option("-x,--extract", extract, "Extract the file or directory with the given name.");

	std::optional<std::string> output {};
	app.add_option("-o,--output", output, "Output extracted files to the given path");

	CLI11_PARSE(app, argc, argv);

	try {
		if (display_version) {
			fmt::print("zvdfs v{}\n", ZVDFS_VERSION);
		} else {
			auto in = px::buffer::empty();
			if (file) {
				in = px::buffer::mmap(*file);
			} else {
				std::vector<std::byte> data {};

				while (!std::cin.eof()) {
					data.push_back(static_cast<std::byte>(std::cin.get()));
				}

				if (data.empty()) {
					fmt::print(stderr, "no data provided via stdin");
					return EXIT_FAILURE;
				}

				// remove the EOF byte
				data.pop_back();
				in = phoenix::buffer::of(std::move(data));
			}

			if (action_list) {
				auto vdf = phoenix::vdf_file::open(in);
				do_list("", vdf.entries);
			} else if (extract) {
				const auto vdf = phoenix::vdf_file::open(in);
				if (extract) {
					auto* entry = vdf.find_entry(*extract);
					if (entry == nullptr) {
						fmt::print(stderr, "cannot extract entry {}: not found\n", *extract);
						return EXIT_FAILURE;
					}

					auto buf = entry->open();

					std::optional<std::string> output_arg;
					if (output) {
						std::ofstream out {*output_arg, std::ios::binary};
						out.write((const char*) buf.array(), buf.limit());
						out.close();
					} else {
						std::cout.write((const char*) buf.array(), buf.limit());
					}
				} else {
					std::optional<std::string> output_arg;
					if (output) {
						if (!fs::is_directory(*output_arg)) {
							fmt::print(stderr, "the output directory does not exist.\n");
							return EXIT_FAILURE;
						}

						do_extract(*output_arg, "", vdf.entries);
					} else {
						do_extract(".", "", vdf.entries);
					}
				}
			}
		}
	} catch (const std::exception& e) {
		fmt::print(stderr, "failed to read from vdf: {}", e.what());
		return EXIT_FAILURE;
	}

	return 0;
}
