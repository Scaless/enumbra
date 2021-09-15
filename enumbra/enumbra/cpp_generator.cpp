#include "cpp_generator.h"
#include <sstream>
#include <vector>
#include <algorithm>


using namespace enumbra;
using namespace enumbra::cpp;

// Seriously C++?
std::string to_upper(const std::string& str)
{
	std::string strcopy = str;
	std::transform(strcopy.begin(), strcopy.end(), strcopy.begin(), ::toupper);
	return strcopy;
}

std::string cpp_generator::generate_cpp_output(const enumbra_config& cfg, const enumbra::enum_meta_config& enum_meta)
{
	auto& cpp = cfg.cpp_config;

	std::stringstream output;

	// Setting up re-usable tokens
	LF = (cpp.line_ending_style == cpp::LineEndingStyle::LF) ? "\n" : "\r\n";
	TAB = cpp.output_tab_characters;
	const std::string def_macro = "ENUMBRA_" + to_upper(enum_meta.block_name) + "_H";

	// PREAMBLE
	for (const auto& line : cpp.preamble_text) {
		output << line << LF;
	}

	// INCLUDE GUARD
	switch (cpp.include_guard_style)
	{
	case enumbra::cpp::IncludeGuardStyle::None:
		break;
	case enumbra::cpp::IncludeGuardStyle::PragmaOnce:
		output << "#pragma once" << LF << LF;
		break;
	case enumbra::cpp::IncludeGuardStyle::CStyle:
	{
		output << "#ifndef " << def_macro << LF << "#define " << def_macro << LF << LF;
		break;
	}
	default:
		throw std::runtime_error("include_guard_wrap: Invalid IncludeGuardStyle");
	}

	// INCLUDES
	if (cpp.use_cstdint) {
		output << "#include <cstdint>" << LF;
	}

	if (cpp.string_table_layout != StringTableLayout::None) {
		if (cpp.string_table_type == StringTableType::ConstexprStringView || cpp.string_table_type == StringTableType::ConstexprWStringView)
		{
			output << "#include <string_view>" << LF;
		}
	}

	for (const auto& inc : cpp.additional_includes) {
		output << "#include " << inc << LF;
	}
	output << LF;

	// START CONFIG NAMESPACE
	for (const auto& ns : cpp.output_namespace) {
		output << "namespace " << ns << " {" << LF;
	}
	output << LF;

	// ENUM DEFINITIONS
	for (const auto& e : enum_meta.enum_definitions) {
		output << "enum class " << e.name << " : " << cpp.get_size_type_from_index(e.size_type_index).generated_name << " {" << LF;
		for (const auto& v : e.values) {
			output << TAB << v.name << " = " << v.value << LF;
		}
		output << "}" << LF;
	}

	// END CONFIG NAMESPACE
	output << LF;
	for(auto& ns = cpp.output_namespace.rbegin(); ns != cpp.output_namespace.rend(); ++ns) {
		output << "} // " << *ns << LF;
	}

	// END INCLUDE GUARD
	if (cpp.include_guard_style == IncludeGuardStyle::CStyle) {
		output << "#endif // " << def_macro << LF;
	}

	return output.str();
}

std::vector<cpp_enum_generated> cpp_generator::generate_enums(const cpp_enum_config_final& cfg)
{
	throw std::logic_error("Unimplemented");
}
