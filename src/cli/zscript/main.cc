// Copyright © 2022 Luis Michaelis <lmichaelis.all+dev@gmail.com>
// SPDX-License-Identifier: MIT
#include <phoenix/DaedalusScript.hh>

#include <CLI/App.hpp>
#include <fmt/format.h>

#include <fstream>
#include <string>
#include <string_view>
#include <unordered_map>

#include "config.hh"
#include "decompiler.hh"

using namespace phoenix;

#define PRINT_FLAG(cond, flag)                                                                                         \
	do {                                                                                                               \
		if (cond) {                                                                                                    \
			fmt::print("{}", flag);                                                                                    \
		} else {                                                                                                       \
			fmt::print(" ");                                                                                           \
		}                                                                                                              \
	} while (false)

#define SV_CONTAINS(sv, chr) ((sv).find(chr) != std::string_view::npos)

void print_assembly_of_symbol(const DaedalusScript& scr, const DaedalusSymbol& sym);
void print_assembly(const DaedalusScript& scr);

void print_symbol_detailed(const DaedalusScript& scr, const DaedalusSymbol& sym);
void print_symbol_list(const DaedalusScript& scr,
                       std::string_view include_filter,
                       std::string_view exclude_filter,
                       std::string_view search);

std::string print_definition(const DaedalusScript& scr,
                             const DaedalusSymbol& sym,
                             const DaedalusSymbol* parent,
                             std::string_view indent = "");
void find_usages(const DaedalusScript& scr, const DaedalusSymbol& sym);

int main(int argc, char** argv) {
	phoenix::Logging::use_default_logger();

	CLI::App app {"Inspect ZenGin script files."};

	bool display_version {false};
	app.add_flag("-v,--version", display_version, "Print version information");

	std::optional<std::string> file {};
	app.add_option("-f,--file", file, "Operate on this file from disk or a VDF if -e is specified");

	std::optional<std::string> vdf {};
	app.add_option("-e,--vdf", vdf, "Open the given file from this VDF");

	bool action_symbolize = false;
	bool action_disassemble = false;
	bool action_usages = false;
	bool action_decompile = false;
	app.add_flag("-t,--symbols",
	             action_symbolize,
	             "Print a list of all symbols or details of the symbol passed using -s");
	app.add_flag("-d,--disassemble", action_disassemble, "Print the disassembly of the symbol passed using -s");
	app.add_flag("-u,--usages", action_usages, "Print the locations of all usages of the symbol passed using -s");
	app.add_flag("-k,--decompile", action_decompile, "Print a decompilation of the symbol passed using -s");

	std::optional<std::string> symbol_name {};
	app.add_option("-s,--symbol", symbol_name, "The name of a symbol to inspect");

	std::optional<std::string> include {};
	app.add_option("-i,--include", include, "Only display symbols with any of the given flags (only relevant for -t)");

	std::optional<std::string> exclude {};
	app.add_option("-x,--exclude",
	               exclude,
	               "Only display symbols without any of the given flags (only relevant for -t)");

	std::optional<std::string> search {};
	app.add_option("-c,--find",
	               search,
	               "Only display symbols where the name contains the given perm (only relevant for -t)");

	CLI11_PARSE(app, argc, argv);

	if (display_version) {
		fmt::print("zscript v{}\n", ZSCRIPT_VERSION);
		return EXIT_SUCCESS;
	} else {
		if (!file) {
			fmt::print(stderr, "no input file given");
			return EXIT_FAILURE;
		}

		auto scr = DaedalusScript::parse(*file);
		const DaedalusSymbol* sym = nullptr;

		if (symbol_name) {
			sym = scr.find_symbol_by_name(*symbol_name);

			if (sym == nullptr) {
				fmt::print("symbol with name {} not found\n", *symbol_name);
				return EXIT_FAILURE;
			}
		}

		if (action_symbolize) {
			if (symbol_name) {
				print_symbol_detailed(scr, *sym);
			} else {
				print_symbol_list(scr, include.value_or(""), exclude.value_or(""), search.value_or(""));
			}
		} else if (action_disassemble) {
			if (symbol_name) {
				print_assembly_of_symbol(scr, *sym);
			} else {
				print_assembly(scr);
			}
		} else if (action_usages) {
			if (!symbol_name) {
				fmt::print(stderr, "Please provide a symbol of which to find usages.");
				return EXIT_FAILURE;
			}

			find_usages(scr, *sym);
		} else if (action_decompile) {
			if (symbol_name) {
				fmt::print("{}", print_definition(scr, *sym, scr.find_symbol_by_index(sym->parent())));
				fmt::print(" {{\n{}}}\n", decompile(scr, *sym, 4));
			} else {
				std::unordered_map<uint32_t, std::string> files;
				std::unordered_map<uint32_t, std::string> file_names;

				for (auto& s : scr.symbols()) {
					if (s.name() == "$INSTANCE_HELP" || s.is_generated() || s.name().find('.') != -1)
						continue;

					if (files.find(s.file_index()) == files.end()) {
						files[s.file_index()] = {};
					}

					std::string def = print_definition(scr, s, scr.find_symbol_by_index(s.parent()));

					if (!s.is_member() && !s.is_external() &&
					    (s.type() == DaedalusDataType::PROTOTYPE ||
					     (s.type() == DaedalusDataType::FUNCTION && s.is_const()) ||
					     (s.type() == DaedalusDataType::INSTANCE && s.is_const()))) {
						def += fmt::format(" {{\n{}}}\n", decompile(scr, s, 4));
					}

					files[s.file_index()] += def + "\n";

					if (file_names.find(s.file_index()) == file_names.end()) {
						file_names[s.file_index()] = s.name();
					}

					fmt::print("Processed {}\n", s.name());
				}

				for (auto& idx : files) {
					std::ofstream out {file_names[idx.first] + ".d"};
					out << idx.second;
					out.close();
				}

				std::ofstream src_file {"Gothic.src"};
				for (auto& fn : file_names) {
					src_file << fn.second << ".d\n";
				}
				src_file.close();
			}
		}
	}

	return 0;
}

