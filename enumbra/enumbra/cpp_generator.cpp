#include "cpp_generator.h"
#include <sstream>

using namespace enumbra;
using namespace enumbra::cpp;

std::string cpp_generator::generate_cpp_output(const enumbra_config& cfg, const enumbra::enum_definition& enum_def)
{
	auto& cpp_config = cfg.cpp_config;

	std::stringstream output;

	LF = (cpp_config.line_ending_style == cpp::LineEndingStyle::LF) ? "\n" : "\r\n";

	// PREAMBLE
	for (const auto& line : cpp_config.preamble_text) {
		output << line << LF;
	}

	// INCLUDE GUARD
	switch (cpp_config.include_guard_style)
	{
	case enumbra::cpp::IncludeGuardStyle::None:
		break;
	case enumbra::cpp::IncludeGuardStyle::PragmaOnce:
		output << "#pragma once" << LF << LF;
		break;
	case enumbra::cpp::IncludeGuardStyle::CStyle:
	{
		std::string def = "ENUMBRA_" + enum_def.name + "_H";
		output << "#ifndef " << def << LF << "#define " << def << LF << LF;
		break;
	}
	default:
		throw std::runtime_error("include_guard_wrap: Invalid IncludeGuardStyle");
	}

	// INCLUDES
	if (cpp_config.use_cstdint) {
		output << "#include <cstdint>" << LF;
	}

	if (cpp_config.string_table_layout != StringTableLayout::None) {
		if (cpp_config.string_table_type == StringTableType::ConstexprStringView || cpp_config.string_table_type == StringTableType::ConstexprWStringView)
		{
			output << "#include <string_view>" << LF;
		}
	}

	for (const auto& inc : cpp_config.additional_includes) {
		output << "#include " << inc << LF;
	}
	output << LF;

	// START NAMESPACE
	for (const auto& ns : cpp_config.output_namespace) {
		output << "namespace " << ns << " {" << LF;
	}

	// ENUM DEFINITIONS
	for (const auto& e : enum_def.values) {
		e.
	}




	// END NAMESPACE
	for (auto& ns : cpp_config.output_namespace) {
		output << "}" << LF;
	}

	// END INCLUDE GUARD
	if (cpp_config.include_guard_style == IncludeGuardStyle::CStyle) {
		output << "#endif" << LF;
	}

	return output.str();
}
