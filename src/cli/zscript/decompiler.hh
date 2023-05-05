// Copyright © 2022 Luis Michaelis <lmichaelis.all+dev@gmail.com>
// SPDX-License-Identifier: MIT
#pragma once
#include <phoenix/DaedalusScript.hh>

#include <string>

constexpr std::string_view get_type_name(phoenix::DaedalusDataType tp) {
	using phoenix::DaedalusDataType;
	switch (tp) {
	case DaedalusDataType::VOID:
		return "void";
	case DaedalusDataType::FLOAT:
		return "float";
	case DaedalusDataType::INT:
		return "int";
	case DaedalusDataType::STRING:
		return "string";
	case DaedalusDataType::CLASS:
		return "class";
	case DaedalusDataType::FUNCTION:
		return "func";
	case DaedalusDataType::PROTOTYPE:
		return "prototype";
	case DaedalusDataType::INSTANCE:
		return "instance";
	default:
		return "*ERR*";
	}
}

std::string decompile(const phoenix::DaedalusScript& script, const phoenix::DaedalusSymbol& sym, int indent = 0);
