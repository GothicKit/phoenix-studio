// Copyright © 2022 Luis Michaelis <lmichaelis.all+dev@gmail.com>
// SPDX-License-Identifier: MIT
#include "decompiler.hh"

#include "fmt/format.h"
#include "phoenix/DaedalusScript.hh"
#include <set>
#include <vector>

using namespace phoenix;

static uint32_t current_instance = -1;
static const DaedalusSymbol* current_symbol = nullptr;
static std::set<uint32_t> current_symbol_params {};

struct stack_frame {
	DaedalusInstruction instr;
	uint32_t instance;
};

using local_storage = std::unordered_map<std::string, const DaedalusSymbol*>;

bool is_terminating_instruction(const DaedalusScript& script, const DaedalusInstruction& i) {
	switch (i.op) {
	case DaedalusOpcode::MOVI:
	case DaedalusOpcode::ADDMOVI:
	case DaedalusOpcode::SUBMOVI:
	case DaedalusOpcode::MULMOVI:
	case DaedalusOpcode::DIVMOVI:
	case DaedalusOpcode::RSR:
	case DaedalusOpcode::MOVS:
	case DaedalusOpcode::MOVSS:
	case DaedalusOpcode::MOVVF:
	case DaedalusOpcode::MOVF:
	case DaedalusOpcode::MOVVI:
	case DaedalusOpcode::B:
	case DaedalusOpcode::BZ:
		return true;
	case DaedalusOpcode::GMOVI:
		current_instance = i.symbol;
		return false;
	case DaedalusOpcode::BL:
		return !script.find_symbol_by_address(i.address)->has_return();
	case DaedalusOpcode::BE:
		return !script.find_symbol_by_index(i.symbol)->has_return();
	default:
		return false;
	}
}

stack_frame
extract_statement(const DaedalusScript& script, uint32_t& pointer, uint32_t end_ptr, std::vector<stack_frame>& stack) {
	DaedalusInstruction instr = script.instruction_at(pointer);
	pointer += instr.size;

	while (!is_terminating_instruction(script, instr) && pointer < end_ptr) {
		if (instr.op != DaedalusOpcode::NOP && instr.op != DaedalusOpcode::GMOVI) {
			stack.push_back({instr, current_instance});
		}

		instr = script.instruction_at(pointer);
		pointer += instr.size;
	}

	return {instr, current_instance};
}

static std::unordered_map<DaedalusOpcode, std::string> OPCODE_STR {
    {DaedalusOpcode::ADD, "+"},      {DaedalusOpcode::SUB, "-"},      {DaedalusOpcode::MUL, "*"},
    {DaedalusOpcode::DIV, "/"},      {DaedalusOpcode::MOD, "%"},      {DaedalusOpcode::OR, "|"},
    {DaedalusOpcode::ANDB, "&"},     {DaedalusOpcode::LT, "<"},       {DaedalusOpcode::GT, ">"},
    {DaedalusOpcode::ORR, "||"},     {DaedalusOpcode::AND, "&&"},     {DaedalusOpcode::LSL, "<<"},
    {DaedalusOpcode::LSR, ">>"},     {DaedalusOpcode::LTE, "<="},     {DaedalusOpcode::EQ, "=="},
    {DaedalusOpcode::NEQ, "!="},     {DaedalusOpcode::GTE, ">="},     {DaedalusOpcode::PLUS, "+"},
    {DaedalusOpcode::NEGATE, "-"},   {DaedalusOpcode::NOT, "!"},      {DaedalusOpcode::CMPL, "~"},
    {DaedalusOpcode::ADDMOVI, "+="}, {DaedalusOpcode::SUBMOVI, "-="}, {DaedalusOpcode::MULMOVI, "*="},
    {DaedalusOpcode::DIVMOVI, "/="},
};

