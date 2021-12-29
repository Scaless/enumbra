#include "cpp_generator.h"
#include <sstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <set>
#include <fmt/format.h>

using namespace enumbra;
using namespace enumbra::cpp;

std::string to_upper(const std::string& str)
{
	std::string strcopy = str;
	std::transform(strcopy.begin(), strcopy.end(), strcopy.begin(), ::toupper);
	return strcopy;
}

const int64_t get_flags_enum_value(const FlagsEnumDefaultValueStyle& style, const enum_definition& definition)
{
	switch (style)
	{
	case FlagsEnumDefaultValueStyle::Zero: return 0;
	case FlagsEnumDefaultValueStyle::Min:
	{
		auto& m = std::min_element(definition.values.begin(), definition.values.end(),
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
		auto& m = std::max_element(definition.values.begin(), definition.values.end(),
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
		auto& m = std::min_element(definition.values.begin(), definition.values.end(),
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
		auto& m = std::max_element(definition.values.begin(), definition.values.end(),
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
constexpr uint64_t Log2Unsigned(uint64_t x)
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

// Enum names must be unique across value and flag enums
bool enum_meta_has_unique_enum_names(const enumbra::enum_meta_config& enum_meta)
{
	std::set<std::string> seen_names;
	for (auto& v : enum_meta.value_enum_definitions)
	{
		auto& seen = seen_names.find(v.name);
		if (seen != seen_names.end())
		{
			throw std::logic_error("enum_meta_has_unique_enum_names: Value-Enum name is not unique (name = " + *seen + ")");
		}
		seen_names.insert(v.name);
	}
	for (auto& v : enum_meta.flag_enum_definitions)
	{
		auto& seen = seen_names.find(v.name);
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
	int64_t check_bit = Log2Unsigned(*flags.begin()) + 1;
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
	case enumbra::cpp::IncludeGuardStyle::None:
	{
		break;
	}
	case enumbra::cpp::IncludeGuardStyle::PragmaOnce:
	{
		write("#pragma once");
		write_linefeed(2);
		break;
	}
	case enumbra::cpp::IncludeGuardStyle::CStyle:
	{
		write("#ifndef {0}{1}#define {0}{1}{1}", def_macro, LF);
		break;
	}
	default:
		throw std::runtime_error("include_guard_wrap: Invalid IncludeGuardStyle");
	}

	// INCLUDES
	write_line("#include <array>"); // Required currently, could we use C-style arrays to remove dependency?
	for (const auto& inc : cpp.additional_includes) {
		write_line("#include {}", inc);
	}
	if (cpp.string_table_layout != StringTableLayout::None) {
		if (cpp.string_table_type == StringTableType::ConstexprStringView || cpp.string_table_type == StringTableType::ConstexprWStringView)
		{
			write_line("#include <string_view>");
		}
	}
	write_linefeed();

	// MACRO DEFINITIONS
	if (cfg.cpp_config.packed_declaration_macros)
	{
		write("#define ENUMBRA_PACK(Enum, Name) Enum::Value Name : Enum##::bits_required_storage();");
		write_linefeed(2);
	}

	// START CONFIG NAMESPACE
	for (const auto& ns : cpp.output_namespace) {
		write_line("namespace {} {{", ns);
	}
	write_linefeed();

	// VALUE ENUM DEFINITIONS
	for (auto& e : enum_meta.value_enum_definitions) {

		// Precondition checks
		// 1. Names of contained values must be unique
		std::set<std::string> seen_names;
		for (auto& v : e.values)
		{
			auto& seen = seen_names.find(v.name);
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
		const size_t bits_required_storage = Log2Unsigned(max_entry.value) + 1;
		const size_t bits_required_transmission = Log2Unsigned(max_entry.value - min_entry.value) + 1;
		const std::string size_type = cpp.get_size_type_from_index(e.size_type_index).generated_name;

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
			write_line_tabbed(1, "using ValueType = {};", size_type);
			write_line_tabbed(1, "enum class Value : {} {{", cpp.get_size_type_from_index(e.size_type_index).generated_name);
			for (const auto& v : e.values) {
				write_line_tabbed(2, "{} = {},", v.name, v.value);
			}
			write_line_tabbed(1, "}};");
			write_linefeed();

			write_line_tabbed(1, "constexpr {}() : value(Value({})) {{ }};", e.name, default_entry.value);
			write_line_tabbed(1, "constexpr {}(Value v) : value(v) {{ }}", e.name);
			write_linefeed();
			write_line_tabbed(1, "constexpr static std::array<Value, {}> Values = {{", entry_count);
			for (const auto& v : e.values) {
				write_line_tabbed(2, "Value::{},", v.name);
			}
			write_line_tabbed(1, "}};");
			write_linefeed();

			for (const auto& v : e.values) {
				write_line_tabbed(1, "constexpr static Value {0} = Value::{0};", v.name);
			}
			write_linefeed();

			// String Tables
			/*write_line_tabbed(1, "constexpr static std::array<const char*, {}> Values = {{", entry_count);
			for (const auto& v : e.values) {
				write_line_tabbed(2, "Value::{},", v.name);
			}
			write_line_tabbed(1, "}};");*/

			// Operators
			write_line_tabbed(1, "constexpr operator Value() const {{ return value; }}");
			write_line_tabbed(1, "explicit operator bool() = delete;");
			write_linefeed();

			// Functions
			write_line_tabbed(1, "constexpr void reset() {{ value = {}(); }}", e.name);
			write_linefeed();

			// Introspection
			write_line_tabbed(1, "static constexpr bool is_enumbra_value_enum() {{ return true; }}");
			write_line_tabbed(1, "static constexpr bool is_enumbra_flags_enum() {{ return false; }}");
			write_line_tabbed(1, "static constexpr {1} min() {{ return {2}; }}", e.name, size_type, min_entry.value);
			write_line_tabbed(1, "static constexpr {1} max() {{ return {2}; }}", e.name, size_type, max_entry.value);
			write_line_tabbed(1, "static constexpr int count() {{ return {1}; }}", e.name, unique_entry_count);
			write_line_tabbed(1, "static constexpr bool is_contiguous() {{ return {1}; }}", e.name, (is_contiguous ? "true" : "false"));
			write_line_tabbed(1, "static constexpr {0} from_underlying_unsafe({1} v) {{ return {0}(static_cast<Value>(v)); }}", e.name, size_type);
			write_line_tabbed(1, "static constexpr {1} bits_required_storage() {{ return {2}; }}", e.name, size_type, bits_required_storage);
			write_line_tabbed(1, "static constexpr {1} bits_required_transmission() {{ return {2}; }}", e.name, size_type, bits_required_transmission);
			if (is_contiguous)
			{
				write_line_tabbed(1, "static constexpr bool contains({0} v) {{ return ({2} <= static_cast<{1}>(v.value)) && (static_cast<{1}>(v.value) <= {3}); }}", e.name, size_type, min_entry.value, max_entry.value);
				write_line_tabbed(1, "static constexpr bool contains({1} v) {{ return ({2} <= v) && (v <= {3}); }}", e.name, size_type, min_entry.value, max_entry.value);
			}
			else
			{
				write_line_tabbed(1, "static constexpr bool contains({0} v) {{ return std::find(Values.begin(), Values.end(), v) != Values.end(); }}", e.name);
				write_line_tabbed(1, "static constexpr bool contains({1} v) {{ return std::find(Values.begin(), Values.end(), static_cast<{0}>(v)) != Values.end(); }}", e.name, size_type);
			}
			write_linefeed();
			write_line("private:");
			write_line_tabbed(1, "Value value;");
		}
		write_line("}};");
		write_linefeed();
	}

	// Flag ENUM DEFINITIONS
	for (auto& e : enum_meta.flag_enum_definitions) {

		// Precondition checks
		// 1. Names of contained values must be unique
		std::set<std::string> seen_names;
		for (auto& v : e.values)
		{
			auto& seen = seen_names.find(v.name);
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
		const size_t bits_required_storage = Log2Unsigned(max_value) + 1;
		const size_t bits_required_transmission = bits_required_storage;
		const std::string size_type = cpp.get_size_type_from_index(e.size_type_index).generated_name;

		// Determine if range is contiguous
		// Enables some minor optimizations for range-checking values if true
		// TODO: Enforce if flag is set
		bool is_contiguous = is_flags_set_contiguous(unique_values);

		// Definition
		write_line("// {} Definition", e.name);
		write_line("struct {} {{", e.name);
		{
			write_line_tabbed(1, "using ValueType = {};", size_type);
			write_line_tabbed(1, "enum class Value : {} {{", cpp.get_size_type_from_index(e.size_type_index).generated_name);
			for (const auto& v : e.values) {
				write_line_tabbed(2, "{} = {},", v.name, v.value);
			}
			write_line_tabbed(1, "}};");
			write_linefeed();

			write_line_tabbed(1, "constexpr {}() : value(Value({})) {{ }};", e.name, default_value);
			write_line_tabbed(1, "constexpr {}(Value v) : value(v) {{ }}", e.name);
			write_linefeed();
			write_line_tabbed(1, "constexpr static std::array<Value, {}> Values = {{", entry_count);
			for (const auto& v : e.values) {
				write_line_tabbed(2, "Value::{},", v.name);
			}
			write_line_tabbed(1, "}};");
			write_linefeed();

			for (const auto& v : e.values) {
				write_line_tabbed(1, "constexpr static Value {0} = Value::{0};", v.name);
			}
			write_linefeed();

			// Operators
			write_line_tabbed(1, "constexpr operator Value() const {{ return value; }}");
			write_line_tabbed(1, "explicit operator bool() = delete;");
			write_linefeed();

			// Functions
			write_line_tabbed(1, "constexpr void reset() {{ value = {}(); }}", e.name);
			write_line_tabbed(1, "constexpr bool is_set(Value v) const {{ return (*this & v) == v; }}");

			write_line_tabbed(1, "constexpr bool all() const {{ return static_cast<{0}>(value) == {1}; }}", size_type, max_value);
			write_line_tabbed(1, "constexpr bool any() const {{ return static_cast<{0}>(value) > 0; }}", size_type);
			write_line_tabbed(1, "constexpr bool none() const {{ return static_cast<{0}>(value) == 0; }}", size_type);
			write_linefeed();

			// Introspection
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
				write_line_tabbed(1, "static constexpr bool contains({0} v) {{ return ({2} <= static_cast<{1}>(v.value)) && (static_cast<{1}>(v.value) <= {3}); }}", e.name, size_type, min_value, max_value);
				write_line_tabbed(1, "static constexpr bool contains({1} v) {{ return ({2} <= v) && (v <= {3}); }}", e.name, size_type, min_value, max_value);
			}
			else
			{
				write_line_tabbed(1, "static constexpr bool contains({0} v) {{ return std::find(Values.begin(), Values.end(), v) != Values.end(); }}", e.name);
				write_line_tabbed(1, "static constexpr bool contains({1} v) {{ return std::find(Values.begin(), Values.end(), static_cast<{0}>(v)) != Values.end(); }}", e.name, size_type);
			}
			write_linefeed();

			// Requires Value operators which are defined after the class
			write_line_tabbed(1, "friend constexpr {0} operator~(const {0} a);", e.name);
			write_line_tabbed(1, "friend constexpr {0} operator|(const {0} a, const {0} b);", e.name);
			write_line_tabbed(1, "friend constexpr {0} operator&(const {0} a, const {0} b);", e.name);
			write_line_tabbed(1, "friend constexpr {0} operator^(const {0} a, const {0} b);", e.name);
			write_line_tabbed(1, "friend constexpr {0} operator|(const {0} a, const {0}::Value b);", e.name);
			write_line_tabbed(1, "friend constexpr {0} operator&(const {0} a, const {0}::Value b);", e.name);
			write_line_tabbed(1, "friend constexpr {0} operator^(const {0} a, const {0}::Value b);", e.name);
			write_line_tabbed(1, "friend constexpr {0} operator|(const {0}::Value a, const {0} b);", e.name);
			write_line_tabbed(1, "friend constexpr {0} operator&(const {0}::Value a, const {0} b);", e.name);
			write_line_tabbed(1, "friend constexpr {0} operator^(const {0}::Value a, const {0} b);", e.name);

			write_line("private:");
			write_line_tabbed(1, "Value value;");
		}
		write_line("}};");
		write_linefeed();

		write_line("// {} Operator Overloads", e.name);
		// Value operators are first because they are required for the Enum operators
		write_line("constexpr {0}::Value operator~(const {0}::Value a) {{ return static_cast<{0}::Value>(~static_cast<{1}>(a)); }}", e.name, size_type);
		write_line("constexpr {0}::Value operator|(const {0}::Value a, const {0}::Value b) {{ return static_cast<{0}::Value>(static_cast<{1}>(a) | static_cast<{1}>(b)); }}", e.name, size_type);
		write_line("constexpr {0}::Value operator&(const {0}::Value a, const {0}::Value b) {{ return static_cast<{0}::Value>(static_cast<{1}>(a) & static_cast<{1}>(b)); }}", e.name, size_type);
		write_line("constexpr {0}::Value operator^(const {0}::Value a, const {0}::Value b) {{ return static_cast<{0}::Value>(static_cast<{1}>(a) ^ static_cast<{1}>(b)); }}", e.name, size_type);
		// Enum operators
		write_line("constexpr {0} operator~ (const {0} a) {{ return static_cast<{0}>(static_cast<{0}::Value>(~static_cast<{1}>(a.value))); }}", e.name, size_type);
		write_line("constexpr {0} operator| (const {0} a, const {0} b) {{ return a.value | b.value; }}", e.name);
		write_line("constexpr {0} operator& (const {0} a, const {0} b) {{ return a.value & b.value; }}", e.name);
		write_line("constexpr {0} operator^ (const {0} a, const {0} b) {{ return a.value ^ b.value; }}", e.name);
		write_line("constexpr {0} operator| (const {0} a, const {0}::Value b) {{ return a.value | b; }}", e.name);
		write_line("constexpr {0} operator& (const {0} a, const {0}::Value b) {{ return a.value & b; }}", e.name);
		write_line("constexpr {0} operator^ (const {0} a, const {0}::Value b) {{ return a.value ^ b; }}", e.name);
		write_line("constexpr {0} operator| (const {0}::Value a, const {0} b) {{ return a | b.value; }}", e.name);
		write_line("constexpr {0} operator& (const {0}::Value a, const {0} b) {{ return a & b.value; }}", e.name);
		write_line("constexpr {0} operator^ (const {0}::Value a, const {0} b) {{ return a ^ b.value; }}", e.name);
		write_line("constexpr {0}& operator|= ({0} & a, const {0} b) {{ a = a | b; return a; }}", e.name);
		write_line("constexpr {0}& operator&= ({0} & a, const {0} b) {{ a = a & b; return a; }}", e.name);
		write_line("constexpr {0}& operator^= ({0} & a, const {0} b) {{ a = a ^ b; return a; }}", e.name);
		
		write_linefeed();
	}

	// END CONFIG NAMESPACE
	write_linefeed();
	for (auto& ns = cpp.output_namespace.rbegin(); ns != cpp.output_namespace.rend(); ++ns) {
		write_line("}} // namespace {}", *ns);
	}

	// END INCLUDE GUARD
	if (cpp.include_guard_style == IncludeGuardStyle::CStyle) {
		write_line("#endif // {}", def_macro);
	}

	return Output;
}

std::vector<cpp_enum_generated> cpp_generator::generate_enums(const cpp_enum_config_final& cfg)
{
	throw std::logic_error("Unimplemented");
}