/// \brief Tests whether the symbol has a value related to it
/// \param symbol The symbol to test on
/// \return `true` if the symbol has a value `false` otherwise.
bool has_constant_value(const DaedalusSymbol& symbol) {
	return symbol.is_const() && symbol.type() != DaedalusDataType::PROTOTYPE &&
	    symbol.type() != DaedalusDataType::INSTANCE && symbol.type() != DaedalusDataType::FUNCTION;
}

/// \brief Prints the value of a symbol
/// \param symbol The symbol to print the value of
std::string print_symbol_value(const DaedalusSymbol& symbol, int index = -1) {
	std::string val {};
	auto print_value = [&](uint8_t index) {
		switch (symbol.type()) {
		case DaedalusDataType::FLOAT:
			val += fmt::format("{}", symbol.get_float(index));
			break;
		case DaedalusDataType::INT:
			val += fmt::format("{}", symbol.get_int(index));
			break;
		case DaedalusDataType::STRING:
			val += fmt::format("\"{}\"", symbol.get_string(index));
			break;
		default:
			break;
		}
	};

	if (index >= 0) {
		print_value(index);
	} else {
		if (symbol.count() > 1)
			val += "{";

		for (unsigned i = 0; i < symbol.count(); ++i) {
			if (i > 0)
				val += ", ";
			print_value(i);
		}

		if (symbol.count() > 1) {
			val += "}";
		}
	}

	return val;
}