std::string decompile_statement(const DaedalusScript& script,
                                const stack_frame& stmt,
                                std::vector<stack_frame>& stack,
                                local_storage& locals) {
	switch (stmt.instr.op) {
	case DaedalusOpcode::ADD:
	case DaedalusOpcode::SUB:
	case DaedalusOpcode::MUL:
	case DaedalusOpcode::DIV:
	case DaedalusOpcode::MOD:
	case DaedalusOpcode::OR:
	case DaedalusOpcode::ANDB:
	case DaedalusOpcode::LT:
	case DaedalusOpcode::GT:
	case DaedalusOpcode::ORR:
	case DaedalusOpcode::AND:
	case DaedalusOpcode::LSL:
	case DaedalusOpcode::LSR:
	case DaedalusOpcode::LTE:
	case DaedalusOpcode::EQ:
	case DaedalusOpcode::NEQ:
	case DaedalusOpcode::GTE: {
		stack_frame a_instr {DaedalusOpcode::PUSHI, 0, 0, 0, 0, 0};
		if (!stack.empty()) {
			a_instr = stack.back();
			stack.pop_back();
		}

		auto a = decompile_statement(script, a_instr, stack, locals);

		stack_frame b_instr {DaedalusOpcode::PUSHI, 0, 0, 0, 0, 0};
		if (!stack.empty()) {
			b_instr = stack.back();
			stack.pop_back();
		}

		auto b = decompile_statement(script, b_instr, stack, locals);
		return fmt::format("({}) {} ({})", a, OPCODE_STR[stmt.instr.op], b);
	}
	case DaedalusOpcode::ADDMOVI:
	case DaedalusOpcode::SUBMOVI:
	case DaedalusOpcode::MULMOVI:
	case DaedalusOpcode::DIVMOVI: {
		stack_frame ref_instr {DaedalusOpcode::PUSHI, 0, 0, 0, 0, 0};
		if (!stack.empty()) {
			ref_instr = stack.back();
			stack.pop_back();
		}

		auto ref = decompile_statement(script, ref_instr, stack, locals);

		stack_frame a_instr {DaedalusOpcode::PUSHI, 0, 0, 0, 0, 0};
		if (!stack.empty()) {
			a_instr = stack.back();
			stack.pop_back();
		}

		auto a = decompile_statement(script, a_instr, stack, locals);
		return fmt::format("{} {} {}", ref, OPCODE_STR[stmt.instr.op], a);
	}
	case DaedalusOpcode::PLUS:
	case DaedalusOpcode::NEGATE:
	case DaedalusOpcode::NOT:
	case DaedalusOpcode::CMPL: {
		stack_frame a_instr {DaedalusOpcode::PUSHI, 0, 0, 0, 0, 0};
		if (!stack.empty()) {
			a_instr = stack.back();
			stack.pop_back();
		}
		auto a = decompile_statement(script, a_instr, stack, locals);
		return fmt::format("{}({})", OPCODE_STR[stmt.instr.op], a);
	}
	case DaedalusOpcode::NOP:
		return "";
	case DaedalusOpcode::RSR: {
		if (!current_symbol->has_return()) {
			return "return";
		}

		if (stack.empty()) {
			return "return 0 /* !broken stack! */";
		}

		auto a_instr = stack.back();
		stack.pop_back();
		auto a = decompile_statement(script, a_instr, stack, locals);
		return fmt::format("return {}", a);
	}
	case DaedalusOpcode::BL: {
		auto* sym = script.find_symbol_by_address(stmt.instr.address);
		auto params = script.find_parameters_for_function(sym);
		std::string call;

		for (unsigned i = params.size(); i > 0; --i) {
			stack_frame a_instr {DaedalusOpcode::PUSHI, 0, 0, 0, 0, 0};
			if (!stack.empty()) {
				a_instr = stack.back();
				stack.pop_back();
			}

			if (i == 0 || call.empty()) {
				call = fmt::format("{}{}", decompile_statement(script, a_instr, stack, locals), call);
			} else {
				call = fmt::format("{}, {}", decompile_statement(script, a_instr, stack, locals), call);
			}
		}

		return fmt::format("{}({})", sym->name(), call);
	}
	case DaedalusOpcode::BE: {
		auto* sym = script.find_symbol_by_index(stmt.instr.symbol);
		auto params = script.find_parameters_for_function(sym);
		std::string call;

		for (unsigned i = params.size(); i > 0; --i) {
			stack_frame a_instr {DaedalusOpcode::PUSHI, 0, 0, 0, 0, 0};
			if (!stack.empty()) {
				a_instr = stack.back();
				stack.pop_back();
			}

			if (i == 0 || call.empty()) {
				call = fmt::format("{}{}", decompile_statement(script, a_instr, stack, locals), call);
			} else {
				call = fmt::format("{}, {}", decompile_statement(script, a_instr, stack, locals), call);
			}
		}

		return fmt::format("{}({})", sym->name(), call);
	}
	case DaedalusOpcode::PUSHI: {
		if (stmt.instr.immediate > 1000) {
			return std::to_string(stmt.instr.immediate); // TODO: Optimize obvious bitshifts and multiples of 16
		}
		return std::to_string(stmt.instr.immediate);
	}
	case DaedalusOpcode::PUSHV:
	case DaedalusOpcode::PUSHVI: {
		auto sym = script.find_symbol_by_index(stmt.instr.symbol);

		if (sym->is_generated() && sym->type() == DaedalusDataType::STRING) {
			return fmt::format("\"{}\"", sym->get_string());
		}

		std::string sym_name;
		if (sym->is_member() &&
		    !((current_symbol->type() == DaedalusDataType::INSTANCE ||
		       current_symbol->type() == DaedalusDataType::PROTOTYPE) &&
		      current_instance == current_symbol->index())) {
			auto inst_sym = script.find_symbol_by_index(stmt.instance);
			if (inst_sym == nullptr) {
				sym_name = "???" + sym->name().substr(sym->name().find('.'));
			} else {
				if (inst_sym->name().find(current_symbol->name()) == 0) {
					sym_name = inst_sym->name().substr(inst_sym->name().find('.') + 1) +
					    sym->name().substr(sym->name().find('.'));
				} else {
					sym_name = inst_sym->name() + sym->name().substr(sym->name().find('.'));
				}
			}
		} else {
			auto dot = sym->name().find('.');

			if (dot != -1) {
				sym_name = sym->name().substr(dot + 1);

				if (sym->name().substr(0, dot) == current_symbol->name() &&
				    current_symbol_params.find(sym->index()) == current_symbol_params.end()) {
					if (locals.find(sym_name) == locals.end()) {
						locals[sym_name] = sym;
					}
				}
			} else {
				sym_name = sym->name();
			}
		}

		if (sym->count() > 1) {
			return fmt::format("{}[0]", sym_name);
		}

		return sym_name;
	}
	case DaedalusOpcode::PUSHVV: {
		auto sym = script.find_symbol_by_index(stmt.instr.symbol);
		std::string sym_name;

		if (sym->is_member() &&
		    !((current_symbol->type() == DaedalusDataType::INSTANCE ||
		       current_symbol->type() == DaedalusDataType::PROTOTYPE) &&
		      current_instance == current_symbol->index())) {
			auto inst_sym = script.find_symbol_by_index(stmt.instance);
			if (inst_sym == nullptr) {
				sym_name = "???" + sym->name().substr(sym->name().find('.'));
			} else {
				if (inst_sym->name().find(current_symbol->name()) == 0) {
					sym_name = inst_sym->name().substr(inst_sym->name().find('.') + 1) +
					    sym->name().substr(sym->name().find('.'));
				} else {
					sym_name = inst_sym->name() + sym->name().substr(sym->name().find('.'));
				}
			}
		} else {
			auto dot = sym->name().find('.');
			if (dot != -1) {
				sym_name = sym->name().substr(dot + 1);

				if (sym->name().substr(0, dot) == current_symbol->name() &&
				    current_symbol_params.find(sym->index()) == current_symbol_params.end()) {
					if (locals.find(sym_name) == locals.end()) {
						locals[sym_name] = sym;
					}
				}
			} else {
				sym_name = sym->name();
			}
		}

		return fmt::format("{}[{}]", sym_name, stmt.instr.index);
	}
	case DaedalusOpcode::MOVI:
	case DaedalusOpcode::MOVS:
	case DaedalusOpcode::MOVSS:
	case DaedalusOpcode::MOVVF:
	case DaedalusOpcode::MOVF:
	case DaedalusOpcode::MOVVI: {
		stack_frame a_instr {DaedalusOpcode::PUSHI, 0, 0, 0, 0, 0};
		if (!stack.empty()) {
			a_instr = stack.back();
			stack.pop_back();
		}

		auto a = decompile_statement(script, a_instr, stack, locals);
		auto sym = script.find_symbol_by_index(a_instr.instr.symbol);

		stack_frame b_instr {DaedalusOpcode::PUSHI, 0, 0, 0, 0, 0};
		if (!stack.empty()) {
			b_instr = stack.back();
			stack.pop_back();
		}
		auto b = decompile_statement(script, b_instr, stack, locals);

		if (sym->type() == DaedalusDataType::FLOAT) {
			return fmt::format("{} = {}", a, reinterpret_cast<float&>(b_instr.instr.immediate));
		} else if (sym->type() == DaedalusDataType::FUNCTION) {
			return fmt::format("{} = {}", a, script.find_symbol_by_index(b_instr.instr.immediate)->name());
		}

		return fmt::format("{} = {}", a, b);
	}
	default:
		return fmt::format("/* set_instance({}) */", stmt.instr.symbol);
	}
}

