#pragma once

#include <string>
#include <vector>
#include <array>

namespace enumbra {
	namespace csharp {
		// ...
	}

	namespace cpp {
		enum class NamingConvention {
			None,				// No enforced style
			SnakeCase,			// enforced_style
			ScreamingSnakeCase, // ENFORCED_STYLE
			CamelCase,			// enforcedStyle
			PascalCase			// EnforcedStyle
		};
		constexpr std::array<std::pair<std::string_view, NamingConvention>, 5> NamingConventionMapped 
		{ {
			{ "none", NamingConvention::None },
			{ "snake_case", NamingConvention::SnakeCase },
			{ "SCREAMING_SNAKE_CASE", NamingConvention::ScreamingSnakeCase },
			{ "camelCase", NamingConvention::CamelCase },
			{ "PascalCase", NamingConvention::PascalCase },
		} };

		enum class IncludeGuardStyle {
			None,
			PragmaOnce,
			CStyle
		};
		constexpr std::array<std::pair<std::string_view, IncludeGuardStyle>, 3> IncludeGuardStyleMapped 
		{ {
			{ "none", IncludeGuardStyle::None },
			{ "pragma_once", IncludeGuardStyle::PragmaOnce },
			{ "c_style", IncludeGuardStyle::CStyle },
		} };

		enum class StringTableType {
			ConstexprStringView,
			ConstexprWStringView,
			ConstCharPtr,
			ConstWCharPtr
		};
		constexpr std::array<std::pair<std::string_view, StringTableType>, 4> StringTableTypeMapped
		{ {
			{ "constexpr_string_view", StringTableType::ConstexprStringView },
			{ "constexpr_wstring_view", StringTableType::ConstexprWStringView },
			{ "const_char_ptr", StringTableType::ConstCharPtr },
			{ "const_wchar_ptr", StringTableType::ConstWCharPtr },
		} };

		enum class StringTableLayout {
			None,
			NameOnly,
			NameAndDescription
		};
		constexpr std::array<std::pair<std::string_view, StringTableLayout>, 3> StringTableLayoutMapped 
		{ {
			{ "none", StringTableLayout::None },
			{ "name", StringTableLayout::NameOnly },
			{ "name_and_description", StringTableLayout::NameAndDescription },
		} };

		enum class LineEndingStyle {
			LF,
			CRLF
		};
		constexpr std::array<std::pair<std::string_view, LineEndingStyle>, 2> LineEndingStyleMapped
		{ {
			{ "LF", LineEndingStyle::LF },
			{ "CRLF", LineEndingStyle::CRLF },
		} };

		struct enum_size_type {
			std::string name;
			int64_t bits{ 0 };
			bool is_signed{ true };
			std::string generated_name;
		};

		struct cpp_config
		{
			std::string output_extension{};
			std::vector<std::string> output_namespace;
			std::vector<std::string> preamble_text{};
			std::vector<std::string> additional_includes;
			IncludeGuardStyle include_guard_style{ IncludeGuardStyle::PragmaOnce };
			LineEndingStyle line_ending_style{ LineEndingStyle::LF };
			bool use_cstdint{ true };

			std::string value_enum_name_prefix;
			std::string value_enum_name_postfix;
			std::string value_enum_value_prefix;
			std::string value_enum_value_postfix;
			std::string flags_enum_name_prefix;
			std::string flags_enum_name_postfix;
			std::string flags_enum_value_prefix;
			std::string flags_enum_value_postfix;

			std::vector<enum_size_type> size_types;
			size_t default_value_enum_size_type_index{ };
			size_t default_flags_enum_size_type_index{ };

			std::vector<size_t> flags_enum_smallest_unsigned_evaluation_order{};
			std::vector<size_t> value_enum_smallest_unsigned_evaluation_order{};
			std::vector<size_t> value_enum_smallest_signed_evaluation_order{};

			StringTableType string_table_type{ StringTableType::ConstCharPtr };
			StringTableLayout string_table_layout{ StringTableLayout::NameAndDescription };

			bool bitwise_op_functions{ true };
			bool default_functions{ true };
			bool bounds_check_functions{ true };
			bool density_functions{ true };
			bool min_max_functions{ true };
			bool bit_info_functions{ true };
			bool flag_helper_functions{ true };
			bool packed_declaration_macros{ true };

			bool warnings_as_errors{ true };
			bool value_enum_name_naming_convention_violation{ true };
			bool value_enum_value_naming_convention_violation{ true };
			bool flags_enum_name_naming_convention_violation{ true };
			bool flags_enum_value_naming_convention_violation{ true };

			int64_t get_size_type_index(std::string_view name);
		};
	}

	enum class Verbosity {
		Low,
		High
	};

	enum class ValueEnumDefaultValueStyle {
		Min,
		Max,
		First,
		Last
	};
	constexpr std::array<std::pair<std::string_view, ValueEnumDefaultValueStyle>, 4> ValueEnumDefaultValueStyleMapped
	{ {
		{ "min", ValueEnumDefaultValueStyle::Min },
		{ "max", ValueEnumDefaultValueStyle::Max },
		{ "first", ValueEnumDefaultValueStyle::First },
		{ "last", ValueEnumDefaultValueStyle::Last },
	} };

	enum class FlagsEnumDefaultValueStyle {
		Zero,
		UsedBitsSet,
		Min,
		Max,
		First,
		Last
	};
	constexpr std::array<std::pair<std::string_view, FlagsEnumDefaultValueStyle>, 6> FlagsEnumDefaultValueStyleMapped
	{ {
		{ "zero", FlagsEnumDefaultValueStyle::Zero },
		{ "used_bits_set", FlagsEnumDefaultValueStyle::UsedBitsSet },
		{ "min", FlagsEnumDefaultValueStyle::Min },
		{ "max", FlagsEnumDefaultValueStyle::Max },
		{ "first", FlagsEnumDefaultValueStyle::First },
		{ "last", FlagsEnumDefaultValueStyle::Last },
	} };

	struct enum_entry
	{
		std::string name;
		int64_t value;
	};

	struct enum_definition
	{
		std::string name;
		std::vector<enum_entry> values;
		int64_t default_value{ 0 };

		size_t size_type_index;
	};

	struct enum_meta_config
	{
		ValueEnumDefaultValueStyle value_enum_default_value_style{ ValueEnumDefaultValueStyle::Min };
		FlagsEnumDefaultValueStyle flags_enum_default_value_style{ FlagsEnumDefaultValueStyle::Zero };
		int64_t value_enum_start_value{ 0 };
		uint64_t flags_enum_start_value{ 1 };

		bool value_enum_require_sequential{ true };
		bool flags_enum_require_packed_bits{ true };
		bool value_enum_require_unique_values{ true };
		bool flags_enum_allow_overlap{ false };
		bool flags_enum_allow_multi_bit_values{ false };
	};

	struct enumbra_config
	{
		bool generate_cpp{ true };
		bool generate_csharp{ false };
		cpp::cpp_config cpp_config{};
		Verbosity verbosity{ Verbosity::Low };
	};

	enumbra_config load_config(std::string_view config_toml_file, enumbra::Verbosity verbosity);
}