/// \brief Generates a daedalus definition from the given symbol.
/// \param scr The script to reference
/// \param sym The symbol to generate a definition of
/// \param parent The parent of the symbol
/// \param indent A custom indentation to prepend
std::string print_definition(const DaedalusScript& scr,
                             const DaedalusSymbol& sym,
                             const DaedalusSymbol* parent,
                             std::string_view indent) {
	std::string def {};
	if (sym.is_member())
		return def;
	def += indent;
	if (sym.is_external())
		def += "extern ";

	if (sym.type() == DaedalusDataType::INSTANCE) {
		def += fmt::format("instance {}({})", sym.name(), (parent == nullptr ? "*ERR*" : parent->name()));

		if (!sym.is_const()) {
			def += ";";
		}
	} else if (sym.type() == DaedalusDataType::PROTOTYPE) {
		def += fmt::format("prototype {}({})", sym.name(), (parent == nullptr ? "*ERR*" : parent->name()));
	} else if (sym.type() == DaedalusDataType::CLASS) {
		def += fmt::format("class {} {{\n", sym.name());

		for (const auto& member : scr.symbols()) {
			if (member.parent() == sym.index() && member.is_member()) {
				auto name = member.name().substr(member.name().find('.') + 1);
				def += fmt::format("{}\tvar {} {}", indent, get_type_name(member.type()), name);
				if (member.count() > 1)
					def += fmt::format("[{}]", member.count());
				def += ";\n";
			}
		}

		def += fmt::format("{}}};", indent);
	} else if (sym.type() == DaedalusDataType::FUNCTION) {
		def += fmt::format("func {} {}(", get_type_name(sym.rtype()), sym.name());

		auto params = scr.find_parameters_for_function(&sym);
		unsigned count = 0;
		for (const auto& param : params) {
			if (count > 0)
				def += ", ";

			if (param->type() == DaedalusDataType::INSTANCE) {
				const auto* dt = scr.find_symbol_by_index(param->parent());

				if (dt == nullptr) {
					def += "var instance ";
				} else {
					def += fmt::format("var {} ", dt->name());
				}
			} else {
				def += fmt::format("var {} ", get_type_name(param->type()));
			}

			def += param->name().substr(param->name().find('.') + 1);
			count++;
		}

		def += ")";
	} else {
		if (sym.is_const()) {
			def += "const ";
		} else {
			def += "var ";
		}

		def += fmt::format("{} {}", get_type_name(sym.type()), sym.name());
		if (sym.count() > 1)
			def += fmt::format("[{}]", sym.count());

		if (sym.is_const())
			def += " = " + print_symbol_value(sym);

		def += ";";
	}

	return def;
}

/// \brief Print detailed information about a symbol
/// \param scr The script to reference
/// \param sym The symbol to print details of
void print_symbol_detailed(const DaedalusScript& scr, const DaedalusSymbol& sym) {
	const auto* parent = scr.find_symbol_by_index(sym.parent());

	fmt::print("{:0>8x} <{}>\n", sym.index(), sym.name());
	fmt::print("\tName: {}\n", sym.name());
	fmt::print("\tAddress: {:x}\n", sym.address());
	fmt::print("\tSize: {}\n", sym.count());
	fmt::print("\tType: {}\n", get_type_name(sym.type()));
	fmt::print("\tGenerated: {}\n", (sym.is_generated() ? "Yes" : "No"));
	fmt::print("\tFlags:\n");
	fmt::print("\t\tConst: {}\n", (sym.is_const() ? "Yes" : "No"));
	fmt::print("\t\tReturn: {}\n", (sym.has_return() ? "Yes" : "No"));
	fmt::print("\t\tMember: {}\n", (sym.is_member() ? "Yes" : "No"));
	fmt::print("\t\tExtern: {}\n", (sym.is_external() ? "Yes" : "No"));
	fmt::print("\t\tMerged: {}\n", (sym.is_merged() ? "Yes" : "No"));
	fmt::print("\tFile Index: {}\n", sym.file_index());
	fmt::print("\tLine Start: {}\n", sym.line_start());
	fmt::print("\tLine Count: {}\n", sym.line_count());
	fmt::print("\tChar Start: {}\n", sym.char_start());
	fmt::print("\tChar Count: {}\n", sym.char_count());

	if (parent != nullptr) {
		fmt::print("\tParent: {} ({:x})\n", parent->name(), parent->index());
	}

	if (sym.is_member()) {
		fmt::print("\tVariable Offset: {}\n", sym.offset_as_member());
	}

	if (sym.type() == DaedalusDataType::CLASS) {
		fmt::print("\tClass Size: {}\n", sym.class_size());
	}

	if (sym.is_const() && sym.type() == DaedalusDataType::FUNCTION) {
		fmt::print("\tReturn Type: {}\n", get_type_name(sym.rtype()));
	}

	if (!sym.is_member()) {
		fmt::print("\tDefinition:\n{}\n", print_definition(scr, sym, parent, "\t\t"));
	}
}

