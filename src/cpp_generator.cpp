#include "cpp_generator.h"
#include <vector>
#include <set>

using namespace enumbra;
using namespace enumbra::cpp;

std::string to_upper(const std::string& str)
{
	std::string copy = str;
	for (auto& c : copy) {
		c = std::toupper(c, std::locale("en_US.utf8"));
	}
	return copy;
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
			// TODO: Decide on a consistent behavior to format here, or make it customizable?
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

uint64_t get_flags_enum_value(const FlagsEnumDefaultValueStyle& style, const enum_definition& definition)
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

enum_entry get_value_enum_entry(const ValueEnumDefaultValueStyle& style, const enum_definition& definition)
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
constexpr uint64_t get_storage_bits_required(uint128 x)
{
	return log_2_unsigned(x) + 1;
}

// Number of bits required to transmit a range of values
constexpr uint64_t get_transmission_bits_required(uint128 x)
{
	// Special case for a single value, which requires no bits to transmit
	if (x == 0)
	{
		return 0;
	}

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
	int64_t check_bit = get_storage_bits_required(*flags.begin());
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
    case enumbra::cpp::IncludeGuardStyle::None:
    default:
        break;
	}

	// INCLUDES
	write_line("#include <array>");
	for (const auto& inc : cpp.additional_includes) {
		write_line("#include {}", inc);
	}

	// REQUIRED MACROS
	{
		// Increment this if macros below are modified.
		const int enumbra_required_macros_version = 7;
		const std::string macro_strings = R"(
#if !defined(ENUMBRA_REQUIRED_MACROS_VERSION) 
#define ENUMBRA_REQUIRED_MACROS_VERSION {0}

// Find out what language version we're using
// 2024-07-04:MSVC Doesn't officially support C++23 yet
#if (__cplusplus >= 202302L)
#define ENUMBRA_CPP_VERSION 23
#elif ((defined(_MSVC_LANG) && _MSVC_LANG >= 202002L)) || (__cplusplus >= 202002L)
#define ENUMBRA_CPP_VERSION 20
#elif ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)) || (__cplusplus >= 201703L)
#define ENUMBRA_CPP_VERSION 17
#else
#error Headers generated by enumbra require a compiler that supports C++17 or higher.
#endif

#if defined(__clang__)
#define ENUMBRA_COMPILER_CLANG
#elif defined(__GNUG__)
#define ENUMBRA_COMPILER_GCC
#elif defined(_MSC_VER)
#define ENUMBRA_COMPILER_MSVC
#else
#define ENUMBRA_COMPILER_UNKNOWN
#endif

#if defined(ENUMBRA_USER_UNREACHABLE)
#define ENUMBRA_UNREACHABLE ENUMBRA_USER_UNREACHABLE
#else
#define ENUMBRA_UNREACHABLE
#endif

#else // check existing version supported 
#if (ENUMBRA_REQUIRED_MACROS_VERSION + 0) == 0 
#error ENUMBRA_REQUIRED_MACROS_VERSION has been defined without a proper version number. Check your build system. 
#elif (ENUMBRA_REQUIRED_MACROS_VERSION + 0) < {0} 
#error An included header was generated using a newer version of enumbra. Regenerate your headers using the same version. 
#elif (ENUMBRA_REQUIRED_MACROS_VERSION + 0) > {0} 
#error An included header was generated using an older version of enumbra. Regenerate your headers using the same version. 
#endif // end check existing version supported
#endif // ENUMBRA_REQUIRED_MACROS_VERSION)";

		write(macro_strings, enumbra_required_macros_version);
		write_linefeed();
	}

	// OPTIONAL MACRO DEFINITIONS
	if (cfg.cpp_config.enumbra_bitfield_macros)
	{
		// Increment this if macros below are modified.
		const int enumbra_optional_macros_version = 6;
		std::string macro_strings = R"(
#if !defined(ENUMBRA_OPTIONAL_MACROS_VERSION)
#define ENUMBRA_OPTIONAL_MACROS_VERSION {0}

// Bitfield convenience functions
#define ENUMBRA_ZERO(Field) {{ decltype(Field) _field_ = Field; zero(_field_); Field = _field_; }}
#define ENUMBRA_SET(Field, Value) {{ decltype(Field) _field_ = Field; set(_field_, Value); Field = _field_; }}
#define ENUMBRA_UNSET(Field, Value) {{ decltype(Field) _field_ = Field; unset(_field_, Value); Field = _field_; }}
#define ENUMBRA_TOGGLE(Field, Value) {{ decltype(Field) _field_ = Field; toggle(_field_, Value); Field = _field_; }}

// Bit field storage helper
#define ENUMBRA_PACK_UNINITIALIZED(Enum, Name) Enum Name : ::enumbra::bits_required_storage<Enum>();
#define ENUMBRA_INIT(Name, InitValue) Name(::enumbra::default_value<decltype(Name)>())
#define ENUMBRA_INIT_DEFAULT(Name) Name(::enumbra::default_value<decltype(Name)>())

#if ENUMBRA_CPP_VERSION >= 20
// Bit field storage helper with type-checked member initialization
#define ENUMBRA_PACK_INIT(Enum, Name, InitValue) Enum Name : ::enumbra::bits_required_storage<Enum>() {{ InitValue }};
// Bit field storage helper with default value initialization
#define ENUMBRA_PACK_INIT_DEFAULT(Enum, Name) Enum Name : ::enumbra::bits_required_storage<Enum>() {{ ::enumbra::default_value<Enum>() }};
#endif

#else // check existing version supported
#if (ENUMBRA_OPTIONAL_MACROS_VERSION + 0) == 0
#error ENUMBRA_OPTIONAL_MACROS_VERSION has been defined without a proper version number. Check your build system.
#elif (ENUMBRA_OPTIONAL_MACROS_VERSION + 0) < {0}
#error An included header was generated using a newer version of enumbra. Regenerate your headers using the same version.
#elif (ENUMBRA_OPTIONAL_MACROS_VERSION + 0) > {0}
#error An included header was generated using an older version of enumbra. Regenerate your headers using the same version.
#endif // end check existing version supported
#endif // ENUMBRA_OPTIONAL_MACROS_VERSION)";

		write(macro_strings, enumbra_optional_macros_version);
		write_linefeed();
	}

	// TEMPLATES
	{
		// Increment this if templates below are modified.
		const int enumbra_templates_version = 9;
		const std::string str_templates = R"(
#if !defined(ENUMBRA_BASE_TEMPLATES_VERSION)
#define ENUMBRA_BASE_TEMPLATES_VERSION {0}
namespace enumbra {{
    namespace detail {{
        // Type info
        template<bool is_enumbra, bool is_value_enum, bool is_flags_enum>
        struct type_info {{ 
            static constexpr bool enumbra_type = is_enumbra;
            static constexpr bool enumbra_value_enum = is_value_enum;
            static constexpr bool enumbra_flags_enum = is_flags_enum;
        }};

        // Value enum info
        template<typename underlying_type, underlying_type min_v, underlying_type max_v,
            underlying_type default_v, typename count_type, count_type count_v,
            bool is_contiguous_v, int bits_required_storage_v, int bits_required_transmission_v>
        struct value_enum_info {{
            using underlying_t = underlying_type;
            static constexpr underlying_type min = min_v;
            static constexpr underlying_type max = max_v;
            static constexpr underlying_type def = default_v;
            static constexpr count_type count = count_v;
            static constexpr bool is_contiguous = is_contiguous_v;
            static constexpr int bits_required_storage = bits_required_storage_v;
            static constexpr int bits_required_transmission = bits_required_transmission_v;
        }};

        // Flags enum info
        template<typename underlying_type, underlying_type min_v, underlying_type max_v, 
            underlying_type default_v, typename count_type, count_type count_v,
            bool is_contiguous_v, int bits_required_storage_v, int bits_required_transmission_v>
        struct flags_enum_info {{
            using underlying_t = underlying_type;
            static constexpr underlying_type min = min_v;
            static constexpr underlying_type max = max_v;
            static constexpr underlying_type default_value = default_v;
            static constexpr count_type count = count_v;
            static constexpr bool is_contiguous = is_contiguous_v;
            static constexpr int bits_required_storage = bits_required_storage_v;
            static constexpr int bits_required_transmission = bits_required_transmission_v;
        }};
        
        // Default template for non-enumbra types
        template<class T>
        struct base_helper : type_info<false, false, false> {{ }};
        template<class T>
        struct value_enum_helper;
        template<class T>
        struct flags_enum_helper;

        // Constexpr string compare
        template<class T> constexpr bool streq_s(T* a, std::size_t a_len, T* b, std::size_t b_len) {{
            if(a == nullptr) {{ return false; }}
            if(b == nullptr) {{ return false; }}
            if(a_len != b_len) {{ return false; }}
#if defined(ENUMBRA_COMPILER_CLANG)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#endif
            for(std::size_t i = 0; i < a_len; ++i) {{ if(a[i] != b[i]) {{ return false; }} }}
#if defined(ENUMBRA_COMPILER_CLANG)
#pragma clang diagnostic pop
#endif
            return true;
        }}
        template<class T> constexpr bool streq_known_size(T* a, T* b, std::size_t len) {{
            if(a == nullptr) {{ return false; }}
            if(b == nullptr) {{ return false; }}
#if defined(ENUMBRA_COMPILER_CLANG)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#endif
            for(std::size_t i = 0; i < len; ++i) {{ if(a[i] != b[i]) {{ return false; }} }}
#if defined(ENUMBRA_COMPILER_CLANG)
#pragma clang diagnostic pop
#endif
            return true;
        }}

        struct string_table_entry
        {{
            uint32_t offset = 0;
            uint32_t count = 0;
        }};
    }} // end namespace enumbra::detail
    template<class T> constexpr bool is_enumbra_enum() {{ return detail::base_helper<T>::enumbra_type; }}
    template<class T> constexpr bool is_enumbra_enum(T) {{ return detail::base_helper<T>::enumbra_type; }}
    template<class T> constexpr bool is_enumbra_value_enum() {{ return is_enumbra_enum<T>() && detail::base_helper<T>::enumbra_value_enum; }}
    template<class T> constexpr bool is_enumbra_value_enum(T) {{ return is_enumbra_enum<T>() && detail::base_helper<T>::enumbra_value_enum; }}
    template<class T> constexpr bool is_enumbra_flags_enum() {{ return is_enumbra_enum<T>() && detail::base_helper<T>::enumbra_flags_enum; }}
    template<class T> constexpr bool is_enumbra_flags_enum(T) {{ return is_enumbra_enum<T>() && detail::base_helper<T>::enumbra_flags_enum; }}
    
    template<class T, typename std::enable_if<is_enumbra_value_enum<T>(), T>::type* = nullptr>
    constexpr T min() {{ return static_cast<T>(detail::value_enum_helper<T>::min); }}
    template<class T, typename std::enable_if<is_enumbra_flags_enum<T>(), T>::type* = nullptr>
    constexpr T min() {{ return static_cast<T>(detail::flags_enum_helper<T>::min); }}
    template<class T, typename std::enable_if<!is_enumbra_enum<T>(), T>::type* = nullptr>
    constexpr T min() = delete;

    template<class T, typename std::enable_if<is_enumbra_value_enum<T>(), T>::type* = nullptr>
    constexpr T max() {{ return static_cast<T>(detail::value_enum_helper<T>::max); }}
    template<class T, typename std::enable_if<is_enumbra_flags_enum<T>(), T>::type* = nullptr>
    constexpr T max() {{ return static_cast<T>(detail::flags_enum_helper<T>::max); }}
    template<class T, typename std::enable_if<!is_enumbra_enum<T>(), T>::type* = nullptr>
    constexpr T max() = delete;

    template<class T, typename std::enable_if<is_enumbra_value_enum<T>(), T>::type* = nullptr>
    constexpr T default_value() {{ return static_cast<T>(detail::value_enum_helper<T>::default_value); }}
    template<class T, typename std::enable_if<is_enumbra_flags_enum<T>(), T>::type* = nullptr>
    constexpr T default_value() {{ return static_cast<T>(detail::flags_enum_helper<T>::default_value); }}
    template<class T, typename std::enable_if<!is_enumbra_enum<T>(), T>::type* = nullptr>
    constexpr T default_value() = delete;

    template<class T, typename std::enable_if<is_enumbra_value_enum<T>(), T>::type* = nullptr>
    constexpr int count() {{ return detail::value_enum_helper<T>::count; }}
    template<class T, typename std::enable_if<is_enumbra_flags_enum<T>(), T>::type* = nullptr>
    constexpr int count() {{ return detail::flags_enum_helper<T>::count; }}
    template<class T, typename std::enable_if<!is_enumbra_enum<T>(), T>::type* = nullptr>
    constexpr int count() = delete;

    template<class T, typename std::enable_if<is_enumbra_value_enum<T>(), T>::type* = nullptr>
    constexpr bool is_contiguous() {{ return detail::value_enum_helper<T>::is_contiguous; }}
    template<class T, typename std::enable_if<is_enumbra_flags_enum<T>(), T>::type* = nullptr>
    constexpr bool is_contiguous() {{ return detail::flags_enum_helper<T>::is_contiguous; }}
    template<class T, typename std::enable_if<!is_enumbra_enum<T>(), T>::type* = nullptr>
    constexpr bool is_contiguous() = delete;

    template<class T, typename std::enable_if<is_enumbra_value_enum<T>(), T>::type* = nullptr>
    constexpr int bits_required_storage() {{ return detail::value_enum_helper<T>::bits_required_storage; }}
    template<class T, typename std::enable_if<is_enumbra_flags_enum<T>(), T>::type* = nullptr>
    constexpr int bits_required_storage() {{ return detail::flags_enum_helper<T>::bits_required_storage; }}
    template<class T, typename std::enable_if<!is_enumbra_enum<T>(), T>::type* = nullptr>
    constexpr int bits_required_storage() = delete;

    template<class T, typename std::enable_if<is_enumbra_value_enum<T>(), T>::type* = nullptr>
    constexpr int bits_required_transmission() {{ return detail::value_enum_helper<T>::bits_required_transmission; }}
    template<class T, typename std::enable_if<is_enumbra_flags_enum<T>(), T>::type* = nullptr>
    constexpr int bits_required_transmission() {{ return detail::flags_enum_helper<T>::bits_required_transmission; }}
    template<class T, typename std::enable_if<!is_enumbra_enum<T>(), T>::type* = nullptr>
    constexpr int bits_required_transmission() = delete;

    template<class T, class underlying_type = typename detail::base_helper<T>::base_type, typename std::enable_if<is_enumbra_enum<T>(), T>::type* = nullptr>
    constexpr T from_underlying_unsafe(underlying_type e) {{ return static_cast<T>(e); }}
    template<class T, class underlying_type = typename detail::base_helper<T>::base_type, typename std::enable_if<!is_enumbra_enum<T>(), T>::type* = nullptr>
    constexpr T from_underlying_unsafe(underlying_type e) = delete;

    template<class T, class underlying_type = typename detail::value_enum_helper<T>::underlying_t, typename std::enable_if<is_enumbra_value_enum<T>(), T>::type* = nullptr>
    constexpr underlying_type to_underlying(T e) {{ return static_cast<underlying_type>(e); }}
    template<class T, class underlying_type = typename detail::flags_enum_helper<T>::underlying_t, typename std::enable_if<is_enumbra_flags_enum<T>(), T>::type* = nullptr>
    constexpr underlying_type to_underlying(T e) {{ return static_cast<underlying_type>(e); }}
    template<class T, class underlying_type = T, typename std::enable_if<!is_enumbra_enum<T>(), T>::type* = nullptr>
    constexpr underlying_type to_underlying(T e) = delete;

    template<class T>
    struct from_string_result
    {{
        bool success;
        T result;
    }};

    template<class T>
    struct from_wstring_result
    {{
        bool success;
        T result;
    }};

}} // end namespace enumbra
#else // check existing version supported
#if (ENUMBRA_BASE_TEMPLATES_VERSION + 0) == 0
#error ENUMBRA_BASE_TEMPLATES_VERSION has been defined without a proper version number. Check your build system.
#elif (ENUMBRA_BASE_TEMPLATES_VERSION + 0) < {0}
#error An included header was generated using a newer version of enumbra. Regenerate your headers using same version of enumbra.
#elif (ENUMBRA_BASE_TEMPLATES_VERSION + 0) > {0}
#error An included header was generated using an older version of enumbra. Regenerate your headers using same version of enumbra.
#endif // check existing version supported
#endif // ENUMBRA_BASE_TEMPLATES_VERSION)";

		write_line(str_templates, enumbra_templates_version);
		write_linefeed();
	}

	std::vector<std::string> template_specializations;

	// Construct the full namespace for templates
	std::string full_ns;
	for (auto& ns : cpp.output_namespace) {
		full_ns += ns + "::";
	}

	// START NAMESPACE
	for (const auto& ns : cpp.output_namespace) {
		write_line("namespace {} {{", ns);
	}
	write_linefeed();

	// Default Templates
	write_line_tabbed(1, "// Begin Default Templates");

	write_line_tabbed(1, "template<class T>");
	if (cfg.cpp_config.string_table_type == StringTableType::ConstWCharPtr)
	{
		write_line_tabbed(1, "constexpr ::enumbra::from_wstring_result<T> from_wstring(const wchar_t* str, std::size_t len) = delete;");
	}
	else
	{
		write_line_tabbed(1, "constexpr ::enumbra::from_string_result<T> from_string(const char* str, std::size_t len) = delete;");
	}

	const std::string default_templates = R"(
    template<class T>
    constexpr auto& values() = delete;

    template<class T>
    constexpr auto& flags() = delete;

    template<class T, class underlying_type = typename ::enumbra::detail::base_helper<T>::base_type>
    constexpr bool is_valid(underlying_type value) = delete;
    // End Default Templates

)";
	write(default_templates);

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

		size_t bits_required_storage = get_storage_bits_required(max_abs_representable);
		const size_t bits_required_transmission = get_transmission_bits_required(max_entry.p_value - min_entry.p_value);

		// Because of the way signed integers map to bit fields, a bit field may require an additional
		// bit of storage to accomodate the sign bit even if it is unused. For example, given the following enum:
		//   enum class ESignedValueBits : int8_t { A = 0, B = 1, C = 2, D = 3 }
		// To properly store and assign to this enum, we need 3 bits:
		//   int8_t Value : 1; // maps to the range -1 - 0, unexpected!
		//   int8_t Value : 2; // maps to the range -2 - 1, still not big enough
		//   int8_t Value : 3; // maps to the range -4 - 3, big enough, but we're wasting space
		// For this reason, when utilizing packed enums it is recommended to always prefer an unsigned underlying
		// type unless your enum actually contains negative values.
		if (is_size_type_signed && (max_entry.p_value > 0)) {
			uint64_t signed_range_max = 0;
			for (size_t i = 0; i < bits_required_storage - 1; i++) {
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
		write_line_tabbed(1, "// {} Definition", e.name);
		{
			write_line_tabbed(1, "enum class {0} : {1} {{", e.name, size_type);
			for (const auto& v : e.values) {
				write_line_tabbed(2, "{} = {},", v.name, Int128FormatValue{ v.p_value, type_bits, is_size_type_signed });
			}
			write_line_tabbed(1, "}};");
		}
		write_linefeed();

		// detail namespace
		write_line_tabbed(1, "namespace detail::{0} {{", e.name);

		// values_arr
		write_line_tabbed(2, "constexpr std::array<::{2}{0}, {1}> values_arr =", e.name, entry_count, full_ns);
		write_line_tabbed(2, "{{{{");
		for (const auto& v : e.values) {
			write_line_tabbed(3, "::{2}{0}::{1},", e.name, v.name, full_ns);
		}
		write_line_tabbed(2, "}}}};");

		if (e.values.size() > 1)
		{


			struct StringLookupTables
			{
				std::vector<size_t> sizes;
				std::vector<std::pair<size_t, size_t>> offset_and_count;
				std::vector<enum_entry> entries;
			};

			auto generate_string_lookup_tables = [&]() -> StringLookupTables
				{
					StringLookupTables output;

					// Put strings into buckets by size
					std::map<size_t, std::vector<enum_entry>> buckets_by_size;
					for (auto& ed : e.values)
					{
						buckets_by_size[ed.name.length()].push_back(ed);
					}

					size_t offset = 0;
					for (auto& bucket : buckets_by_size)
					{
						output.sizes.push_back(bucket.first);

						const size_t count = bucket.second.size();
						output.offset_and_count.emplace_back(offset, count);
						offset += count;

						for (auto& entry : bucket.second)
						{
							output.entries.push_back(entry);
						}
					}

					return output;
				};

			auto string_tables = generate_string_lookup_tables();

			// string_sizes
			write_line_tabbed(2, "constexpr std::array<uint32_t, {0}> string_sizes = {{{{", string_tables.sizes.size());
			for (auto& s : string_tables.sizes) {
				write_line_tabbed(3, "{0},", s);
			}
			write_line_tabbed(2, "}}}};");

			// offset_and_count
			write_line_tabbed(2, "constexpr std::array<::enumbra::detail::string_table_entry, {0}> string_table_meta = {{{{", string_tables.offset_and_count.size());
			for (auto& s : string_tables.offset_and_count) {
				write_line_tabbed(3, "{{{0}, {1}}},", s.first, s.second);
			}
			write_line_tabbed(2, "}}}};");

			// enum_strings
			write_line_tabbed(2, "constexpr std::array<{0}, {1}> enum_strings = {{{{", char_type, string_tables.entries.size());
			for (auto& s : string_tables.entries) {
				write_line_tabbed(3, "{0}\"{1}\",", string_literal_prefix, s.name);
			}
			write_line_tabbed(2, "}}}};");

			// enum_string_values
			write_line_tabbed(2, "constexpr std::array<::{2}{1}, {0}> enum_string_values = {{{{", entry_count, e.name, full_ns);
			for (auto& v : string_tables.entries)
			{
				write_line_tabbed(3, "::{2}{1}::{0},", v.name, e.name, full_ns);
			}
			write_line_tabbed(2, "}}}};");

		}


        // End detail namespace
		write_line_tabbed(1, "}}");
		write_linefeed();

		write_line_tabbed(1, "template<>");
		write_line_tabbed(1, "constexpr auto& values<{0}>()", e.name);
		write_line_tabbed(1, "{{");
		write_line_tabbed(2, "return detail::{0}::values_arr;", e.name);
		write_line_tabbed(1, "}}");
		write_linefeed();

		// is_valid variations
		if (e.values.size() == 1)
		{
			write_line_tabbed(1, "template<>", e.name);
			write_line_tabbed(1, "constexpr bool is_valid<{0}>({1} v) {{ return {2} == v; }}",
				e.name,
				size_type,
				Int128FormatValue{ max_entry.p_value, type_bits, is_size_type_signed }
			);
		}
		else if (is_contiguous)
		{
			if ((min_entry.p_value == 0) && !is_size_type_signed) // Unsigned values can't go below 0 so we just need to check that we're <= max
			{
				write_line_tabbed(1, "template<>", e.name);
				write_line_tabbed(1, "constexpr bool is_valid<{0}>({1} v) {{ return v <= {2}; }}",
					e.name,
					size_type,
					Int128FormatValue{ max_entry.p_value, type_bits, is_size_type_signed }
				);
			}
			else
			{
				write_line_tabbed(1, "template<>", e.name);
				write_line_tabbed(1, "constexpr bool is_valid<{0}>({1} v) {{ return ({2} <= v) && (v <= {3}); }}",
					e.name,
					size_type,
					Int128FormatValue{ min_entry.p_value, type_bits, is_size_type_signed },
					Int128FormatValue{ max_entry.p_value, type_bits, is_size_type_signed }
				);
			}
		}
		else
		{
			write_line_tabbed(1, "template<>", e.name);
			write_line_tabbed(1, "constexpr bool is_valid<{0}>({1} v) {{", e.name, size_type);
			write_line_tabbed(2, "for({0} val : values<{0}>()) {{", e.name, size_type);
			write_line_tabbed(3, "if(val == static_cast<{0}>(v)) {{ return true; }}", e.name, size_type);
			write_line_tabbed(2, "}}");
			write_line_tabbed(2, "return false;");
			write_line_tabbed(1, "}}");
		}
		write_linefeed();

		// String Functions
		write_line_tabbed(1, "constexpr {0} to_{1}string(const {2} v) {{", char_type, string_function_prefix, e.name);
		write_line_tabbed(2, "switch (v) {{");
		for (auto& v : e.values) {
			write_line_tabbed(3, "case {2}::{0}: return {1}\"{0}\";", v.name, string_literal_prefix, e.name);
		}
		write_line_tabbed(2, "}}");
		write_line_tabbed(2, "return {0}\"\";", string_literal_prefix);
		write_line_tabbed(1, "}}");
		write_linefeed();

		// from_string
		if (e.values.size() == 1)
		{
			const auto& v = e.values.at(0);
			write_line_tabbed(1, "template<>", e.name);
			write_line_tabbed(1, "constexpr ::enumbra::from_{0}string_result<{2}> from_{0}string<{2}>({1} str, std::size_t len) {{", string_function_prefix, char_type, e.name);
			write_line_tabbed(2, "if (enumbra::detail::streq_s({0}\"{1}\", {2}, str, len)) {{", string_literal_prefix, v.name, v.name.length());
			write_line_tabbed(3, "return {{true, {0}::{1}}};", e.name, v.name);
			write_line_tabbed(2, "}}");
			write_line_tabbed(2, "return {{false, {0}()}};", e.name);
			write_line_tabbed(1, "}}");
		}
		else
		{
			write_line_tabbed(1, "template<>");
			write_line_tabbed(1, "constexpr ::enumbra::from_{0}string_result<{2}> from_{0}string<{2}>({1} str, std::size_t len) {{", string_function_prefix, char_type, e.name);

			// Value String Table
			const std::string from_string_complex = R"(        constexpr ::enumbra::from_{1}string_result<{0}> empty_val = {{false, {0}()}};
        uint32_t offset = 0;
        uint32_t count = 0;
        for (std::size_t i = 0; i < detail::{0}::string_sizes.size(); ++i) {{
            const auto& current = detail::{0}::string_sizes[i];
            if (current > len) {{ return empty_val; }}
            if (current == len) {{ 
                offset = detail::{0}::string_table_meta[i].offset;
                count = detail::{0}::string_table_meta[i].count;
                break;
            }}
        }}
        if (count == 0) {{ return empty_val; }}

        for (std::size_t i = 0; i < count; ++i)
        {{
            const auto& e_str = detail::{0}::enum_strings[offset + i];
            if (enumbra::detail::streq_known_size(e_str, str, len)) {{
                return {{true, detail::{0}::enum_string_values[offset + i]}};
            }}
        }}

        return empty_val;
    }}
)";
			write(from_string_complex,
				e.name,
                string_function_prefix
			);
		}

		// Helper specializations
		template_specializations.emplace_back(
			fmt::format("template<> struct enumbra::detail::value_enum_helper<{0}{1}> : enumbra::detail::value_enum_info<{2}, {3}, {4}, {5}, int, {6}, {7}, {8}, {9}> {{{{ }}}};",
				full_ns,
				e.name,
				size_type,
				Int128FormatValue{ min_entry.p_value, type_bits, is_size_type_signed },
				Int128FormatValue{ max_entry.p_value, type_bits, is_size_type_signed },
				Int128FormatValue{ default_entry.p_value, type_bits, is_size_type_signed },
				unique_entry_count,
				is_contiguous ? "true" : "false",
				bits_required_storage,
				bits_required_transmission
			)
		);

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

		// TODO: Add default<>
		const uint64_t default_value = get_flags_enum_value(enum_meta.flags_enum_default_value_style, e);

		//const size_t entry_count = e.values.size();
		const size_t bits_required_storage = get_storage_bits_required(max_value);
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
		write_line_tabbed(1, "// {} Definition", e.name);
		{
			write_line_tabbed(1, "enum class {0} : {1} {{", e.name, size_type);
			for (const auto& v : e.values) {
				write_line_tabbed(2, "{} = {},", v.name, Int128FormatValue{ v.p_value, type_bits, is_size_type_signed });
			}
			write_line_tabbed(1, "}};");
		}
		write_linefeed();

		write_line_tabbed(1, "namespace detail {{ namespace {0} {{", e.name);
		write_line_tabbed(2, "constexpr std::array<::{2}{0}, {1}> flags_arr =", e.name, unique_entry_count, full_ns);
		write_line_tabbed(2, "{{{{");
		for (const auto& v : e.values) {
			write_line_tabbed(3, "::{2}{0}::{1},", e.name, v.name, full_ns);
		}
		write_line_tabbed(2, "}}}};");
		write_line_tabbed(1, "}} }}");
		write_linefeed();

		write_line_tabbed(1, "template<>");
		write_line_tabbed(1, "constexpr auto& flags<{0}>()", e.name);
		write_line_tabbed(1, "{{");
		write_line_tabbed(2, "return detail::{0}::flags_arr;", e.name);
		write_line_tabbed(1, "}}");
		write_linefeed();

		//// Functions
		write_line_tabbed(1, "constexpr void zero({0}& value) {{ value = static_cast<{0}>(0); }}", e.name);
		write_line_tabbed(1, "constexpr bool test({0} value, {0} flags) {{ return (static_cast<{1}>(flags) & static_cast<{1}>(value)) == static_cast<{1}>(flags); }}", e.name, size_type);
		write_line_tabbed(1, "constexpr void set({0}& value, {0} flags) {{ value = static_cast<{0}>(static_cast<{1}>(value) | static_cast<{1}>(flags)); }}", e.name, size_type);
		write_line_tabbed(1, "constexpr void unset({0}& value, {0} flags) {{ value = static_cast<{0}>(static_cast<{1}>(value) & (~static_cast<{1}>(flags))); }}", e.name, size_type);
		write_line_tabbed(1, "constexpr void toggle({0}& value, {0} flags) {{ value = static_cast<{0}>(static_cast<{1}>(value) ^ static_cast<{1}>(flags)); }}", e.name, size_type);
		write_line_tabbed(1, "constexpr bool is_all({0} value) {{ return static_cast<{1}>(value) >= {2:#x}; }}", e.name, size_type, max_value);
		write_line_tabbed(1, "constexpr bool is_any({0} value) {{ return static_cast<{1}>(value) > 0; }}", e.name, size_type);
		write_line_tabbed(1, "constexpr bool is_none({0} value) {{ return static_cast<{1}>(value) == 0; }}", e.name, size_type);
		write_line_tabbed(1, "constexpr bool is_single({0} value) {{ {1} n = static_cast<{1}>(value); return n && !(n & (n - 1)); }}", e.name, size_type);
		write_linefeed();

		//// is_valid variations
		//if (is_contiguous)
		//{
		//	if (min_value == 0 && !is_size_type_signed) // Unsigned values can't go below 0 so we just need to check that we're <= max
		//	{
		//		write_line_tabbed(1, "static constexpr bool _is_valid({0} v) {{ return static_cast<{1}>(v.value_) <= {2}; }}", e.name, size_type, max_value);
		//		write_line_tabbed(1, "static constexpr bool _is_valid({1} v) {{ return v <= {2}; }}", e.name, size_type, max_value);
		//	}
		//	else
		//	{
		//		write_line_tabbed(1, "static constexpr bool _is_valid({0} v) {{ return ({2} <= static_cast<{1}>(v.value_)) && (static_cast<{1}>(v.value_) <= {3}); }}", e.name, size_type, min_value, max_value);
		//		write_line_tabbed(1, "static constexpr bool _is_valid({1} v) {{ return ({2} <= v) && (v <= {3}); }}", e.name, size_type, min_value, max_value);
		//	}
		//}
		//else
		//{
		//	write_line_tabbed(1, "static constexpr bool _is_valid({0} v) {{ for(std::size_t i = 0; i < _values.size(); i++) {{ auto& val = _values[i]; if(val == v._value()) return true; }} return false; }}", e.name);
		//	write_line_tabbed(1, "static constexpr bool _is_valid({1} v) {{ for(std::size_t i = 0; i < _values.size(); i++) {{ auto& val = _values[i]; if(val == _enum(v)) return true; }} return false; }}", e.name, size_type);
		//}
		//write_linefeed();

		std::vector<const char*> operator_strings = {
			"// {} Operator Overloads",

			"constexpr {0} operator~(const {0} a) {{ return static_cast<{0}>(~static_cast<{1}>(a)); }}",
			"constexpr {0} operator|(const {0} a, const {0} b) {{ return static_cast<{0}>(static_cast<{1}>(a) | static_cast<{1}>(b)); }}",
			"constexpr {0} operator&(const {0} a, const {0} b) {{ return static_cast<{0}>(static_cast<{1}>(a) & static_cast<{1}>(b)); }}",
			"constexpr {0} operator^(const {0} a, const {0} b) {{ return static_cast<{0}>(static_cast<{1}>(a) ^ static_cast<{1}>(b)); }}",

			"constexpr {0}& operator|=({0}& a, const {0} b) {{ a = a | b; return a; }}",
			"constexpr {0}& operator&=({0}& a, const {0} b) {{ a = a & b; return a; }}",
			"constexpr {0}& operator^=({0}& a, const {0} b) {{ a = a ^ b; return a; }}",
		};
		for (auto& str : operator_strings) {
			write_line_tabbed(1, str, e.name, size_type);
		}

		// Helper specializations
		template_specializations.emplace_back(
			fmt::format("template<> struct enumbra::detail::flags_enum_helper<{0}{1}> : enumbra::detail::flags_enum_info<{2}, {3}, {4}, {5}, int, {6}, {7}, {8}, {9}> {{{{ }}}};",
				full_ns,
				e.name,
				size_type,
				Int128FormatValue{ min_value, type_bits, is_size_type_signed },
				Int128FormatValue{ max_value, type_bits, is_size_type_signed },
				Int128FormatValue{ default_value, type_bits, is_size_type_signed },
				unique_entry_count,
				is_contiguous ? "true" : "false",
				bits_required_storage,
				bits_required_transmission
			)
		);

		write_linefeed();
	}

	// END NAMESPACE
	for (auto ns = cpp.output_namespace.rbegin(); ns != cpp.output_namespace.rend(); ++ns) {
		write_line("}} // namespace {}", *ns);
	}
	write_linefeed();

	// MSVC C2888: Template specializations need to be outside of the user-defined namespace so we'll stick them after the definitions.
	{
		write_line("// Template Specializations Begin");

		// Value Enum Template Specializations
		for (auto& e : enum_meta.value_enum_definitions) {
			std::vector<const char*> template_strings = {
				"template<> struct enumbra::detail::base_helper<{3}{0}> : enumbra::detail::type_info<true, {1}, {2}> {{ }};",
			};
			for (auto& str : template_strings) {
				write_line(str, e.name, "true", "false", full_ns);
			}
		}

		// Flags Enum Template Specializations
		for (auto& e : enum_meta.flag_enum_definitions) {
			std::vector<const char*> template_strings = {
				"template<> struct enumbra::detail::base_helper<{3}{0}> : enumbra::detail::type_info<true, {1}, {2}> {{ }};",
			};
			for (auto& str : template_strings) {
				write_line(str, e.name, "false", "true", full_ns);
			}
		}

		// Write out the buffered template specializations
		for (auto& s : template_specializations)
		{
			write_line(s);
		}

		write_line("// Template Specializations End");
	}

	// END INCLUDE GUARD
	if (cpp.include_guard_style == IncludeGuardStyle::CStyle) {
		write_line("#endif // {}", def_macro);
	}

	return Output;
}
