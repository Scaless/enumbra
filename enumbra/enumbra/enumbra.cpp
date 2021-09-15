// enumbra.cpp : Defines the entry point for the application.
//

#include "enumbra.h"
#include "cpp_generator.h"
#include <iostream>
#include <filesystem>
#include <cxxopts.hpp>
#include <toml++/toml.h>

std::string version = "0.0.2";

using namespace enumbra;

enumbra::enumbra_config load_enumbra_config(std::string_view config_toml_file, enumbra::Verbosity verbosity);
enumbra::enum_meta_config load_meta_config(enumbra::enumbra_config& enumbra_config, std::string_view config_toml_file, enumbra::Verbosity verbosity);
void parse_enumbra_cpp(enumbra::enumbra_config& enumbra_config, toml::node_view<toml::node>& cpp_config);
void parse_enumbra_csharp(enumbra::enumbra_config& enumbra_config, toml::node_view<toml::node>& cpp_config);
void parse_enum_meta(enumbra::enumbra_config& enumbra_config, enumbra::enum_meta_config& enum_config, toml::node_view<toml::node>& meta_config);
void parse_enum_meta_cpp(enumbra::enumbra_config& enumbra_config, enumbra::enum_meta_config& enum_config, toml::node_view<toml::node>& meta_cpp_config);
void parse_enum_meta_csharp(enumbra::enumbra_config& enumbra_config, enumbra::enum_meta_config& enum_config, toml::node_view<toml::node>& meta_csharp_config);

void print_help(const cxxopts::Options& options)
{
	std::cout << options.help() << std::endl;
}

void print_version()
{
	std::cout << "enumbra v" << version << std::endl;
}

int main(int argc, char** argv)
{
	try {
		cxxopts::Options options("enumbra", "An enum code generator.");
		options
			.custom_help("-c enumbra_config.toml -s enum.toml")
			.add_options()
			("h,help", "You are here.")
			("c,config", "[Required] Path to enumbra config file (enumbra.toml)", cxxopts::value<std::string>())
			("s,source", "[Required] Path to enum config file (enum.toml)", cxxopts::value<std::string>())
			("o,output", "[Optional] Path to output folder (default: source dir)", cxxopts::value<std::string>())
			("v,verbose", "Print additional information during processing.")
			("version", "Prints version information.")
			("p,print", "Prints output to the console.")
			;

		auto result = options.parse(argc, argv);

		if (result.count("h") || result.arguments().size() == 0) {
			print_help(options);
			return 0;
		}
		if (result.count("version")) {
			print_version();
			return 0;
		}
		if (!result.count("c")) {
			throw std::logic_error("Config (-c|--config) argument is required.");
		}
		if (!result.count("s")) {
			throw std::logic_error("Source (-s|--source) argument is required.");
		}

		auto config_file_path = result["c"].as<std::string>();
		auto source_file_path = result["s"].as<std::string>();

		if (!std::filesystem::exists(config_file_path)) {
			throw std::logic_error("Config file does not exist.");
		}
		if (!std::filesystem::exists(source_file_path)) {
			throw std::logic_error("Source file does not exist.");
		}

		auto verbosity = (result.count("v") ? enumbra::Verbosity::High : enumbra::Verbosity::Low);
		auto loaded_enumbra_config = load_enumbra_config(config_file_path, verbosity);

		//TODO: Load the real enum config

		auto enum_config = load_meta_config(loaded_enumbra_config, source_file_path, verbosity);

		enumbra::enum_meta_config enum_meta;
		enum_meta.block_name = "EnumMeta";

		enumbra::enum_definition def;
		def.name = "test";
		def.size_type_index = 0;
		def.values = {
			{"A", "Wow", 1},
			{"B", "Neat", 2}
		};

		enumbra::enum_definition def2;
		def2.name = "test2";
		def2.size_type_index = 0;
		def2.values = {
			{"A", "Wow", 1},
			{"B", "Neat", 2}
		};

		enumbra::enum_definition def3;
		def3.name = "test3";
		def3.size_type_index = 0;
		def3.values = {
			{"A", "Wow", 1},
			{"B", "Neat", 2}
		};

		enum_meta.enum_definitions.push_back(def);
		enum_meta.enum_definitions.push_back(def2);
		enum_meta.enum_definitions.push_back(def3);

		cpp_generator cpp_gen;
		std::string cpp_out = cpp_gen.generate_cpp_output(loaded_enumbra_config, enum_meta);

		if (result.count("p")) {
			std::cout << cpp_out << std::endl;
		}
	}
	catch (const std::exception& e) {
		std::cout << e.what();
		return -1;
	}

	return 0;
}


enumbra::enumbra_config load_enumbra_config(std::string_view config_toml_file, enumbra::Verbosity verbosity)
{
	enumbra::enumbra_config cfg;

	toml::table tbl = toml::parse_file(config_toml_file);

	auto& configuration = tbl["configuration"];
	auto& cpp_config = configuration["cpp_generator"];
	auto& csharp_config = configuration["csharp_generator"];

	bool generate_cpp = get_required<bool>(configuration, "generate_cpp");
	bool generate_csharp = get_required<bool>(configuration, "generate_csharp");

	if (cfg.generate_cpp) {
		parse_enumbra_cpp(cfg, cpp_config);
	}
	if (cfg.generate_csharp) {
		parse_enumbra_csharp(cfg, csharp_config);
	}

	return cfg;
}