/// \brief Gets the one-char abbreviation of a DaedalusDataType
/// \param type The type to get the abbreviation of
/// \return The abbreviation
constexpr char get_type_abbrev(DaedalusDataType tp) {
	switch (tp) {
	case DaedalusDataType::VOID:
		return 'v';
	case DaedalusDataType::FLOAT:
		return 'f';
	case DaedalusDataType::INT:
		return 'i';
	case DaedalusDataType::STRING:
		return 's';
	case DaedalusDataType::CLASS:
		return 'C';
	case DaedalusDataType::FUNCTION:
		return 'F';
	case DaedalusDataType::PROTOTYPE:
		return 'P';
	case DaedalusDataType::INSTANCE:
		return 'I';
	}

	return '?';
}

/// \brief Print a string containing all the given symbols flags
/// \param sym The symbol to print the flags of
void print_symbol_flags(const DaedalusSymbol& sym) {
	fmt::print("{}", get_type_abbrev(sym.type()));

	PRINT_FLAG(sym.is_const(), 'c');
	PRINT_FLAG(sym.has_return(), 'r');
	PRINT_FLAG(sym.is_member(), 'm');
	PRINT_FLAG(sym.is_external(), 'e');
	PRINT_FLAG(sym.is_merged(), 'M');
	PRINT_FLAG(sym.is_generated(), 'g');
}

/// \brief Prints the name of the symbol's parent symbol if it exists
/// \param sym The symbol to print the parent of
/// \param scr The script to look up the parent in.
void print_symbol_parent(const DaedalusSymbol& sym, const DaedalusScript& scr) {
	if (sym.parent() != unset) {
		const auto* parent = scr.find_symbol_by_index(sym.parent());

		if (parent == nullptr) {
			fmt::print("{: <25}", "*ERR*");
		} else {
			fmt::print("{: <25}", parent->name());
		}
	} else {
		fmt::print("{: <25}", "*UND*");
	}
}

/// \brief Tests if a symbol matches the given filter
/// \param sym the symbol to test
/// \param include_filter Filter of flags to include
/// \param exclude_filter Filter of flags to exclude
/// \return `true` if the filters match, `false` otherwise.
bool symbol_matches_filter(const DaedalusSymbol& sym,
                           std::string_view include_filter,
                           std::string_view exclude_filter) {
	bool matches_include = true;
	bool matches_exclude = false;

	auto filter_matches = [&](std::string_view filter) {
		if ((sym.is_const() && SV_CONTAINS(filter, 'c')) || (sym.has_return() && SV_CONTAINS(filter, 'r')) ||
		    (sym.is_member() && SV_CONTAINS(filter, 'm')) || (sym.is_external() && SV_CONTAINS(filter, 'e')) ||
		    (sym.is_merged() && SV_CONTAINS(filter, 'M')) || (sym.is_generated() && SV_CONTAINS(filter, 'g'))) {
			return true;
		}

		if ((sym.type() == DaedalusDataType::VOID && SV_CONTAINS(filter, 'v')) ||
		    (sym.type() == DaedalusDataType::FLOAT && SV_CONTAINS(filter, 'f')) ||
		    (sym.type() == DaedalusDataType::INT && SV_CONTAINS(filter, 'i')) ||
		    (sym.type() == DaedalusDataType::STRING && SV_CONTAINS(filter, 's')) ||
		    (sym.type() == DaedalusDataType::CLASS && SV_CONTAINS(filter, 'C')) ||
		    (sym.type() == DaedalusDataType::FUNCTION && SV_CONTAINS(filter, 'F')) ||
		    (sym.type() == DaedalusDataType::PROTOTYPE && SV_CONTAINS(filter, 'P')) ||
		    (sym.type() == DaedalusDataType::INSTANCE && SV_CONTAINS(filter, 'I'))) {
			return true;
		}
		return false;
	};

	if (!include_filter.empty()) {
		matches_include = filter_matches(include_filter);
	}

	if (!exclude_filter.empty()) {
		matches_exclude = filter_matches(exclude_filter);
	}

	return matches_include && !matches_exclude;
}