std::pair<std::string, std::uint32_t> decompile_block(const DaedalusScript& script,
                                                      local_storage& locals,
                                                      int indent,
                                                      uint32_t pointer,
                                                      int64_t end_ptr = -1,
                                                      std::vector<stack_frame> stack = {}) {
	std::string code {};
	stack_frame stmt;

	do {
		stmt = extract_statement(script, pointer, end_ptr, stack);

		if (stmt.instr.op == DaedalusOpcode::BZ) {
			stack_frame real_stmt {DaedalusOpcode::PUSHI, 0, 0, 0, 0, 0};
			if (!stack.empty()) {
				real_stmt = stack.back();
				stack.pop_back();
			}

			code +=
			    fmt::format("{: >{}}if ({}) {{\n", "", indent, decompile_statement(script, real_stmt, stack, locals));

			if (pointer == stmt.instr.address) {
				code += fmt::format("{: >{}}}};\n", "", indent);
				continue;
			}

			auto [if_block, next_branch] = decompile_block(script, locals, indent + 4, pointer, stmt.instr.address);
			pointer = stmt.instr.address;

			code += if_block + fmt::format("{: >{}}}}", "", indent);

			// if the `br` at the end of the `if`-statement jumps to after where the initial `brz` jumps,
			// this is either an `else if`-statement or and `else`-statement
			while (next_branch > stmt.instr.address && pointer < end_ptr) {
				auto new_stmt = extract_statement(script, pointer, end_ptr, stack);

				if (new_stmt.instr.op == DaedalusOpcode::BZ) {
					// else-if block
					if (!stack.empty()) {
						real_stmt = stack.back();
						stack.pop_back();
					}

					code += fmt::format(" else if ({}) {{\n", decompile_statement(script, real_stmt, stack, locals));

					if (pointer == new_stmt.instr.address) {
						code += fmt::format("{: >{}}}};\n", "", indent);
						continue;
					}

					auto [else_if_block, else_if_next_branch] =
					    decompile_block(script, locals, indent + 4, pointer, new_stmt.instr.address);
					next_branch = else_if_next_branch;
					pointer = new_stmt.instr.address;

					code += else_if_block + fmt::format("{: >{}}}}", "", indent);
					stmt = new_stmt;
				} else {
					// else block
					code += fmt::format(" else {{\n");

					if (pointer == new_stmt.instr.address) {
						code += fmt::format("{: >{}}}};\n", "", indent);
						break;
					}

					code += fmt::format("{: >{}}{};\n",
					                    "",
					                    indent + 4,
					                    decompile_statement(script, new_stmt, stack, locals));

					if (pointer < next_branch) {
						auto [else_block, else_next_branch] =
						    decompile_block(script, locals, indent + 4, pointer, next_branch);
						next_branch = else_next_branch;
						pointer = else_next_branch;
						code += else_block;
					}

					code += fmt::format("{: >{}}}}", "", indent);
					stmt = new_stmt;

					// No additional `else`-blocks may follow!
					break;
				}
			}

			code += ";\n";
		} else if (stmt.instr.op == DaedalusOpcode::B) {
			return {code, stmt.instr.address};
		} else {
			auto s = decompile_statement(script, stmt, stack, locals);

			if (!stack.empty()) {
				auto unused_return = stack.back();
				stack.pop_back();

				code +=
				    fmt::format("{: >{}}{};\n", "", indent, decompile_statement(script, unused_return, stack, locals));
			}

			if (!s.empty()) {
				code += fmt::format("{: >{}}{};\n", "", indent, s);
			}
		}
	} while (stmt.instr.op != DaedalusOpcode::RSR && pointer != end_ptr);

	return {code, pointer};
}