enumbra::enum_meta_config load_meta_config(enumbra::enumbra_config& enumbra_config, std::string_view config_toml_file, enumbra::Verbosity verbosity)
{
	enumbra::enum_meta_config cfg;

	toml::table tbl = toml::parse_file(config_toml_file);

	auto& enum_meta = tbl["enum_meta"];
	auto& cpp_meta = enum_meta["cpp"];
	auto& csharp_meta = enum_meta["csharp"];

	parse_enum_meta(enumbra_config, cfg, enum_meta);

	if (enumbra_config.generate_cpp) {
		parse_enum_meta_cpp(enumbra_config, cfg, cpp_meta);
	}
	if (enumbra_config.generate_csharp) {
		parse_enum_meta_csharp(enumbra_config, cfg, csharp_meta);
	}


	return cfg;
}

void parse_enumbra_cpp(enumbra::enumbra_config& enumbra_config, toml::node_view<toml::node>& cpp_cfg)
{
	using namespace enumbra::cpp;
	try {
		enumbra::cpp::cpp_config& c = enumbra_config.cpp_config;
		c.output_namespace = get_array<std::string>(cpp_cfg, "output_namespace");
		c.output_extension = get_required<std::string>(cpp_cfg, "output_extension");
		c.line_ending_style = get_mapped<LineEndingStyle>(LineEndingStyleMapped, cpp_cfg, "output_line_ending_style");
		c.output_tab_characters = get_required<std::string>(cpp_cfg, "output_tab_characters");
		c.preamble_text = get_array<std::string>(cpp_cfg, "preamble_text");
		c.include_guard_style = get_mapped<IncludeGuardStyle>(IncludeGuardStyleMapped, cpp_cfg, "include_guard");
		c.use_cstdint = get_required<bool>(cpp_cfg, "use_cstdint");
		c.additional_includes = get_array<std::string>(cpp_cfg, "additional_includes");

		c.value_enum_name_prefix = get_required<std::string>(cpp_cfg, "value_enum_name_prefix");
		c.value_enum_value_prefix = get_required<std::string>(cpp_cfg, "value_enum_value_prefix");
		c.flags_enum_name_prefix = get_required<std::string>(cpp_cfg, "flags_enum_name_prefix");
		c.flags_enum_value_prefix = get_required<std::string>(cpp_cfg, "flags_enum_value_prefix");

		c.value_enum_name_postfix = get_required<std::string>(cpp_cfg, "value_enum_name_postfix");
		c.value_enum_value_postfix = get_required<std::string>(cpp_cfg, "value_enum_value_postfix");
		c.flags_enum_name_postfix = get_required<std::string>(cpp_cfg, "flags_enum_name_postfix");
		c.flags_enum_value_postfix = get_required<std::string>(cpp_cfg, "flags_enum_value_postfix");

		// TODO: Refactor into a function to get an array of struct
		if (toml::array* size_types = cpp_cfg["size_types"].as_array())
		{
			for (auto& elem : *size_types)
			{
				elem.visit([&c](auto&& el)
					{
						if constexpr (toml::is_table<decltype(el)>)
						{
							enum_size_type t;
							t.name = get_required<std::string>(el, "name");
							t.bits = get_required<int64_t>(el, "bits");
							t.is_signed = get_required<bool>(el, "is_signed");
							t.generated_name = get_required<std::string>(el, "generated_name");
							c.size_types.push_back(t);
						}
						else {
							throw std::logic_error("Invalid layout for size_type entry.");
						}
					});
			}
		}
		if (c.size_types.size() == 0) {
			throw std::logic_error("size_types array is required. See the enumbra documentation for details.");
		}

		{
			std::string default_value_enum_size_type = get_required<std::string>(cpp_cfg, "default_value_enum_size_type");
			c.default_value_enum_size_type_index = c.get_size_type_index_from_name(default_value_enum_size_type);
			if (c.default_value_enum_size_type_index == SIZE_MAX) {
				throw std::logic_error("default_value_enum_size_type must reference an existing size_type.");
			}
		}

		{
			std::string default_flags_enum_size_type = get_required<std::string>(cpp_cfg, "default_flags_enum_size_type");
			c.default_flags_enum_size_type_index = c.get_size_type_index_from_name(default_flags_enum_size_type);
			if (c.default_flags_enum_size_type_index == SIZE_MAX) {
				throw std::logic_error("default_flags_enum_size_type must reference an existing size_type.");
			}
		}

		{
			std::vector<std::string> flags_enum_smallest_unsigned_evaluation_order = get_array<std::string>(cpp_cfg, "flags_enum_smallest_unsigned_evaluation_order");
			for (auto& str : flags_enum_smallest_unsigned_evaluation_order) {
				auto index = c.get_size_type_index_from_name(str);
				if (index == SIZE_MAX) {
					throw std::logic_error("flags_enum_smallest_unsigned_evaluation_order must reference an existing size_type.");
				}
				c.flags_enum_smallest_unsigned_evaluation_order.push_back(index);
			}
		}

		{
			std::vector<std::string> value_enum_smallest_unsigned_evaluation_order = get_array<std::string>(cpp_cfg, "value_enum_smallest_unsigned_evaluation_order");
			for (auto& str : value_enum_smallest_unsigned_evaluation_order) {
				auto index = c.get_size_type_index_from_name(str);
				if (index == SIZE_MAX) {
					throw std::logic_error("value_enum_smallest_unsigned_evaluation_order must reference an existing size_type.");
				}
				c.value_enum_smallest_unsigned_evaluation_order.push_back(index);
			}
		}

		{
			std::vector<std::string> value_enum_smallest_signed_evaluation_order = get_array<std::string>(cpp_cfg, "value_enum_smallest_signed_evaluation_order");
			for (auto& str : value_enum_smallest_signed_evaluation_order) {
				auto index = c.get_size_type_index_from_name(str);
				if (index == SIZE_MAX) {
					throw std::logic_error("value_enum_smallest_signed_evaluation_order must reference an existing size_type.");
				}
				c.value_enum_smallest_signed_evaluation_order.push_back(index);
			}
		}

		{
			c.string_table_layout = get_mapped<StringTableLayout>(StringTableLayoutMapped, cpp_cfg, "string_table_layout");
			if (c.string_table_layout != StringTableLayout::None) {
				c.string_table_type = get_mapped<StringTableType>(StringTableTypeMapped, cpp_cfg, "string_table_type");
			}
		}

		c.bitwise_op_functions = get_required<bool>(cpp_cfg, "bitwise_op_functions");
		c.default_functions = get_required<bool>(cpp_cfg, "default_functions");
		c.bounds_check_functions = get_required<bool>(cpp_cfg, "bounds_check_functions");
		c.density_functions = get_required<bool>(cpp_cfg, "density_functions");
		c.min_max_functions = get_required<bool>(cpp_cfg, "min_max_functions");
		c.bit_info_functions = get_required<bool>(cpp_cfg, "bit_info_functions");
		c.flag_helper_functions = get_required<bool>(cpp_cfg, "flag_helper_functions");
		c.packed_declaration_macros = get_required<bool>(cpp_cfg, "packed_declaration_macros");
	}
	catch (const std::exception& e) {

		std::string x = std::string("parse_enumbra_cpp_config: ") + e.what();
		throw std::logic_error(x.c_str());
	}
}