/// \brief Print a list of symbols and their contents
/// \param scr The script to print the symbols of
/// \param include_filter Filter for symbols to include
/// \param exclude_filter Filter for symbols to exclude
void print_symbol_list(const DaedalusScript& scr,
                       std::string_view include_filter,
                       std::string_view exclude_filter,
                       std::string_view search) {
	fmt::print("Index    Flags   Parent                    Address  R Name\n");

	for (const auto& sym : scr.symbols()) {
		if (!symbol_matches_filter(sym, include_filter, exclude_filter) ||
		    (!search.empty() && sym.name().find(search) == std::string_view::npos)) {
			continue;
		}

		// Index
		fmt::print("{:0>8x} ", sym.index());

		// Flags
		print_symbol_flags(sym);
		fmt::print(" ");

		// Parent
		print_symbol_parent(sym, scr);
		fmt::print(" ");

		// Address
		fmt::print("{:0>8x} ", sym.address());

		// Return type
		if (sym.is_const() && sym.type() == DaedalusDataType::FUNCTION) {
			if (sym.has_return()) {
				fmt::print("{} ", get_type_abbrev(sym.rtype()));
			} else {
				fmt::print("v ");
			}
		} else {
			fmt::print("  ");
		}

		// Symbol name
		fmt::print("{}\n", sym.name());
	}
}

std::unordered_map<DaedalusOpcode, std::string_view> DAEDALUS_OPCODES {
    {DaedalusOpcode::ADD, "ADD"},
    {DaedalusOpcode::SUB, "SUB"},
    {DaedalusOpcode::MUL, "MUL"},
    {DaedalusOpcode::DIV, "DIV"},
    {DaedalusOpcode::MOD, "MOD"},
    {DaedalusOpcode::OR, "OR"},
    {DaedalusOpcode::ANDB, "ANDB"},
    {DaedalusOpcode::LT, "LT"},
    {DaedalusOpcode::GT, "GT"},
    {DaedalusOpcode::MOVI, "MOVI"},
    {DaedalusOpcode::ORR, "ORR"},
    {DaedalusOpcode::AND, "AND"},
    {DaedalusOpcode::LSL, "LSL"},
    {DaedalusOpcode::LSR, "LSR"},
    {DaedalusOpcode::LTE, "LTE"},
    {DaedalusOpcode::EQ, "EQ"},
    {DaedalusOpcode::NEQ, "NEQ"},
    {DaedalusOpcode::GTE, "GTE"},
    {DaedalusOpcode::ADDMOVI, "ADDMOVI"},
    {DaedalusOpcode::SUBMOVI, "SUBMOVI"},
    {DaedalusOpcode::MULMOVI, "MULMOVI"},
    {DaedalusOpcode::DIVMOVI, "DIVMOVI"},
    {DaedalusOpcode::PLUS, "PLUS"},
    {DaedalusOpcode::NEGATE, "NEGATE"},
    {DaedalusOpcode::NOT, "NOT"},
    {DaedalusOpcode::CMPL, "CMPL"},
    {DaedalusOpcode::NOP, "NOP"},
    {DaedalusOpcode::RSR, "RSR"},
    {DaedalusOpcode::BL, "BL"},
    {DaedalusOpcode::BE, "BE"},
    {DaedalusOpcode::PUSHI, "PUSHI"},
    {DaedalusOpcode::PUSHV, "PUSHV"},
    {DaedalusOpcode::PUSHVI, "PUSHVI"},
    {DaedalusOpcode::MOVS, "MOVS"},
    {DaedalusOpcode::MOVSS, "MOVSS"},
    {DaedalusOpcode::MOVVF, "MOVVF"},
    {DaedalusOpcode::MOVF, "MOVF"},
    {DaedalusOpcode::MOVVI, "MOVVI"},
    {DaedalusOpcode::B, "B"},
    {DaedalusOpcode::BZ, "BZ"},
    {DaedalusOpcode::GMOVI, "GMOVI"},
    {DaedalusOpcode::PUSHVV, "PUSHVV"},
};

