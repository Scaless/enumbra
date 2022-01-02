#pragma once

#include <string>
#include <vector>
#include <array>

namespace enumbra {
	// Remember to update vcpkg.json as well
	constexpr char* kEnumbraVersion = "0.0.5";

	namespace csharp {
		// ...
	}

	namespace cpp {
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
			ConstCharPtr,
			ConstWCharPtr
		};
		constexpr std::array<std::pair<std::string_view, StringTableType>, 4> StringTableTypeMapped
		{ {
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
			{ "block_name", StringTableLayout::NameOnly },
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
			std::vector<std::string> output_namespace;
			std::vector<std::string> preamble_text{};
			std::vector<std::string> additional_includes;
			IncludeGuardStyle include_guard_style{ IncludeGuardStyle::PragmaOnce };
			LineEndingStyle line_ending_style{ LineEndingStyle::LF };
			std::string output_tab_characters{};

			std::string value_enum_name_prefix;
			std::string value_enum_name_postfix;
			std::string value_enum_value_prefix;
			std::string value_enum_value_postfix;
			std::string flags_enum_name_prefix;
			std::string flags_enum_name_postfix;
			std::string flags_enum_value_prefix;
			std::string flags_enum_value_postfix;

			std::vector<enum_size_type> size_types;
			size_t default_value_enum_size_type_index{ SIZE_MAX };
			size_t default_flags_enum_size_type_index{ SIZE_MAX };

			std::vector<size_t> flags_enum_smallest_unsigned_evaluation_order{};
			std::vector<size_t> value_enum_smallest_unsigned_evaluation_order{};
			std::vector<size_t> value_enum_smallest_signed_evaluation_order{};

			StringTableType string_table_type{ StringTableType::ConstCharPtr };
			StringTableLayout string_table_layout{ StringTableLayout::NameAndDescription };

			bool min_max_functions{ true };
			bool bit_info_functions{ true };
			bool enumbra_macros{ true };
			bool templated_extensions{ true };

			size_t get_size_type_index_from_name(std::string_view name);
			const enum_size_type& get_size_type_from_index(size_t index) const;
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
		std::string description;
		int64_t value;
	};

	struct enum_definition
	{
		std::string name;
		std::vector<enum_entry> values;
		std::string default_value_name;

		size_t size_type_index{ SIZE_MAX };
	};

	struct enum_meta_config
	{
		std::string block_name;
		ValueEnumDefaultValueStyle value_enum_default_value_style{ ValueEnumDefaultValueStyle::Min };
		FlagsEnumDefaultValueStyle flags_enum_default_value_style{ FlagsEnumDefaultValueStyle::Zero };
		int64_t value_enum_start_value{ 0 };
		uint64_t flags_enum_start_value{ 1 };

		bool value_enum_require_sequential{ true };
		bool flags_enum_require_packed_bits{ true };
		bool value_enum_require_unique_values{ true };
		bool flags_enum_allow_overlap{ false };
		bool flags_enum_allow_multi_bit_values{ false };

		std::vector<enum_definition> value_enum_definitions;
		std::vector<enum_definition> flag_enum_definitions;
	};

	struct value_enum_override_config
	{
		ValueEnumDefaultValueStyle value_enum_default_value_style{ ValueEnumDefaultValueStyle::Min };
		int64_t value_enum_start_value{ 0 };
		bool value_enum_require_sequential{ true };
		bool value_enum_require_unique_values{ true };
	};

	struct flags_enum_override_config
	{
		FlagsEnumDefaultValueStyle flags_enum_default_value_style{ FlagsEnumDefaultValueStyle::Zero };
		uint64_t flags_enum_start_value{ 1 };
		bool flags_enum_require_packed_bits{ true };
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

	// Get a required value from the toml structure, throwing if not found
	template<typename T, typename node>
	auto get_required(node& config, std::string_view param) {
		auto opt = config[param].value<T>();
		if (opt.has_value()) {
			return opt.value();
		}

		std::string x = "Configuration value is required: " + std::string(param);
		throw std::logic_error(x.c_str());
	}

	// Get an array of T from the toml structure, possibly empty
	template<typename T, typename node>
	std::vector<T> get_array(node& config, std::string_view param) {
		std::vector<T> out;
		if (toml::array* preamble = config[param].as_array()) {
			for (auto& elem : *preamble) {
				auto opt_val = elem.value<T>();
				if (opt_val.has_value()) {
					out.push_back(opt_val.value());
				}
				else {
					throw std::logic_error("");
				}
			}
		}
		return out;
	}

	// Get a mapped value from the toml structure using a string key, throwing if key is not mapped
	// Requires a mapping table of the following form: std::array<std::pair<std::string_view, T>, #>
	template<typename T, typename mapping, typename node>
	T get_mapped(mapping& map, node& config, std::string_view field_name) {
		if (map.size() == 0) {
			throw std::logic_error("Map passed to get_mapped has a size of 0");
		}

		auto param = get_required<std::string>(config, field_name);

		using namespace enumbra::cpp;
		auto found_map = std::find_if(map.begin(), map.end(),
			[&param](const std::pair<std::string_view, T>& entry) {
				return entry.first == param;
			});
		if (found_map != map.end()) {
			return found_map->second;
		}

		std::string exception = std::string(field_name) + " value must be one of: ";
		for (int x = 0; x < map.size() - 1; x++) {
			exception += std::string(map[x].first) + ", ";
		}
		exception += map[map.size() - 1].first;
		throw std::logic_error(exception.c_str());
	}

}
