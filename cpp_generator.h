#pragma once

#include "enumbra.h"
#include <array>
#include <string>
#include <string_view>

//#include <string_view>
//using namespace std::string_view_literals;
//constexpr std::wstring_view x = L""sv;

using namespace std::string_view_literals;
using namespace std::string_literals;

struct cpp_enum_intermediate
{
	std::string name;
	std::string description;
	int64_t value;
};

struct cpp_enum_generated
{
	std::string name;
	std::vector<cpp_enum_intermediate> values;
};

struct cpp_enum_config_final
{

};

class cpp_generator {
public:
	const std::string& generate_cpp_output(const enumbra::enumbra_config& cfg, const enumbra::enum_meta_config& enum_meta);

private:
	std::string LF; // Line Feed as configured in enumbra_config
	std::string TAB; // Tabs as configured in enumbra_config
	std::string Output;

	std::vector<cpp_enum_generated> generate_enums(const cpp_enum_config_final& cfg);

	template <typename... Args>
	void write(std::string_view fmt, Args&&... args) {
		fmt::format_to(std::back_inserter(Output), fmt, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void write_linefeed(int count = 1) {
		for (int i = 0; i < count; i++)
			write("{}", LF);
	}

	template <typename... Args>
	void write_tab(int count = 1) {
		for (int i = 0; i < count; i++)
			write("{}", TAB);
	}

	template <typename... Args>
	void write_line(std::string_view fmt, Args&&... args) {
		write(fmt, args...);
		write_linefeed();
	}

	template <typename... Args>
	void write_line_tabbed(int tab_count, std::string_view fmt, Args&&... args) {
		for (int i = 0; i < tab_count; i++)
			write("{}", TAB);
		write(fmt, args...);
		write_linefeed();
	}
};

// https://en.cppreference.com/w/cpp/keyword

constexpr std::array<std::string_view, 117> ReservedCPPKeywords{
	"alignas",
	"alignof",
	"and",
	"and_eq",
	"asm",
	"atomic_cancel",
	"atomic_commit",
	"atomic_noexcept",
	"auto",
	"bitand",
	"bitor",
	"bool",
	"break",
	"case",
	"catch",
	"char",
	"char8_t",
	"char16_t",
	"char32_t",
	"class",
	"compl",
	"concept",
	"const",
	"consteval",
	"constexpr",
	"constinit",
	"const_cast",
	"continue",
	"co_await",
	"co_return",
	"co_yield",
	"decltype",
	"default",
	"delete",
	"do",
	"double",
	"dynamic_cast",
	"else",
	"enum",
	"explicit",
	"export",
	"extern",
	"false",
	"float",
	"for",
	"friend",
	"goto",
	"if",
	"inline",
	"int",
	"long",
	"mutable",
	"namespace",
	"new",
	"noexcept",
	"not",
	"not_eq",
	"nullptr",
	"operator",
	"or",
	"or_eq",
	"private",
	"protected",
	"public",
	"reflexpr",
	"register",
	"reinterpret_cast",
	"requires",
	"return",
	"short",
	"signed",
	"sizeof",
	"static",
	"static_assert",
	"static_cast",
	"struct",
	"switch",
	"synchronized",
	"template",
	"this",
	"thread_local",
	"throw",
	"true",
	"try",
	"typedef",
	"typeid",
	"typename",
	"union",
	"unsigned",
	"using",
	"virtual",
	"void",
	"volatile",
	"wchar_t",
	"while",
	"xor",
	"xor_eq",

	"final",
	"override",
	"transaction_safe",
	"transaction_safe_dynamic",
	"export",
	"import",
	"module",

	"if",
	"elif",
	"else",
	"endif",
	"ifdef",
	"ifndef",
	"define",
	"undef",
	"include",
	"line",
	"error",
	"pragma",
	"defined"
};

// https://en.cppreference.com/w/cpp/language/identifiers
// Other rules

