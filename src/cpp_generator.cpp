#include "cpp_generator.h"
#include <vector>
#include <set>

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

struct Int128FormatValue
{
	int128 value;
	int64_t bits;
	bool bIsSigned;
};

// https://wgml.pl/blog/formatting-user-defined-types-fmt.html
template <>
struct fmt::formatter<Int128FormatValue> {
	template<typename ParseContext>
	constexpr auto parse(ParseContext& ctx)
	{
		return ctx.begin();
	}

	auto format(Int128FormatValue c, format_context& ctx) {
		if (c.bIsSigned)
		{
			// If a value is the exact minimum of its storage type, we need to output it as an expression
			// because negative literals are actually a positive literal with a unary minus applied to them.
			// I'm not making this up. Fantastic language design.
			// https://stackoverflow.com/a/11270104
			const bool bIsMinValueForType =
				(c.bits == 8 && c.value == INT8_MIN) ||
				(c.bits == 16 && c.value == INT16_MIN) ||
				(c.bits == 32 && c.value == INT32_MIN) ||
				(c.bits == 64 && c.value == INT64_MIN);
			if (bIsMinValueForType)
			{
				return fmt::format_to(ctx.out(), "({0} - 1)", static_cast<int64_t>(c.value + 1));
			}

			return fmt::format_to(ctx.out(), "{0}", static_cast<int64_t>(c.value));
		}
		else
		{
			const uint64_t value = static_cast<uint64_t>(c.value);
			if (value > 0xFF)
			{
				// Note: I find it disappointing that 0XFF {:#X} and 0xff {:#x} are fmt options,
				// but not 0xFF which I think is the most readable format :)
				return fmt::format_to(ctx.out(), "0x{0:X}", value);
			}
			else
			{
				return fmt::format_to(ctx.out(), "{0}", value);
			}
		}
	}
};

