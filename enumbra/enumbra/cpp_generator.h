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

// https://en.cppreference.com/w/cpp/keyword

constexpr std::array<std::string_view, 117> ReservedCPPKeywords {
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


class cpp_generator {

public:
	std::string generate_cpp_output(const enumbra::enumbra_config& cfg, const enumbra::enum_definition& enum_def);

private:
	std::string LF; // Line Feed


};

