// Copyright © 2022 Luis Michaelis <lmichaelis.all+dev@gmail.com>
// SPDX-License-Identifier: MIT
#pragma once
#include "phoenix/script.hh"

#include <string>

constexpr std::string_view get_type_name(phoenix::datatype tp) {
	using phoenix::datatype;
	switch (tp) {
	case datatype::void_:
		return "void";
	case datatype::float_:
		return "float";
	case datatype::integer:
		return "int";
	case datatype::string:
		return "string";
	case datatype::class_:
		return "class";
	case datatype::function:
		return "func";
	case datatype::prototype:
		return "prototype";
	case datatype::instance:
		return "instance";
	default:
		return "*ERR*";
	}
}

std::string decompile(const phoenix::script& script, const phoenix::symbol& sym, int indent = 0);