std::string_view get_opcode_name(DaedalusOpcode code) {
	auto it = DAEDALUS_OPCODES.find(code);
	if (it != DAEDALUS_OPCODES.end()) return it->second;
	return "???";
}

/// \brief Prints the given instruction as raw bytes
/// \param i The instruction to print
void print_instruction_bytes(const DaedalusInstruction& i) {
	fmt::print("{:0>2x} ", (uint32_t) i.op);

	switch (i.op) {
	case DaedalusOpcode::BL:
	case DaedalusOpcode::BZ:
	case DaedalusOpcode::B:
		fmt::print("{:0>2x} {:0>2x} {:0>2x} {:0>2x}    ",
		           i.address & 0xFFU,
		           (i.address >> 8U) & 0xFFU,
		           (i.address >> 16U) & 0xFFU,
		           (i.address >> 24U) & 0xFFU);
		break;
	case DaedalusOpcode::PUSHI:
		fmt::print("{:0>2x} {:0>2x} {:0>2x} {:0>2x}    ",
		           i.immediate & 0xFFU,
		           (i.immediate >> 8U) & 0xFFU,
		           (i.immediate >> 16U) & 0xFFU,
		           (i.immediate >> 24U) & 0xFFU);
		break;
	case DaedalusOpcode::BE:
	case DaedalusOpcode::PUSHV:
	case DaedalusOpcode::PUSHVI:
	case DaedalusOpcode::GMOVI:
		fmt::print("{:0>2x} {:0>2x} {:0>2x} {:0>2x}    ",
		           i.symbol & 0xFFU,
		           (i.symbol >> 8U) & 0xFFU,
		           (i.symbol >> 16U) & 0xFFU,
		           (i.symbol >> 24U) & 0xFFU);
		break;
	case DaedalusOpcode::PUSHVV:
		fmt::print("{:0>2x} {:0>2x} {:0>2x} {:0>2x} {:0>2x} ",
		           i.symbol & 0xFFU,
		           (i.symbol >> 8U) & 0xFFU,
		           (i.symbol >> 16U) & 0xFFU,
		           (i.symbol >> 24U) & 0xFFU,
		           i.index);
		break;
	default:
		fmt::print("               ");
		break;
	}
}