void parse_enumbra_csharp(enumbra::enumbra_config& c, toml::node_view<toml::node>& cpp_config)
{
	throw std::logic_error("parse_enumbra_csharp not implemented. Set generate_csharp to false.");
}

void parse_enum_meta(enumbra::enumbra_config& enumbra_config, enumbra::enum_meta_config& enum_config, toml::node_view<toml::node>& meta_config) {
	enum_config.block_name = get_required<std::string>(meta_config, "block_name");
	enum_config.value_enum_default_value_style = get_mapped<ValueEnumDefaultValueStyle>(ValueEnumDefaultValueStyleMapped, meta_config, "value_enum_default_value_style");
	enum_config.flags_enum_default_value_style = get_mapped<FlagsEnumDefaultValueStyle>(FlagsEnumDefaultValueStyleMapped, meta_config, "flags_enum_default_value_style");
	enum_config.value_enum_start_value = get_required<int64_t>(meta_config, "value_enum_start_value");
	enum_config.flags_enum_start_value = get_required<uint64_t>(meta_config, "flags_enum_start_value");
	enum_config.value_enum_require_sequential = get_required<bool>(meta_config, "value_enum_require_sequential");
	enum_config.flags_enum_require_packed_bits = get_required<bool>(meta_config, "flags_enum_require_packed_bits");
	enum_config.value_enum_require_unique_values = get_required<bool>(meta_config, "value_enum_require_unique_values");
	enum_config.flags_enum_allow_overlap = get_required<bool>(meta_config, "flags_enum_allow_overlap");
	enum_config.flags_enum_allow_multi_bit_values = get_required<bool>(meta_config, "flags_enum_allow_multi_bit_values");
}
void parse_enum_meta_cpp(enumbra::enumbra_config& enumbra_config, enumbra::enum_meta_config& enum_config, toml::node_view<toml::node>& meta_cpp_config) {

}
void parse_enum_meta_csharp(enumbra::enumbra_config& enumbra_config, enumbra::enum_meta_config& enum_config, toml::node_view<toml::node>& meta_csharp_config) {
	throw std::logic_error("parse_enum_meta_csharp not implemented. Set generate_csharp to false.");
}

size_t enumbra::cpp::cpp_config::get_size_type_index_from_name(std::string_view name)
{
	for (size_t i = 0; i < size_types.size(); i++) {
		if (size_types[i].name == name) {
			return i;
		}
	}
	return SIZE_MAX;
}

const enumbra::cpp::enum_size_type& enumbra::cpp::cpp_config::get_size_type_from_index(size_t index) const
{
	// Will throw if index out of bounds
	return size_types.at(index);
}