const uint64_t get_flags_enum_value(const FlagsEnumDefaultValueStyle& style, const enum_definition& definition)
{
	switch (style)
	{
	case FlagsEnumDefaultValueStyle::Zero: return 0;
	case FlagsEnumDefaultValueStyle::Min:
	{
		auto m = std::min_element(definition.values.begin(), definition.values.end(),
			[](const enum_entry& lhs, const enum_entry& rhs) { return lhs.p_value < rhs.p_value; });
		if (m != definition.values.end())
		{
			return static_cast<uint64_t>(m->p_value);
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
			[](const enum_entry& lhs, const enum_entry& rhs) { return lhs.p_value < rhs.p_value; });
		if (m != definition.values.end())
		{
			return static_cast<uint64_t>(m->p_value);
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
			bits |= static_cast<uint64_t>(v.p_value);
		}
		return bits;
	}
	case FlagsEnumDefaultValueStyle::First:
	{
		return static_cast<uint64_t>(definition.values.front().p_value);
	}
	case FlagsEnumDefaultValueStyle::Last:
	{
		return static_cast<uint64_t>(definition.values.back().p_value);
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
			[](const enum_entry& lhs, const enum_entry& rhs) { return lhs.p_value < rhs.p_value; });
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
			[](const enum_entry& lhs, const enum_entry& rhs) { return lhs.p_value < rhs.p_value; });
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
constexpr uint64_t log_2_unsigned(uint128 x)
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

// Number of bits required to store an unsigned value
constexpr uint64_t unsigned_bits_required(uint128 x)
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

bool is_value_set_contiguous(const std::set<int128> values)
{
	int128 value = *values.begin();
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

bool is_flags_set_contiguous(const std::set<int128> flags)
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
	write_line("// THIS FILE WAS GENERATED BY A TOOL (haha)");
	write_line("// Direct your feedback and monetary donations to: https://github.com/Scaless/enumbra");
	write_line("// It is highly recommended to not make manual edits to this file, as they will be overwritten");
	write_line("// when the file is re-generated.");
	if (cfg.cpp_config.time_generated_in_header)
	{
		std::time_t time_point = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		char time_buf[255];
		ctime_s(time_buf, 255, &time_point);
		write_line("// Generated by enumbra v{} on {}", kEnumbraVersion, time_buf);
	}
	else
	{
		write_line("// Generated by enumbra v{}", kEnumbraVersion);
		write_linefeed();
	}

	// Custom preamble
	for (const auto& line : cpp.preamble_text) {
		write_line(line);
	}
	if (cpp.preamble_text.size() == 0) {
		write_line("// You don't have any preamble_text set. If you have a license you want to apply to your");
		write_line("// generated code, you should put it in your enumbra_config.json file!");
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
		// Increment this if macros below are modified.
		const int enumbra_required_macros_version = 3;
		std::vector<const char*> macro_strings = {
			{ "#if !defined(ENUMBRA_REQUIRED_MACROS_VERSION)" },
			{ "#define ENUMBRA_REQUIRED_MACROS_VERSION {1}" },
			{ "" },
			{ "// Find out what language version we're using"},
			{ "#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 202002L)) || (__cplusplus >= 202002L)"},
			{ "#define ENUMBRA_CPP_VERSION 20"},
			{ "#elif ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)) || (__cplusplus >= 201703L)"},
			{ "#define ENUMBRA_CPP_VERSION 17"},
			{ "#elif ((defined(_MSVC_LANG) && _MSVC_LANG >= 201402L)) || (__cplusplus >= 201402L)"},
			{ "#define ENUMBRA_CPP_VERSION 14"},
			{ "#elif ((defined(_MSC_VER) && _MSC_VER >= 1700 /*VS2012*/)) || (__cplusplus >= 201103L)"},
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

	// OPTIONAL MACRO DEFINITIONS
	if (cfg.cpp_config.enumbra_bitfield_macros)
	{
		// Increment this if macros below are modified.
		const int enumbra_macros_version = 2;
		std::vector<const char*> macro_strings = {
			{ "#if !defined(ENUMBRA_OPTIONAL_MACROS_VERSION)" },
			{ "#define ENUMBRA_OPTIONAL_MACROS_VERSION {1}" },
			{ "" },
			{ "// Bit field storage helper" },
			{ "#define ENUMBRA_PACK_UNINITIALIZED(Enum, Name) Enum::Value Name : Enum::bits_required_storage();" },
			{ "#define ENUMBRA_INIT(Name, InitValue) Name(enumbra::enumbra_base_t<decltype(Name)>(InitValue).value())" },
			{ "#define ENUMBRA_INIT_DEFAULT(Name) Name(enumbra::enumbra_base_t<decltype(Name)>::default_value())" },
			{ "" },
			{ "#if ENUMBRA_CPP_VERSION >= 20" },
			{ "// Bit field storage helper with type-checked member initialization" },
			{ "#define ENUMBRA_PACK_INIT(Enum, Name, InitValue) Enum::Value Name : Enum::bits_required_storage() {{ enumbra::enumbra_base_t<Enum>(InitValue).value() }};" },
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

	// TEMPLATES
	{
		// Increment this if templates below are modified.
		const int base_template_version = 3;
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
			{ "{0}{0}template<class T> constexpr bool streq(T* a, T* b) {{ return *a == *b && (*a == '\\0' || streq(a + 1, b + 1)); }}"},
			{ "{0}}} // end namespace enumbra::detail" },
			{ "{0}template<class T> using enumbra_base_t = typename detail::enumbra_base_helper<T>::base_type;" },
			{ "{0}template<class T> constexpr bool is_enumbra_type() {{ return detail::enumbra_base_helper<T>::enumbra_type; }}"},
			{ "{0}template<class T> constexpr bool is_enumbra_type(T) {{ return detail::enumbra_base_helper<T>::enumbra_type; }}"},
			{ "{0}template<class T> constexpr bool is_enumbra_struct() {{ return is_enumbra_type<T>() && !detail::enumbra_base_helper<T>::enumbra_enum_class; }}"},
			{ "{0}template<class T> constexpr bool is_enumbra_struct(T) {{ return is_enumbra_type<T>() && !detail::enumbra_base_helper<T>::enumbra_enum_class; }}"},
			{ "{0}template<class T> constexpr bool is_enumbra_scoped_enum() {{ return is_enumbra_type<T>() && detail::enumbra_base_helper<T>::enumbra_enum_class; }}"},
			{ "{0}template<class T> constexpr bool is_enumbra_scoped_enum(T) {{ return is_enumbra_type<T>() && detail::enumbra_base_helper<T>::enumbra_enum_class; }}"},
			{ "{0}template<class T> constexpr bool is_enumbra_value_enum() {{ return is_enumbra_type<T>() && detail::enumbra_base_helper<T>::enumbra_value_enum; }}"},
			{ "{0}template<class T> constexpr bool is_enumbra_value_enum(T) {{ return is_enumbra_type<T>() && detail::enumbra_base_helper<T>::enumbra_value_enum; }}"},
			{ "{0}template<class T> constexpr bool is_enumbra_flags_enum() {{ return is_enumbra_type<T>() && detail::enumbra_base_helper<T>::enumbra_flags_enum; }}"},
			{ "{0}template<class T> constexpr bool is_enumbra_flags_enum(T) {{ return is_enumbra_type<T>() && detail::enumbra_base_helper<T>::enumbra_flags_enum; }}"},
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
		const std::string size_type = cpp.get_size_type_from_index(e.size_type_index).type_name;
		const bool is_size_type_signed = cpp.get_size_type_from_index(e.size_type_index).is_signed;
		const int64_t type_bits = cpp.get_size_type_from_index(e.size_type_index).bits;
		const std::string char_type = cfg.cpp_config.string_table_type == StringTableType::ConstCharPtr ? "const char*" : "const wchar_t*";
		const std::string string_literal_prefix = cfg.cpp_config.string_table_type == StringTableType::ConstCharPtr ? "" : "L";
		const std::string string_function_prefix = cfg.cpp_config.string_table_type == StringTableType::ConstCharPtr ? "" : "w";

		const int64_t max_abs_representable_signed = std::max(std::abs(static_cast<int64_t>(min_entry.p_value) - 1), static_cast<int64_t>(max_entry.p_value));
		const uint64_t max_abs_representable = is_size_type_signed ? max_abs_representable_signed : static_cast<uint64_t>(max_entry.p_value);
			
		size_t bits_required_storage = unsigned_bits_required(max_abs_representable);
		const size_t bits_required_transmission = unsigned_bits_required(max_entry.p_value - min_entry.p_value);

		// Because of the way signed integers map to bit fields, a bit field may require an additional
		// bit of storage to accomodate the sign bit even if it is unused. For example, given the following enum:
		//   enum class ESignedValueBits : int8_t { A = 0, B = 1, C = 2, D = 3 }
		// To properly store and assign to this enum, we need 3 bits:
		//   int8_t Value : 1; // maps to the range -1 - 0, unexpected!
		//   int8_t Value : 2; // maps to the range -2 - 1, still not big enough
		//   int8_t Value : 3; // maps to the range -4 - 3, big enough but we're wasting space
		// For this reason, when utilizing packed enums it is recommended to always prefer an unsigned underlying
		// type unless your enum actually contains negative values.
		if (is_size_type_signed && (max_entry.p_value > 0)) {
			uint64_t signed_range_max = 0;
			for (int i = 0; i < bits_required_storage - 1; i++) {
				signed_range_max |= 1ULL << i;
			}
			if (static_cast<uint64_t>(max_entry.p_value) > signed_range_max) {
				bits_required_storage += 1;
			}
		}

		// Determine if all values are unique, or if some enum value names overlap.
		// TODO: Enforce if flag is set
		std::set<int128> unique_values;
		for (auto& v : e.values) { unique_values.insert(v.p_value); }
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
				write_line_tabbed(2, "{} = {},", v.name, Int128FormatValue{ v.p_value, type_bits, is_size_type_signed });
			}
			write_line_tabbed(1, "}};");
			write_linefeed();

			write_line_tabbed(1, "constexpr {}() : value_(Value({})) {{ }}", e.name, Int128FormatValue{ default_entry.p_value, type_bits, is_size_type_signed });
			write_line_tabbed(1, "constexpr {}(Value v) : value_(v) {{ }}", e.name);
			write_linefeed();

			for (const auto& v : e.values) {
				write_line_tabbed(1, "constexpr static Value {0} = Value::{0};", v.name);
			}
			write_linefeed();

			// Values Array
			write_line_tabbed(1, "constexpr static std::array<Value, {}> Values = {{{{", entry_count);
			const int MaxValuesWidth = 120;
			size_t CurrentValuesWidth = 8;
			write_tab(2);
			for (const auto& v : e.values) {
				CurrentValuesWidth += v.name.length() + 2;
				if (CurrentValuesWidth < MaxValuesWidth) {
					write("{}, ", v.name);
				}
				else {
					write_line_tabbed(2, "{},", v.name);
					CurrentValuesWidth = 8;
				}
			}
			write_linefeed();
			write_line_tabbed(1, "}}}};");
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
			write_line_tabbed(1, "static constexpr {0}::Value default_value() {{ return Value({1}); }}", e.name, Int128FormatValue{ default_entry.p_value, type_bits, is_size_type_signed });
			write_line_tabbed(1, "static constexpr {1} min() {{ return {2}; }}", e.name, size_type, Int128FormatValue{ min_entry.p_value, type_bits, is_size_type_signed });
			write_line_tabbed(1, "static constexpr {1} max() {{ return {2}; }}", e.name, size_type, Int128FormatValue{ max_entry.p_value, type_bits, is_size_type_signed });
			write_line_tabbed(1, "static constexpr int count() {{ return {1}; }}", e.name, unique_entry_count);
			write_line_tabbed(1, "static constexpr bool is_contiguous() {{ return {1}; }}", e.name, (is_contiguous ? "true" : "false"));
			write_line_tabbed(1, "static constexpr {0} from_underlying_unsafe({1} v) {{ return {0}(static_cast<Value>(v)); }}", e.name, size_type);
			write_line_tabbed(1, "static constexpr {1} bits_required_storage() {{ return {2}; }}", e.name, size_type, bits_required_storage);
			write_line_tabbed(1, "static constexpr {1} bits_required_transmission() {{ return {2}; }}", e.name, size_type, bits_required_transmission);

			// is_valid variations
			if (is_contiguous)
			{
				if ((min_entry.p_value == 0) && !is_size_type_signed) // Unsigned values can't go below 0 so we just need to check that we're <= max
				{
					write_line_tabbed(1, "static constexpr bool is_valid({0} v) {{ return static_cast<{1}>(v.value_) <= {2}; }}", e.name, size_type, Int128FormatValue{ max_entry.p_value, type_bits, is_size_type_signed });
					write_line_tabbed(1, "static constexpr bool is_valid({1} v) {{ return v <= {2}; }}", e.name, size_type, Int128FormatValue{ max_entry.p_value, type_bits, is_size_type_signed });
				}
				else
				{
					write_line_tabbed(1, "static constexpr bool is_valid({0} v) {{ return ({2} <= static_cast<{1}>(v.value_)) && (static_cast<{1}>(v.value_) <= {3}); }}", e.name, size_type, Int128FormatValue{ min_entry.p_value, type_bits, is_size_type_signed }, Int128FormatValue{ max_entry.p_value, type_bits, is_size_type_signed });
					write_line_tabbed(1, "static constexpr bool is_valid({1} v) {{ return ({2} <= v) && (v <= {3}); }}", e.name, size_type, Int128FormatValue{ min_entry.p_value, type_bits, is_size_type_signed }, Int128FormatValue{ max_entry.p_value, type_bits, is_size_type_signed });
				}
			}
			else
			{
				write_line_tabbed(1, "static ENUMBRA_CONSTEXPR_NONCONSTFUNC bool is_valid({0} v) {{ for(std::size_t i = 0; i < Values.size(); i++) {{ auto& val = Values[i]; if(val == v) return true; }} return false; }}", e.name);
				write_line_tabbed(1, "static ENUMBRA_CONSTEXPR_NONCONSTFUNC bool is_valid({1} v) {{ for(std::size_t i = 0; i < Values.size(); i++) {{ auto& val = Values[i]; if(val == Value(v)) return true; }} return false; }}", e.name, size_type);
			}
			write_linefeed();

			// String Functions
			write_line_tabbed(1, "static ENUMBRA_CONSTEXPR_NONCONSTFUNC {0} to_{1}string(const {2}::Value v) {{", char_type, string_function_prefix, e.name);
			write_line_tabbed(2, "switch (v) {{");
			for (auto v : e.values) {
				write_line_tabbed(3, "case {0}: return {1}\"{0}\";", v.name, string_literal_prefix);
			}
			write_line_tabbed(2, "}}");
			write_line_tabbed(2, "return {0}\"\";", string_literal_prefix);
			write_line_tabbed(1, "}}");

			// MSVC:C28020 complains about the comparision in the loop because it effectively expands to 0 <= x <= 0
			if (e.values.size() == 1)
			{
				write_line_tabbed(1, "static ENUMBRA_CONSTEXPR_NONCONSTFUNC std::pair<bool, Value> from_{1}string({2} str) {{", e.name, string_function_prefix, char_type);
				write_line_tabbed(2, "if (enumbra::detail::streq(string_lookup_[0].second, str)) {{");
				write_line_tabbed(3, "return std::make_pair(true, string_lookup_[0].first);");
				write_line_tabbed(2, "}}");
				write_line_tabbed(2, "return std::make_pair(false, default_value());", e.name);
				write_line_tabbed(1, "}}");
			}
			else
			{
				write_line_tabbed(1, "static ENUMBRA_CONSTEXPR_NONCONSTFUNC std::pair<bool, Value> from_{1}string({2} str) {{", e.name, string_function_prefix, char_type);
				write_line_tabbed(2, "for (std::size_t i = 0; i < string_lookup_.size(); i++) {{");
				write_line_tabbed(3, "if (enumbra::detail::streq(string_lookup_[i].second, str)) {{");
				write_line_tabbed(4, "return std::make_pair(true, string_lookup_[i].first);");
				write_line_tabbed(3, "}}");
				write_line_tabbed(2, "}}");
				write_line_tabbed(2, "return std::make_pair(false, default_value());", e.name);
				write_line_tabbed(1, "}}");
			}

			// Private Members
			write_linefeed();
			write_line("private:");
			write_line_tabbed(1, "Value value_;");

			// Value String Table
			write_line_tabbed(1, "constexpr static std::array<std::pair<Value,{0}>, {1}> string_lookup_ = {{{{", char_type, entry_count);
			for (const auto& v : e.values) {
				write_line_tabbed(2, "std::make_pair({1}, {0}\"{1}\"),", string_literal_prefix, v.name, size_type);
			}
			write_line_tabbed(1, "}}}};");

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
		std::set<int128> unique_values;
		for (auto& v : e.values) { unique_values.insert(v.p_value); }
		const size_t unique_entry_count = unique_values.size();
		if (e.values.size() != unique_values.size())
		{
			throw std::logic_error("ENUM DEFINITIONS Check 3: Values in enum do not have unique values (enum name = " + e.name + ")");
		}

		// Get references and metadata for relevant enum values that we will need
		const uint64_t min_value = 0; // The minimum for a flags entry is always 0 - no bits set
		uint64_t max_value = 0;
		for (auto& v : e.values) {
			if (v.p_value < 0) {
				throw std::logic_error("ENUM DEFINITIONS Check 4: Flags-Enum value is less than 0. Flags-Enum values are required to be unsigned. (enum name = " + e.name + ")");
			}
			max_value |= static_cast<uint64_t>(v.p_value);
		}

		const uint64_t default_value = get_flags_enum_value(enum_meta.flags_enum_default_value_style, e);

		const size_t entry_count = e.values.size();
		const size_t bits_required_storage = unsigned_bits_required(max_value);
		const size_t bits_required_transmission = bits_required_storage;
		const std::string size_type = cpp.get_size_type_from_index(e.size_type_index).type_name;
		const bool is_size_type_signed = cpp.get_size_type_from_index(e.size_type_index).is_signed;
		const int64_t type_bits = cpp.get_size_type_from_index(e.size_type_index).bits;
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
				write_line_tabbed(2, "{} = {},", v.name, Int128FormatValue{ v.p_value, type_bits, is_size_type_signed });
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

			// Values Array
			write_line_tabbed(1, "constexpr static std::array<Value, {}> Values = {{{{", entry_count);
			const int MaxValuesWidth = 120;
			size_t CurrentValuesWidth = 8;
			write_tab(2);
			for (const auto& v : e.values) {
				CurrentValuesWidth += v.name.length() + 2;
				if (CurrentValuesWidth < MaxValuesWidth) {
					write("{}, ", v.name);
				}
				else {
					write_line_tabbed(2, "{},", v.name);
					CurrentValuesWidth = 8;
				}
			}
			write_linefeed();
			write_line_tabbed(1, "}}}};");
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
			write_line_tabbed(1, "ENUMBRA_CONSTEXPR_NONCONSTFUNC bool is_single() const {{ {0} n = static_cast<{0}>(value_); return n && !(n & (n - 1)); }}", size_type);
			write_linefeed();

			// Introspection
			write_line_tabbed(1, "static constexpr {0}::Value default_value() {{ return Value({1}); }}", e.name, default_value);
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

			// Just not possible without being able to do non-const reference to bit field
			//{"ENUMBRA_CONSTEXPR_NONCONSTFUNC void operator|=({0}::Value a, const {0}::Value b) {{ a = static_cast<{0}::Value>(static_cast<{1}>(a) | static_cast<{1}>(b)); }}"},
			//{"ENUMBRA_CONSTEXPR_NONCONSTFUNC void operator&=({0}::Value a, const {0}::Value b) {{ a = static_cast<{0}::Value>(static_cast<{1}>(a) & static_cast<{1}>(b)); }}"},
			//{"ENUMBRA_CONSTEXPR_NONCONSTFUNC void operator^=({0}::Value a, const {0}::Value b) {{ a = static_cast<{0}::Value>(static_cast<{1}>(a) ^ static_cast<{1}>(b)); }}"},

			{"constexpr {0} operator~(const {0} a) {{ return ~a.value(); }}"},
			{"constexpr {0} operator|(const {0} a, const {0} b) {{ return a.value() | b.value(); }}"},
			{"constexpr {0} operator&(const {0} a, const {0} b) {{ return a.value() & b.value(); }}"},
			{"constexpr {0} operator^(const {0} a, const {0} b) {{ return a.value() ^ b.value(); }}"},

			// Unnecessary? Value can implicitly construct parent class.
			//{"constexpr {0} operator|(const {0} a, const {0}::Value b) {{ return a.value() | b; }}"},
			//{"constexpr {0} operator&(const {0} a, const {0}::Value b) {{ return a.value() & b; }}"},
			//{"constexpr {0} operator^(const {0} a, const {0}::Value b) {{ return a.value() ^ b; }}"},
			//{"constexpr {0} operator|(const {0}::Value a, const {0} b) {{ return a | b.value(); }}"},
			//{"constexpr {0} operator&(const {0}::Value a, const {0} b) {{ return a & b.value(); }}"},
			//{"constexpr {0} operator^(const {0}::Value a, const {0} b) {{ return a ^ b.value(); }}"},

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