/// \brief Prints the given instruction
/// \param scr The script to reference
/// \param i The instruction to print
void print_instruction(const DaedalusScript& scr, const DaedalusInstruction& i, bool instruction_only = false) {
	if (!instruction_only) {
		print_instruction_bytes(i);
		fmt::print(" ");
	}

	fmt::print("{}", get_opcode_name(i.op));

	switch (i.op) {
	case DaedalusOpcode::BL:
		fmt::print(" {:0>8x}", i.address);
		if (const auto* s = scr.find_symbol_by_address(i.address); s != nullptr && !instruction_only) {
			fmt::print(" ; <{}>", s->name());
		}
		break;
	case DaedalusOpcode::B:
	case DaedalusOpcode::BZ:
		fmt::print(" {:0>8x}", i.address);
		break;
	case DaedalusOpcode::PUSHV:
	case DaedalusOpcode::PUSHVI:
	case DaedalusOpcode::BE:
	case DaedalusOpcode::GMOVI:
		fmt::print(" {:0>8x}", i.symbol);

		if (const auto* s = scr.find_symbol_by_index(i.symbol); s != nullptr && !instruction_only) {
			fmt::print(" ; <{}>", s->name());

			if (has_constant_value(*s)) {
				fmt::print(" = {}", print_symbol_value(*s));
			}
		}
		break;
	case DaedalusOpcode::PUSHI:
		fmt::print(" {}", i.immediate);
		break;
	case DaedalusOpcode::PUSHVV:
		fmt::print(" {:0>8x} + {}", i.address, i.index);

		if (const auto* s = scr.find_symbol_by_index(i.symbol); s != nullptr && !instruction_only) {
			fmt::print(" ; <{}+{}>", s->name(), i.index);

			if (has_constant_value(*s)) {
				fmt::print(" = {}", print_symbol_value(*s, i.index));
			}
		}
		break;
	default:
		break;
	}
}

/// \brief Prints the disassembly of one symbol
/// \param scr The script to reference
/// \param sym The symbol to print the disassembly of
void print_assembly_of_symbol(const DaedalusScript& scr, const DaedalusSymbol& sym) {
	fmt::print("{:0>8x} <{}>:\n", sym.address(), sym.name());

	uint32_t pc = sym.address();
	uint32_t return_after = pc;
	DaedalusInstruction i;

	do {
		i = scr.instruction_at(pc);

		fmt::print("{: >8x}:\t", pc);
		print_instruction(scr, i);
		fmt::print("\n");

		if (i.op == DaedalusOpcode::BZ || i.op == DaedalusOpcode::B) {
			return_after = return_after > i.address ? return_after : i.address;
		}

		pc += i.size;
	} while (i.op != DaedalusOpcode::RSR || pc <= return_after);
}

/// \brief Prints the disassembly of every symbol found in the script
/// \param scr The script to disassemble
void print_assembly(const DaedalusScript& scr) {
	for (const auto& sym : scr.symbols()) {
		if (sym.type() == DaedalusDataType::PROTOTYPE || sym.type() == DaedalusDataType::INSTANCE ||
		    (sym.type() == DaedalusDataType::FUNCTION && !sym.is_external() && sym.is_const())) {
			print_assembly_of_symbol(scr, sym);
		}
	}
}

void find_usages(const DaedalusScript& scr, const DaedalusSymbol& sym) {
	uint32_t pc = 0;
	const DaedalusSymbol* current_function;

	while (pc < scr.size()) {
		auto inst = scr.instruction_at(pc);
		auto func_sym = scr.find_symbol_by_address(pc);

		if (func_sym != nullptr) {
			fmt::print("\r{: <100}\r", func_sym->name());
			current_function = func_sym;
		}

		switch (inst.op) {
		case DaedalusOpcode::ADDMOVI:
		case DaedalusOpcode::SUBMOVI:
		case DaedalusOpcode::MULMOVI:
		case DaedalusOpcode::DIVMOVI:
		case DaedalusOpcode::BL:
		case DaedalusOpcode::BE:
		case DaedalusOpcode::PUSHV:
		case DaedalusOpcode::PUSHVI:
		case DaedalusOpcode::MOVI:
		case DaedalusOpcode::MOVS:
		case DaedalusOpcode::MOVVF:
		case DaedalusOpcode::MOVF:
		case DaedalusOpcode::MOVVI:
		case DaedalusOpcode::PUSHVV:
			if (inst.symbol == sym.index() || (inst.op == DaedalusOpcode::BL && sym.address() == inst.address)) {
				fmt::print("\r[");
				print_instruction(scr, inst, true);
				fmt::print("] at {:0>8x} in {}\n", pc, current_function->name());
			}
			break;
		default:
			break;
		}

		pc += inst.size;
	}

	fmt::print("\r{: <100}\r", "");
}
