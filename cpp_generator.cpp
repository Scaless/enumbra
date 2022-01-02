#include "cpp_generator.h"
#include <sstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <set>
#include <fmt/format.h>
#include <locale>

using namespace enumbra;
using namespace enumbra::cpp;

std::string to_upper(const std::string& str)
{
	std::string strcopy = str;
	for (auto& c : strcopy) {
		c = std::toupper(c, std::locale("en_US.utf8"));
	}
	return strcopy;
}

const int64_t get_flags_enum_value(const FlagsEnumDefaultValueStyle& style, const enum_definition& definition)
{
	switch (style)
	{
	case FlagsEnumDefaultValueStyle::Zero: return 0;
	case FlagsEnumDefaultValueStyle::Min:
	{
		auto m = std::min_element(definition.values.begin(), definition.values.end(),
			[](const enum_entry& lhs, const enum_entry& rhs) { return lhs.value < rhs.value; });
		if (m != definition.values.end())
		{
			return m->value;
		}
		else
		{
			throw std::logic_error("get_flags_enum_value: FlagsEnumDefaultValueStyle::Min failed somehow.");
		}
		break;
	}
	case FlagsEnumDefaultValueStyle::Max:
	{
		auto m = std::max_element(definition.values.begin(), definition.values.end(),
			[](const enum_entry& lhs, const enum_entry& rhs) { return lhs.value < rhs.value; });
		if (m != definition.values.end())
		{
			return m->value;
		}
		else
		{
			throw std::logic_error("get_flags_enum_value: FlagsEnumDefaultValueStyle::Max failed somehow.");
		}
		break;
	}
	case FlagsEnumDefaultValueStyle::UsedBitsSet:
	{
		uint64_t bits = 0;
		for (auto& v : definition.values)
		{
			bits |= v.value;
		}
		return int64_t(bits);
		break;
	}
	case FlagsEnumDefaultValueStyle::First:
	{
		return definition.values.front().value;
		break;
	}
	case FlagsEnumDefaultValueStyle::Last:
	{
		return definition.values.back().value;
		break;
	}
	default:
		throw std::logic_error("get_flags_enum_value: Invalid FlagsEnumDefaultValueStyle");
	}
}

const enum_entry& get_value_enum_entry(const ValueEnumDefaultValueStyle& style, const enum_definition& definition)
{
	switch (style)
	{
	case ValueEnumDefaultValueStyle::Min:
	{
		auto m = std::min_element(definition.values.begin(), definition.values.end(),
			[](const enum_entry& lhs, const enum_entry& rhs) { return lhs.value < rhs.value; });
		if (m != definition.values.end())
		{
			return *m;
		}
		else
		{
			throw std::logic_error("get_value_enum_default_value_style_string: ValueEnumDefaultValueStyle::Min failed somehow.");
		}
		break;
	}
	case ValueEnumDefaultValueStyle::Max:
	{
		auto m = std::max_element(definition.values.begin(), definition.values.end(),
			[](const enum_entry& lhs, const enum_entry& rhs) { return lhs.value < rhs.value; });
		if (m != definition.values.end())
		{
			return *m;
		}
		else
		{
			throw std::logic_error("get_value_enum_default_value_style_string: ValueEnumDefaultValueStyle::Max failed somehow.");
		}
		break;
	}
	case ValueEnumDefaultValueStyle::First:
		return definition.values.front();
	case ValueEnumDefaultValueStyle::Last:
		return definition.values.back();
	default:
		throw std::logic_error("value_enum_default_value_style: Invalid ValueEnumDefaultValueStyle");
	}
}

// Log2 of unsigned int
constexpr uint64_t log_2_unsigned(uint64_t x)
{
	if (x == 0)
	{
		return 0;
	}

	int targetlevel = 0;
	while (x >>= 1)
		++targetlevel;
	return targetlevel;
}

constexpr uint64_t unsigned_bits_required(uint64_t x)
{
	return log_2_unsigned(x) + 1;
}

// Enum names must be unique across value and flag enums
bool enum_meta_has_unique_enum_names(const enumbra::enum_meta_config& enum_meta)
{
	std::set<std::string> seen_names;
	for (auto& v : enum_meta.value_enum_definitions)
	{
		auto seen = seen_names.find(v.name);
		if (seen != seen_names.end())
		{
			throw std::logic_error("enum_meta_has_unique_enum_names: Value-Enum name is not unique (name = " + *seen + ")");
		}
		seen_names.insert(v.name);
	}
	for (auto& v : enum_meta.flag_enum_definitions)
	{
		auto seen = seen_names.find(v.name);
		if (seen != seen_names.end())
		{
			throw std::logic_error("enum_meta_has_unique_enum_names: Flags-Enum name is not unique (name = " + *seen + ")");
		}
		seen_names.insert(v.name);
	}
	return true;
}

bool is_value_set_contiguous(const std::set<int64_t> values)
{
	int64_t value = *values.begin();
	bool skip_first = true;
	for (auto& u : values)
	{
		if (skip_first) {
			skip_first = false;
			continue;
		}
		if (u != (value + 1))
		{
			return false;
		}
		value++;
	}
	return true;
}

bool is_flags_set_contiguous(const std::set<int64_t> flags)
{
	int64_t check_bit = unsigned_bits_required(*flags.begin());
	bool skip_first = true;
	for (auto& u : flags)
	{
		if (skip_first) {
			skip_first = false;
			continue;
		}
		if (u != (1LL << check_bit))
		{
			return false;
		}
		check_bit++;
	}
	return true;
}