std::string decompile(const phoenix::DaedalusScript& script, const phoenix::DaedalusSymbol& sym, int indent) {
	if (sym.is_external()) {
		throw;
	}

	current_symbol = &sym;
	current_instance =
	    (sym.type() == DaedalusDataType::INSTANCE || sym.type() == DaedalusDataType::PROTOTYPE) ? sym.index() : unset;
	std::vector<stack_frame> stack {};
	local_storage locals {};
	auto params = script.find_parameters_for_function(&sym);

	current_symbol_params.clear();
	for (auto* param : params) {
		current_symbol_params.insert(param->index());
	}

	auto result = decompile_block(script, locals, indent, sym.address() + 6 * params.size(), -1, stack).first;

	std::string local_definitions {};
	for (auto& local : locals) {
		if (local.second->type() == phoenix::DaedalusDataType::INSTANCE) {
			local_definitions += fmt::format("{: >{}}var {} {};\n",
			                                 "",
			                                 indent,
			                                 script.find_symbol_by_index(local.second->parent())->name(),
			                                 local.first);
		} else {
			local_definitions +=
			    fmt::format("{: >{}}var {} {};\n", "", indent, get_type_name(local.second->type()), local.first);
		}
	}

	// If the function does not return anything, remove the last return
	if (!sym.has_return()) {
		result.erase(result.rfind("return")).erase(result.rfind('\n') + 1);
	}

	return local_definitions + result;
}
