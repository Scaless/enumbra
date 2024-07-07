#pragma once

#include <string>
#include <vector>
#include <array>
#include <nlohmann/json.hpp>
#include <absl/numeric/int128.h>

using json = nlohmann::json;
using int128 = absl::int128;
using uint128 = absl::uint128;

namespace enumbra {
	constexpr char kEnumbraVersion[] = "0.2.0";

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
			{ "name_only", StringTableLayout::NameOnly },
			{ "name_and_description", StringTableLayout::NameAndDescription },
		} };

		struct enum_size_type {
			std::string name;
			uint64_t bits{ 0 };
			bool is_signed{ true };
			std::string type_name;
			int128 min_possible_value{ absl::Int128Max() };
			int128 max_possible_value{ absl::Int128Max() };
		};

		struct cpp_config
		{
			std::vector<std::string> output_namespace;
			std::vector<std::string> preamble_text{};
			std::vector<std::string> additional_includes;
			IncludeGuardStyle include_guard_style{ IncludeGuardStyle::PragmaOnce };
			bool time_generated_in_header{ true };

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
			bool enumbra_bitfield_macros{ true };

			size_t get_size_type_index_from_name(std::string_view name);
			[[nodiscard]] const enum_size_type& get_size_type_from_index(size_t index) const;
		};
	}

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
		int128 p_value;
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

		std::vector<enum_definition> value_enum_definitions;
		std::vector<enum_definition> flag_enum_definitions;
	};

	struct enumbra_config
	{
		bool generate_cpp{ true };
		bool generate_csharp{ false };
		cpp::cpp_config cpp_config{};
	};

	// Get an array of T from the json structure, possibly empty
	template<typename T>
	std::vector<T> get_array(json& config)
	{
		std::vector<T> out;

		if (config.is_null())
		{
			return out;
		}

		for (auto iter : config)
		{
			out.push_back(iter.get<T>());
		}

		return out;
	}

	// Get a mapped value from the json structure, throwing if key is not mapped
	// Requires a mapping table of the following form: std::array<std::pair<std::string_view, T>, #>
	template<typename T, typename mapping>
	T get_mapped(mapping& map, json& config) {
		if (map.size() == 0) {
			throw std::logic_error("Map passed to get_mapped has a size of 0");
		}

		auto param = config.get<std::string>();

		using namespace enumbra::cpp;
		auto found_map = std::find_if(map.begin(), map.end(),
			[&param](const std::pair<std::string_view, T>& entry) {
				return entry.first == param;
			});
		if (found_map != map.end()) {
			return found_map->second;
		}

		std::string exception = std::string(param) + " value must be one of: ";
		for (size_t x = 0; x < map.size() - 1; x++) {
			exception += std::string(map[x].first) + ", ";
		}
		exception += map[map.size() - 1].first;
		throw std::logic_error(exception.c_str());
	}

}