const std::string& cpp_generator::generate_cpp_output(const enumbra_config& cfg, const enumbra::enum_meta_config& enum_meta)
{
	auto& cpp = cfg.cpp_config;
	Output.clear();

	// Precondition checks
	// 1. Enum names must be unique
	enum_meta_has_unique_enum_names(enum_meta);

	// Setting up re-usable tokens
	LF = (cpp.line_ending_style == cpp::LineEndingStyle::LF) ? "\n" : "\r\n";
	TAB = cpp.output_tab_characters;
	const std::string def_macro = fmt::format("ENUMBRA_{}_H", to_upper(enum_meta.block_name));

	// Enumbra pre-preamble
	std::time_t time_point = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	char time_buf[255];
	ctime_s(time_buf, 255, &time_point);
	write_line("// THIS CODE WAS GENERATED BY A TOOL (haha)");
	write_line("// Direct your feedback and monetary donations to: https://github.com/Scaless/enumbra");
	write_line("// It is highly recommended to not make manual edits to this file, as they will be overwritten");
	write_line("// when the file is re-generated. But do what you want, I'm a tool not a cop.");
	write_line("// Generated by enumbra v{} on {}", kEnumbraVersion, time_buf);

	// Custom preamble
	for (const auto& line : cpp.preamble_text) {
		write_line(line);
	}
	if (cpp.preamble_text.size() == 0) {
		write_line("// Hey! You don't have any preamble_text set. If you have a license you want to apply to your");
		write_line("// generated code, you should edit your enumbra_config.toml file!");
	}
	write_linefeed();

	// INCLUDE GUARD
	switch (cpp.include_guard_style)
	{
	case enumbra::cpp::IncludeGuardStyle::PragmaOnce:
		write_line("#pragma once");
		write_linefeed();
		break;
	case enumbra::cpp::IncludeGuardStyle::CStyle:
		write_line("#ifndef {0}", def_macro);
		write_line("#define {0}", def_macro);
		write_linefeed();
		break;
	}

	// INCLUDES
	write_line("#include <array>");
	for (const auto& inc : cpp.additional_includes) {
		write_line("#include {}", inc);
	}
	write_linefeed();

	// REQUIRED MACROS
	{
		const int enumbra_required_macros_version = 2;
		std::vector<const char*> macro_strings = {
			{ "#if !defined(ENUMBRA_REQUIRED_MACROS_VERSION)" },
			{ "#define ENUMBRA_REQUIRED_MACROS_VERSION {1}" },
			{ "" },
			{ "// Find out what language version we're using"},
			{ "#if (_MSVC_LANG >= 202002L) || (__cplusplus >= 202002L)"},
			{ "#define ENUMBRA_CPP_VERSION 20"},
			{ "#elif (_MSVC_LANG >= 201703L) || (__cplusplus >= 201703L)"},
			{ "#define ENUMBRA_CPP_VERSION 17"},
			{ "#elif (_MSVC_LANG >= 201402L) || (__cplusplus >= 201402L)"},
			{ "#define ENUMBRA_CPP_VERSION 14"},
			{ "#elif (_MSVC_LANG >= 201103L) || (__cplusplus >= 201103L)"},
			{ "#define ENUMBRA_CPP_VERSION 11"},
			{ "#else"},
			{ "#error enumbra generated headers require a C++11 or higher compiler."},
			{ "#endif"},
			{ "" },
			{"// Non-const constexpr functions were added in C++14"},
			{"#if __cpp_constexpr >= 201304L"},
			{"#define ENUMBRA_CONSTEXPR_NONCONSTFUNC constexpr"},
			{"#else"},
			{"#define ENUMBRA_CONSTEXPR_NONCONSTFUNC inline"},
			{"#endif"},
			{ "" },
			
			{ "" },
			{ "#else // check existing version supported" },
			{ "#if (ENUMBRA_REQUIRED_MACROS_VERSION + 0) == 0" },
			{ "#error ENUMBRA_REQUIRED_MACROS_VERSION has been defined without a proper version number. Check your build system." },
			{ "#elif (ENUMBRA_REQUIRED_MACROS_VERSION + 0) < {1}" },
			{ "#error An included header was generated using a newer version of enumbra. Regenerate your headers using the same version." },
			{ "#elif (ENUMBRA_REQUIRED_MACROS_VERSION + 0) > {1}" },
			{ "#error An included header was generated using an older version of enumbra. Regenerate your headers using the same version." },
			{ "#endif // end check existing version supported"},
			{ "#endif // ENUMBRA_REQUIRED_MACROS_VERSION" },
		};
		for (auto& str : macro_strings) {
			write_line(str, TAB, enumbra_required_macros_version);
		}
		write_linefeed();
	}

	// MACRO DEFINITIONS
	if (cfg.cpp_config.enumbra_macros)
	{
		const int enumbra_macros_version = 2;
		std::vector<const char*> macro_strings = {
			{ "#if !defined(ENUMBRA_OPTIONAL_MACROS_VERSION)" },
			{ "#define ENUMBRA_OPTIONAL_MACROS_VERSION {1}" },
			{ "" },
			{ "// Bit field storage helper" },
			{ "#define ENUMBRA_PACK(Enum, Name) Enum::Value Name : Enum::bits_required_storage();" },
			{ "" },
			{ "#if ENUMBRA_CPP_VERSION >= 20" },
			{ "// Bit field storage helper with type-checked member initialization" },
			{ "#define ENUMBRA_PACK_INIT(Enum, Name, InitValue) Enum::Value Name : Enum::bits_required_storage() {{ InitValue }}; \\" },
			{ "{0}static_assert(is_enumbra_type(InitValue), \"InitValue passed to ENUMBRA_PACK_INIT is not a valid enumbra type.\");" },
			{ "// Bit field storage helper with default value initialization" },
			{ "#define ENUMBRA_PACK_INIT_DEFAULT(Enum, Name) Enum::Value Name : Enum::bits_required_storage() {{ Enum().value() }};" },
			{ "#endif" },
			{ "" },
			{ "#else // check existing version supported" },
			{ "#if (ENUMBRA_OPTIONAL_MACROS_VERSION + 0) == 0" },
			{ "#error ENUMBRA_OPTIONAL_MACROS_VERSION has been defined without a proper version number. Check your build system." },
			{ "#elif (ENUMBRA_OPTIONAL_MACROS_VERSION + 0) < {1}" },
			{ "#error An included header was generated using a newer version of enumbra. Regenerate your headers using the same version." },
			{ "#elif (ENUMBRA_OPTIONAL_MACROS_VERSION + 0) > {1}" },
			{ "#error An included header was generated using an older version of enumbra. Regenerate your headers using the same version." },
			{ "#endif // end check existing version supported" },
			{ "#endif // ENUMBRA_OPTIONAL_MACROS_VERSION" },
		};
		for (auto& str : macro_strings) {
			write_line(str, TAB, enumbra_macros_version);
		}
		write_linefeed();
	}

	if (cfg.cpp_config.templated_extensions) {
		// The macro will allow us to update versions later and warn if old versions of the template are in use.
		const int base_template_version = 2;
		std::vector<const char*> base_template_strings = {
			{ "#if !defined(ENUMBRA_BASE_TEMPLATES_VERSION)" },
			{ "#define ENUMBRA_BASE_TEMPLATES_VERSION {1}" },
			{ "namespace enumbra {{" },
			{ "{0}namespace detail {{" },
			{ "{0}{0}// Default templates for non-enumbra types" },
			{ "{0}{0}template<class T>" },
			{ "{0}{0}struct enumbra_base_helper {{ " },
			{ "{0}{0}    static constexpr bool enumbra_type = false;" },
			{ "{0}{0}    static constexpr bool enumbra_enum_class = false;" },
			{ "{0}{0}    static constexpr bool enumbra_value_enum = false;" },
			{ "{0}{0}    static constexpr bool enumbra_flags_enum = false;" },
			{ "{0}{0}    using base_type = T; " },
			{ "{0}{0}}};" },
			{ "{0}{0}template<class T> constexpr bool streq(T* a, T* b) {{"},
			{ "{0}{0}{0}return *a == *b && (*a == '\\0' || streq(a + 1, b + 1));"},
			{ "{0}{0}}}"},
			{ "{0}}} // end namespace enumbra::detail" },
			{ "{0}template<class T> using enumbra_base_t = typename detail::enumbra_base_helper<T>::base_type;" },
			{ "{0}template<class T> constexpr bool is_enumbra_type() {{ return detail::enumbra_base_helper<T>::enumbra_type; }}"},
			{ "{0}template<class T> constexpr bool is_enumbra_type(T v) {{ return detail::enumbra_base_helper<T>::enumbra_type; }}"},
			{ "{0}template<class T> constexpr bool is_enumbra_struct() {{ return is_enumbra_type<T>() && !detail::enumbra_base_helper<T>::enumbra_enum_class; }}"},
			{ "{0}template<class T> constexpr bool is_enumbra_struct(T v) {{ return is_enumbra_type<T>() && !detail::enumbra_base_helper<T>::enumbra_enum_class; }}"},
			{ "{0}template<class T> constexpr bool is_enumbra_scoped_enum() {{ return is_enumbra_type<T>() && detail::enumbra_base_helper<T>::enumbra_enum_class; }}"},
			{ "{0}template<class T> constexpr bool is_enumbra_scoped_enum(T v) {{ return is_enumbra_type<T>() && detail::enumbra_base_helper<T>::enumbra_enum_class; }}"},
			{ "{0}template<class T> constexpr bool is_enumbra_value_enum() {{ return is_enumbra_type<T>() && detail::enumbra_base_helper<T>::enumbra_value_enum; }}"},
			{ "{0}template<class T> constexpr bool is_enumbra_value_enum(T v) {{ return is_enumbra_type<T>() && detail::enumbra_base_helper<T>::enumbra_value_enum; }}"},
			{ "{0}template<class T> constexpr bool is_enumbra_flags_enum() {{ return is_enumbra_type<T>() && detail::enumbra_base_helper<T>::enumbra_flags_enum; }}"},
			{ "{0}template<class T> constexpr bool is_enumbra_flags_enum(T v) {{ return is_enumbra_type<T>() && detail::enumbra_base_helper<T>::enumbra_flags_enum; }}"},
			{ "}} // end namespace enumbra" },
			{ "#else // check existing version supported" },
			{ "#if (ENUMBRA_BASE_TEMPLATES_VERSION + 0) == 0" },
			{ "#error ENUMBRA_BASE_TEMPLATES_VERSION has been defined without a proper version number. Check your build system." },
			{ "#elif (ENUMBRA_BASE_TEMPLATES_VERSION + 0) < {1}" },
			{ "#error An included header was generated using a newer version of enumbra. Regenerate your headers using same version of enumbra." },
			{ "#elif (ENUMBRA_BASE_TEMPLATES_VERSION + 0) > {1}" },
			{ "#error An included header was generated using an older version of enumbra. Regenerate your headers using same version of enumbra." },
			{ "#endif // check existing version supported" },
			{ "#endif // ENUMBRA_BASE_TEMPLATES_VERSION" },
		};
		for (auto& str : base_template_strings) {
			write_line(str, TAB, base_template_version);
		}
		write_linefeed();
	}

	// START NAMESPACE
	for (const auto& ns : cpp.output_namespace) {
		write_line("namespace {} {{", ns);
	}
	write_linefeed();

	// VALUE ENUM DEFINITIONS
	for (auto& e : enum_meta.value_enum_definitions) {

		// TODO: We should move all these precondition checks outside of the actual generation
		// and cache the results since we're going to need them for other languages eventually.

		// Precondition checks
		// 1. Names of contained values must be unique
		std::set<std::string> seen_names;
		for (auto& v : e.values)
		{
			auto seen = seen_names.find(v.name);
			if (seen != seen_names.end())
			{
				throw std::runtime_error("ENUM DEFINITIONS Precondition Check 1: Enum Value Name is not unique (name = " + *seen + ")");
			}
			seen_names.insert(v.name);
		}
		seen_names.clear();

		// 2. Enum must have at least 1 value
		if (e.values.size() == 0)
		{
			throw std::runtime_error("ENUM DEFINITIONS Precondition Check 2: Enum does not contain any values (name = " + e.name + ")");
		}

		// Get references and metadata for relevant enum values that we will need
		const enum_entry& default_entry = get_value_enum_entry(enum_meta.value_enum_default_value_style, e);
		const enum_entry& min_entry = get_value_enum_entry(ValueEnumDefaultValueStyle::Min, e);
		const enum_entry& max_entry = get_value_enum_entry(ValueEnumDefaultValueStyle::Max, e);
		const size_t entry_count = e.values.size();
		const std::string size_type = cpp.get_size_type_from_index(e.size_type_index).generated_name;
		const bool is_size_type_signed = cpp.get_size_type_from_index(e.size_type_index).is_signed;
		const std::string char_type = cfg.cpp_config.string_table_type == StringTableType::ConstCharPtr ? "const char*" : "const wchar_t*";
		const std::string literal_prefix = cfg.cpp_config.string_table_type == StringTableType::ConstCharPtr ? "" : "L";

		int64_t max_abs_representable = std::max(std::abs(min_entry.value - 1), max_entry.value);
		size_t bits_required_storage = unsigned_bits_required(max_abs_representable);
		const size_t bits_required_transmission = log_2_unsigned(max_entry.value - min_entry.value);

		// Because of the way signed integers map to bit fields, a bit field may require an additional
		// bit of storage to accomodate the sign bit even if it is unused. For example, given the following enum:
		//   enum class ESignedValueBits : int8_t { A = 0, B = 1, C = 2, D = 3 }
		// To properly store and assign to this enum, we need 3 bits:
		//   int8_t Value : 2; // ERROR: maps to the range -2 - 1
		//   int8_t Value : 3; // OK:    maps to the range -4 - 3, but we're wasting space
		// For this reason, when utilizing packed enums it is recommended to always prefer an unsigned underlying
		// type unless your enum actually contains negative values.
		if (is_size_type_signed && max_entry.value > 0) {
			uint64_t signed_range_max = 0;
			for (int i = 0; i < bits_required_storage - 1; i++) {
				signed_range_max |= 1ULL << i;
			}
			if (uint64_t(max_entry.value) > signed_range_max) {
				bits_required_storage += 1;
			}
		}

		// Determine if all values are unique, or if some enum value names overlap.
		// TODO: Enforce if flag is set
		std::set<int64_t> unique_values;
		for (auto& v : e.values) { unique_values.insert(v.value); }
		const size_t unique_entry_count = unique_values.size();

		// Determine if range is contiguous
		// Enables some minor optimizations for range-checking values if true
		// TODO: Enforce if flag is set
		bool is_contiguous = is_value_set_contiguous(unique_values);

		// Definition
		write_line("// {} Definition", e.name);
		write_line("struct {} {{", e.name);
		{
			write_line_tabbed(1, "using UnderlyingType = {};", size_type);
			write_line_tabbed(1, "enum class Value : {} {{", size_type);
			for (const auto& v : e.values) {
				write_line_tabbed(2, "{} = {},", v.name, v.value);
			}
			write_line_tabbed(1, "}};");
			write_linefeed();

			write_line_tabbed(1, "constexpr {}() : value_(Value({})) {{ }}", e.name, default_entry.value);
			write_line_tabbed(1, "constexpr {}(Value v) : value_(v) {{ }}", e.name);
			write_linefeed();

			for (const auto& v : e.values) {
				write_line_tabbed(1, "constexpr static Value {0} = Value::{0};", v.name);
			}
			write_linefeed();

			write_line_tabbed(1, "constexpr static std::array<Value, {}> Values = {{", entry_count);
			for (const auto& v : e.values) {
				write_line_tabbed(2, "{},", v.name);
			}
			write_line_tabbed(1, "}};");
			write_linefeed();

			// Operators
			write_line_tabbed(1, "constexpr Value value() const {{ return value_; }}");
			write_line_tabbed(1, "constexpr operator Value() const {{ return value_; }}");
			write_line_tabbed(1, "explicit operator bool() = delete;");
			write_linefeed();

			// Functions
			write_line_tabbed(1, "constexpr {0} to_underlying() const {{ return static_cast<{0}>(value_); }}", size_type);
			write_line_tabbed(1, "ENUMBRA_CONSTEXPR_NONCONSTFUNC void reset_default() {{ *this = {0}(); }}", e.name);
			write_linefeed();

			// Introspection
			write_line_tabbed(1, "static constexpr {0}::Value default_value() {{ return Value({1}); }}", e.name, default_entry.value);
			write_line_tabbed(1, "static constexpr bool is_enumbra_value_enum() {{ return true; }}");
			write_line_tabbed(1, "static constexpr bool is_enumbra_flags_enum() {{ return false; }}");
			write_line_tabbed(1, "static constexpr {1} min() {{ return {2}; }}", e.name, size_type, min_entry.value);
			write_line_tabbed(1, "static constexpr {1} max() {{ return {2}; }}", e.name, size_type, max_entry.value);
			write_line_tabbed(1, "static constexpr int count() {{ return {1}; }}", e.name, unique_entry_count);
			write_line_tabbed(1, "static constexpr bool is_contiguous() {{ return {1}; }}", e.name, (is_contiguous ? "true" : "false"));
			write_line_tabbed(1, "static constexpr {0} from_underlying_unsafe({1} v) {{ return {0}(static_cast<Value>(v)); }}", e.name, size_type);
			write_line_tabbed(1, "static constexpr {1} bits_required_storage() {{ return {2}; }}", e.name, size_type, bits_required_storage);
			write_line_tabbed(1, "static constexpr {1} bits_required_transmission() {{ return {2}; }}", e.name, size_type, bits_required_transmission);

			// is_valid variations
			if (is_contiguous)
			{
				if (min_entry.value == 0 && !is_size_type_signed) // Unsigned values can't go below 0 so we just need to check that we're <= max
				{
					write_line_tabbed(1, "static constexpr bool is_valid({0} v) {{ return static_cast<{1}>(v.value_) <= {2}; }}", e.name, size_type, max_entry.value);
					write_line_tabbed(1, "static constexpr bool is_valid({1} v) {{ return v <= {2}; }}", e.name, size_type, max_entry.value);
				}
				else
				{
					write_line_tabbed(1, "static constexpr bool is_valid({0} v) {{ return ({2} <= static_cast<{1}>(v.value_)) && (static_cast<{1}>(v.value_) <= {3}); }}", e.name, size_type, min_entry.value, max_entry.value);
					write_line_tabbed(1, "static constexpr bool is_valid({1} v) {{ return ({2} <= v) && (v <= {3}); }}", e.name, size_type, min_entry.value, max_entry.value);
				}
			}
			else
			{
				write_line_tabbed(1, "static ENUMBRA_CONSTEXPR_NONCONSTFUNC bool is_valid({0} v) {{ for(std::size_t i = 0; i < Values.size(); i++) {{ auto& val = Values[i]; if(val == v) return true; }} return false; }}", e.name);
				write_line_tabbed(1, "static ENUMBRA_CONSTEXPR_NONCONSTFUNC bool is_valid({1} v) {{ for(std::size_t i = 0; i < Values.size(); i++) {{ auto& val = Values[i]; if(val == Value(v)) return true; }} return false; }}", e.name, size_type);
			}
			write_linefeed();

			// String Functions
			write_line_tabbed(1, "static ENUMBRA_CONSTEXPR_NONCONSTFUNC {0} to_string(const {1}::Value v) {{", char_type, e.name);
			write_line_tabbed(2, "switch (v) {{");
			for (auto v : e.values) {
				write_line_tabbed(3, "case {0}: return {1}\"{0}\";", v.name, literal_prefix);
			}
			write_line_tabbed(3, "default: return \"\";");
			write_line_tabbed(2, "}}");
			write_line_tabbed(1, "}}");

			write_line_tabbed(1, "static ENUMBRA_CONSTEXPR_NONCONSTFUNC {0}::Value from_string({1} str, bool& success) {{", e.name, char_type);
			write_line_tabbed(2, "for (std::size_t i = 0; i < string_lookup_.size(); i++) {{");
			write_line_tabbed(3, "if (enumbra::detail::streq(string_lookup_[i].second, str) == 0) {{");
			write_line_tabbed(4, "return string_lookup_[i].first;");
			write_line_tabbed(3, "}}");
			write_line_tabbed(2, "}}");
			write_line_tabbed(2, "success = false;");
			write_line_tabbed(2, "return default_value();", e.name);
			write_line_tabbed(1, "}}");

			// Private Members
			write_linefeed();
			write_line("private:");
			write_line_tabbed(1, "Value value_;");

			// Value String Table
			write_line_tabbed(1, "constexpr static std::array<std::pair<Value,{0}>, {1}> string_lookup_ = {{", char_type, entry_count);
			for (const auto& v : e.values) {
				write_line_tabbed(2, "std::make_pair({1}, {0}\"{1}\"),", literal_prefix, v.name, size_type);
			}
			write_line_tabbed(1, "}};");

		}
		write_line("}};");

		/*std::vector<const char*> operator_strings = {
			{"// {} Operator Overloads"},
			{"constexpr bool operator==(const {0}& a, const {0}& b) {{ return a.value() == b.value(); }}"},
			{"constexpr bool operator!=(const {0}& a, const {0}& b) {{ return a.value() != b.value(); }}"},
		};
		for (auto& str : operator_strings) {
			write_line(str, e.name);
		}*/

		write_linefeed();
	}

	// Flags ENUM DEFINITIONS
	for (auto& e : enum_meta.flag_enum_definitions) {

		// TODO: We should move all these precondition checks outside of the actual generation
		// and cache the results since we're going to need them for other languages eventually.

		// Precondition checks
		// 1. Names of contained values must be unique
		std::set<std::string> seen_names;
		for (auto& v : e.values)
		{
			auto seen = seen_names.find(v.name);
			if (seen != seen_names.end())
			{
				throw std::logic_error("ENUM DEFINITIONS Check 1: Enum Value Name is not unique (enum name = " + *seen + ")");
			}
			seen_names.insert(v.name);
		}
		seen_names.clear();

		// 2. Enum must have at least 1 value
		if (e.values.size() == 0)
		{
			throw std::logic_error("ENUM DEFINITIONS Check 2: Enum does not contain any values (enum name = " + e.name + ")");
		}

		// Determine if all values are unique, or if some enum value names overlap.
		// TODO: Enforce if flag is set
		std::set<int64_t> unique_values;
		for (auto& v : e.values) { unique_values.insert(v.value); }
		const size_t unique_entry_count = unique_values.size();
		if (e.values.size() != unique_values.size())
		{
			throw std::logic_error("ENUM DEFINITIONS Check 3: Values in enum do not have unique values (enum name = " + e.name + ")");
		}

		// Get references and metadata for relevant enum values that we will need
		const uint64_t min_value = 0; // The minimum for a flags entry is always 0 - no bits set
		uint64_t max_value = 0;
		for (auto& v : e.values) {
			if (v.value < 0) {
				throw std::logic_error("ENUM DEFINITIONS Check 4: Flags-Enum value is less than 0. Flags-Enum values are required to be unsigned. (enum name = " + e.name + ")");
			}
			max_value |= uint64_t(v.value);
		}

		const int64_t default_value = get_flags_enum_value(enum_meta.flags_enum_default_value_style, e);

		const size_t entry_count = e.values.size();
		const size_t bits_required_storage = unsigned_bits_required(max_value);
		const size_t bits_required_transmission = bits_required_storage;
		const std::string size_type = cpp.get_size_type_from_index(e.size_type_index).generated_name;
		const bool is_size_type_signed = cpp.get_size_type_from_index(e.size_type_index).is_signed;
		if (is_size_type_signed) {
			throw std::logic_error("Size type for flags enum is signed. Not a supported configuration.");
		}

		// Determine if range is contiguous
		// Enables some minor optimizations for range-checking values if true
		// TODO: Enforce if flag is set
		bool is_contiguous = is_flags_set_contiguous(unique_values);

		// Definition
		write_line("// {} Definition", e.name);
		write_line("struct {} {{", e.name);
		{
			write_line_tabbed(1, "using UnderlyingType = {};", size_type);
			write_line_tabbed(1, "enum class Value : {} {{", size_type);
			for (const auto& v : e.values) {
				write_line_tabbed(2, "{} = {},", v.name, v.value);
			}
			write_line_tabbed(1, "}};");
			write_linefeed();

			write_line_tabbed(1, "constexpr {}() : value_(Value({})) {{ }}", e.name, default_value);
			write_line_tabbed(1, "constexpr {}(Value v) : value_(v) {{ }}", e.name);
			write_linefeed();

			for (const auto& v : e.values) {
				write_line_tabbed(1, "constexpr static Value {0} = Value::{0};", v.name);
			}
			write_linefeed();

			write_line_tabbed(1, "constexpr static std::array<Value, {}> Values = {{", entry_count);
			for (const auto& v : e.values) {
				write_line_tabbed(2, "{},", v.name);
			}
			write_line_tabbed(1, "}};");
			write_linefeed();

			// Operators
			write_line_tabbed(1, "constexpr Value value() const {{ return value_; }}");
			write_line_tabbed(1, "constexpr explicit operator bool() const = delete;");
			write_linefeed();

			// Functions
			write_line_tabbed(1, "constexpr {0} to_underlying() const {{ return static_cast<{0}>(value_); }}", size_type);
			write_line_tabbed(1, "ENUMBRA_CONSTEXPR_NONCONSTFUNC void reset_default() {{ *this = {}(); }}", e.name);
			write_line_tabbed(1, "ENUMBRA_CONSTEXPR_NONCONSTFUNC void reset_zero() {{ value_ = static_cast<Value>(0); }}", e.name);
			write_line_tabbed(1, "constexpr bool test(Value v) const {{ return (static_cast<{0}>(value_) & static_cast<{0}>(v)) == static_cast<{0}>(v); }}", size_type);
			write_line_tabbed(1, "ENUMBRA_CONSTEXPR_NONCONSTFUNC void unset(Value v) {{ value_ = static_cast<Value>(static_cast<{0}>(value_) & (~static_cast<{0}>(v))); }}", size_type);
			write_line_tabbed(1, "ENUMBRA_CONSTEXPR_NONCONSTFUNC void flip(Value v) {{ value_ = static_cast<Value>(static_cast<{0}>(value_) ^ static_cast<{0}>(v)); }}", size_type);
			write_line_tabbed(1, "constexpr bool all() const {{ return static_cast<{0}>(value_) >= {1:#x}; }}", size_type, max_value);
			write_line_tabbed(1, "constexpr bool any() const {{ return static_cast<{0}>(value_) > 0; }}", size_type);
			write_line_tabbed(1, "constexpr bool none() const {{ return static_cast<{0}>(value_) == 0; }}", size_type);
			write_line_tabbed(1, "ENUMBRA_CONSTEXPR_NONCONSTFUNC bool single() const {{ {0} n = static_cast<uint32_t>(value_); return n && !(n & (n - 1)); }}", size_type);
			write_linefeed();

			// Introspection
			write_line_tabbed(1, "static constexpr {0}::Value default_value() {{ return Value({1}); }}", e.name, default_value);
			write_line_tabbed(1, "static constexpr bool is_enumbra_value_enum() {{ return false; }}");
			write_line_tabbed(1, "static constexpr bool is_enumbra_flags_enum() {{ return true; }}");
			write_line_tabbed(1, "static constexpr {1} min() {{ return {2}; }}", e.name, size_type, min_value);
			write_line_tabbed(1, "static constexpr {1} max() {{ return {2:#x}; }}", e.name, size_type, max_value);
			write_line_tabbed(1, "static constexpr int count() {{ return {1}; }}", e.name, unique_entry_count);
			write_line_tabbed(1, "static constexpr bool is_contiguous() {{ return {1}; }}", e.name, (is_contiguous ? "true" : "false"));
			write_line_tabbed(1, "static constexpr {0} from_underlying_unsafe({1} v) {{ return {0}(static_cast<Value>(v)); }}", e.name, size_type);
			write_line_tabbed(1, "static constexpr {1} bits_required_storage() {{ return {2}; }}", e.name, size_type, bits_required_storage);
			write_line_tabbed(1, "static constexpr {1} bits_required_transmission() {{ return {2}; }}", e.name, size_type, bits_required_transmission);
			if (is_contiguous)
			{
				if (min_value == 0 && !is_size_type_signed) // Unsigned values can't go below 0 so we just need to check that we're <= max
				{
					write_line_tabbed(1, "static constexpr bool is_valid({0} v) {{ return static_cast<{1}>(v.value_) <= {2}; }}", e.name, size_type, max_value);
					write_line_tabbed(1, "static constexpr bool is_valid({1} v) {{ return v <= {2}; }}", e.name, size_type, max_value);
				}
				else
				{
					write_line_tabbed(1, "static constexpr bool is_valid({0} v) {{ return ({2} <= static_cast<{1}>(v.value_)) && (static_cast<{1}>(v.value_) <= {3}); }}", e.name, size_type, min_value, max_value);
					write_line_tabbed(1, "static constexpr bool is_valid({1} v) {{ return ({2} <= v) && (v <= {3}); }}", e.name, size_type, min_value, max_value);
				}
			}
			else
			{
				write_line_tabbed(1, "static ENUMBRA_CONSTEXPR_NONCONSTFUNC bool is_valid({0} v) {{ for(std::size_t i = 0; i < Values.size(); i++) {{ auto& val = Values[i]; if(val == v.value()) return true; }} return false; }}", e.name);
				write_line_tabbed(1, "static ENUMBRA_CONSTEXPR_NONCONSTFUNC bool is_valid({1} v) {{ for(std::size_t i = 0; i < Values.size(); i++) {{ auto& val = Values[i]; if(val == Value(v)) return true; }} return false; }}", e.name, size_type);
			}
			write_linefeed();

			write_line("private:");
			write_line_tabbed(1, "Value value_;");
		}
		write_line("}};");
		write_linefeed();

		std::vector<const char*> operator_strings = {
			{"// {} Operator Overloads"},
			{"constexpr bool operator==(const {0}& a, const {0}& b) {{ return a.value() == b.value(); }}"},
			{"constexpr bool operator!=(const {0}& a, const {0}& b) {{ return a.value() != b.value(); }}"},

			// Value operators are first because they are required for the operators afterwards
			{"constexpr {0}::Value operator~(const {0}::Value a) {{ return static_cast<{0}::Value>(~static_cast<{1}>(a)); }}"},
			{"constexpr {0}::Value operator|(const {0}::Value a, const {0}::Value b) {{ return static_cast<{0}::Value>(static_cast<{1}>(a) | static_cast<{1}>(b)); }}"},
			{"constexpr {0}::Value operator&(const {0}::Value a, const {0}::Value b) {{ return static_cast<{0}::Value>(static_cast<{1}>(a) & static_cast<{1}>(b)); }}"},
			{"constexpr {0}::Value operator^(const {0}::Value a, const {0}::Value b) {{ return static_cast<{0}::Value>(static_cast<{1}>(a) ^ static_cast<{1}>(b)); }}"},

			{"ENUMBRA_CONSTEXPR_NONCONSTFUNC void operator|=({0}::Value a, const {0}::Value b) {{ a = static_cast<{0}::Value>(static_cast<{1}>(a) | static_cast<{1}>(b)); }}"},
			{"ENUMBRA_CONSTEXPR_NONCONSTFUNC void operator&=({0}::Value a, const {0}::Value b) {{ a = static_cast<{0}::Value>(static_cast<{1}>(a) & static_cast<{1}>(b)); }}"},
			{"ENUMBRA_CONSTEXPR_NONCONSTFUNC void operator^=({0}::Value a, const {0}::Value b) {{ a = static_cast<{0}::Value>(static_cast<{1}>(a) ^ static_cast<{1}>(b)); }}"},

			{"constexpr {0} operator~(const {0} a) {{ return ~a.value(); }}"},
			{"constexpr {0} operator|(const {0} a, const {0} b) {{ return a.value() | b.value(); }}"},
			{"constexpr {0} operator&(const {0} a, const {0} b) {{ return a.value() & b.value(); }}"},
			{"constexpr {0} operator^(const {0} a, const {0} b) {{ return a.value() ^ b.value(); }}"},
			{"constexpr {0} operator|(const {0} a, const {0}::Value b) {{ return a.value() | b; }}"},
			{"constexpr {0} operator&(const {0} a, const {0}::Value b) {{ return a.value() & b; }}"},
			{"constexpr {0} operator^(const {0} a, const {0}::Value b) {{ return a.value() ^ b; }}"},
			{"constexpr {0} operator|(const {0}::Value a, const {0} b) {{ return a | b.value(); }}"},
			{"constexpr {0} operator&(const {0}::Value a, const {0} b) {{ return a & b.value(); }}"},
			{"constexpr {0} operator^(const {0}::Value a, const {0} b) {{ return a ^ b.value(); }}"},

			{"ENUMBRA_CONSTEXPR_NONCONSTFUNC {0}& operator|=({0}& a, const {0} b) {{ a = a | b; return a; }}"},
			{"ENUMBRA_CONSTEXPR_NONCONSTFUNC {0}& operator&=({0}& a, const {0} b) {{ a = a & b; return a; }}"},
			{"ENUMBRA_CONSTEXPR_NONCONSTFUNC {0}& operator^=({0}& a, const {0} b) {{ a = a ^ b; return a; }}"},
		};
		for (auto& str : operator_strings) {
			write_line(str, e.name, size_type);
		}
		write_linefeed();
	}

	// END NAMESPACE
	for (auto ns = cpp.output_namespace.rbegin(); ns != cpp.output_namespace.rend(); ++ns) {
		write_line("}} // namespace {}", *ns);
	}
	write_linefeed();

	// Construct the full namespace for templates
	std::string full_ns;
	for (auto& ns : cpp.output_namespace) {
		full_ns += ns + "::";
	}

	// MSVC C2888: Template specializations need to be outside of the user-defined namespace so we'll stick them after the definitions.
	if (cfg.cpp_config.templated_extensions)
	{
		for (auto& e : enum_meta.value_enum_definitions) {
			// Value Enum Template Specializations
			std::string is_value_enum = "true";
			std::string is_flags_enum = "false";
			std::vector<const char*> template_strings = {
				{"// {4}{1} Template Specializations"},
				{"template<> struct enumbra::detail::enumbra_base_helper<{4}{1}::Value> {{"},
				{"{0}static constexpr bool enumbra_type = true;"},
				{"{0}static constexpr bool enumbra_enum_class = true;"},
				{"{0}static constexpr bool enumbra_value_enum = {2};"},
				{"{0}static constexpr bool enumbra_flags_enum = {3};"},
				{"{0}using base_type = {4}{1};"},
				{"}};"},
				{"template<> struct enumbra::detail::enumbra_base_helper<{4}{1}> {{"},
				{"{0}static constexpr bool enumbra_type = true;"},
				{"{0}static constexpr bool enumbra_enum_class = false;"},
				{"{0}static constexpr bool enumbra_value_enum = {2};"},
				{"{0}static constexpr bool enumbra_flags_enum = {3};"},
				{"{0}using base_type = {4}{1};"},
				{"}};"},
			};
			for (auto& str : template_strings) {
				write_line(str, TAB, e.name, is_value_enum, is_flags_enum, full_ns);
			}
		}

		for (auto& e : enum_meta.flag_enum_definitions) {
			// Flags Enum Template Specializations
			std::string is_value_enum = "false";
			std::string is_flags_enum = "true";
			std::vector<const char*> template_strings = {
				{"// {4}{1} Template Specializations"},
				{"template<> struct enumbra::detail::enumbra_base_helper<{4}{1}::Value> {{"},
				{"{0}static constexpr bool enumbra_type = true;"},
				{"{0}static constexpr bool enumbra_enum_class = true;"},
				{"{0}static constexpr bool enumbra_value_enum = {2};"},
				{"{0}static constexpr bool enumbra_flags_enum = {3};"},
				{"{0}using base_type = {4}{1};"},
				{"}};"},
				{"template<> struct enumbra::detail::enumbra_base_helper<{4}{1}> {{"},
				{"{0}static constexpr bool enumbra_type = true;"},
				{"{0}static constexpr bool enumbra_enum_class = false;"},
				{"{0}static constexpr bool enumbra_value_enum = {2};"},
				{"{0}static constexpr bool enumbra_flags_enum = {3};"},
				{"{0}using base_type = {4}{1};"},
				{"}};"},
			};
			for (auto& str : template_strings) {
				write_line(str, TAB, e.name, is_value_enum, is_flags_enum, full_ns);
			}
		}
	}

	// END INCLUDE GUARD
	if (cpp.include_guard_style == IncludeGuardStyle::CStyle) {
		write_line("#endif // {}", def_macro);
	}

	return Output;
}

std::vector<cpp_enum_generated> cpp_generator::generate_enums(const cpp_enum_config_final& /*cfg*/)
{
	throw std::logic_error("Unimplemented");
}
