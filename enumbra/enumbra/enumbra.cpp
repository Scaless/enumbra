// enumbra.cpp : Defines the entry point for the application.
//

#include "enumbra.h"
#include "cpp_generator.h"
#include <iostream>
#include <filesystem>
#include <cxxopts.hpp>
#include <toml++/toml.h>

std::string version = "0.0.1";

void print_help(const cxxopts::Options& options)
{
	std::cout << options.help() << std::endl;
}

void print_version()
{
	std::cout << "enumbra v" << version << std::endl;
}

void fail_exit(std::string_view sv = "", int code = -1)
{
	if (!sv.empty()) {
		std::cout << sv << std::endl;
	}
	exit(code);
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
			fail_exit("Config (-c|--config) argument is required.");
		}
		if (!result.count("s")) {
			fail_exit("Source (-s|--source) argument is required.");
		}

		auto config_file_path = result["c"].as<std::string>();
		auto source_file_path = result["s"].as<std::string>();

		if (!std::filesystem::exists(config_file_path)) {
			fail_exit("Config file does not exist.");
		}
		if (!std::filesystem::exists(source_file_path)) {
			fail_exit("Source file does not exist.");
		}

		auto verbosity = (result.count("v") ? enumbra::Verbosity::High : enumbra::Verbosity::Low);
		auto loaded_config = enumbra::load_config(config_file_path, verbosity);

		enumbra::enum_definition def;
		def.name = "TEST";

		cpp_generator cpp_gen;
		std::string cpp_out = cpp_gen.generate_cpp_output(loaded_config, def);

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

void load_cpp_config(enumbra::enumbra_config& enumbra_config, toml::node_view<toml::node>& cpp_config);
void load_csharp_config(enumbra::enumbra_config& enumbra_config, toml::node_view<toml::node>& cpp_config);

enumbra::enumbra_config enumbra::load_config(std::string_view config_toml_file, enumbra::Verbosity verbosity)
{
	enumbra::enumbra_config cfg;

	toml::table tbl = toml::parse_file(config_toml_file);

	toml::node_view<toml::node>& configuration = tbl["configuration"];
	toml::node_view<toml::node>& cpp_config = configuration["cpp_generator"];
	toml::node_view<toml::node>& csharp_config = configuration["csharp_generator"];

	std::optional<bool> generate_cpp = configuration["generate_cpp"].value<bool>();
	std::optional<bool> generate_csharp = configuration["generate_csharp"].value<bool>();

	if (generate_cpp.has_value())
	{
		cfg.generate_cpp = generate_cpp.value();
		if (cfg.generate_cpp) {
			load_cpp_config(cfg, cpp_config);
		}
	}
	if (generate_csharp.has_value())
	{
		cfg.generate_csharp = generate_csharp.value();
		if (cfg.generate_csharp) {
			load_csharp_config(cfg, csharp_config);
		}
	}

	return cfg;
}

template<typename T, typename node>
auto get_required(node& config, std::string_view param) {
	auto opt = config[param].value<T>();
	if (opt.has_value()) {
		return opt.value();
	}

	std::string x = "Configuration value is required: " + std::string(param);
	throw std::logic_error(x.c_str());
}

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

template<typename T, typename mapping>
T get_mapped(mapping& map, std::string_view field_name, std::string_view param) {
	if (map.size() == 0) {
		throw std::logic_error("Map passed to get_mapped has a size of 0");
	}

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

void load_cpp_config(enumbra::enumbra_config& enumbra_config, toml::node_view<toml::node>& cpp_cfg)
{
	using namespace enumbra::cpp;
	try {
		enumbra::cpp::cpp_config& c = enumbra_config.cpp_config;
		c.output_namespace = get_array<std::string>(cpp_cfg, "output_namespace");
		c.output_extension = get_required<std::string>(cpp_cfg, "output_extension");

		auto output_line_ending_style = get_required<std::string>(cpp_cfg, "output_line_ending_style");
		c.line_ending_style = get_mapped<LineEndingStyle>(LineEndingStyleMapped, "output_line_ending_style", output_line_ending_style);

		c.preamble_text = get_array<std::string>(cpp_cfg, "preamble_text");

		auto include_guard_text = get_required<std::string>(cpp_cfg, "include_guard");
		c.include_guard_style = get_mapped<IncludeGuardStyle>(IncludeGuardStyleMapped, "include_guard", include_guard_text);

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
			c.default_value_enum_size_type_index = c.get_size_type_index(default_value_enum_size_type);
			if (c.default_value_enum_size_type_index < 0) {
				throw std::logic_error("default_value_enum_size_type must reference an existing size_type.");
			}
		}

		{
			std::string default_flags_enum_size_type = get_required<std::string>(cpp_cfg, "default_flags_enum_size_type");
			c.default_flags_enum_size_type_index = c.get_size_type_index(default_flags_enum_size_type);
			if (c.default_flags_enum_size_type_index < 0) {
				throw std::logic_error("default_flags_enum_size_type must reference an existing size_type.");
			}
		}

		{
			std::vector<std::string> flags_enum_smallest_unsigned_evaluation_order = get_array<std::string>(cpp_cfg, "flags_enum_smallest_unsigned_evaluation_order");
			for (auto& str : flags_enum_smallest_unsigned_evaluation_order) {
				auto index = c.get_size_type_index(str);
				if (index < 0) {
					throw std::logic_error("flags_enum_smallest_unsigned_evaluation_order must reference an existing size_type.");
				}
				c.flags_enum_smallest_unsigned_evaluation_order.push_back(index);
			}
		}

		{
			std::vector<std::string> value_enum_smallest_unsigned_evaluation_order = get_array<std::string>(cpp_cfg, "value_enum_smallest_unsigned_evaluation_order");
			for (auto& str : value_enum_smallest_unsigned_evaluation_order) {
				auto index = c.get_size_type_index(str);
				if (index < 0) {
					throw std::logic_error("value_enum_smallest_unsigned_evaluation_order must reference an existing size_type.");
				}
				c.value_enum_smallest_unsigned_evaluation_order.push_back(index);
			}
		}

		{
			std::vector<std::string> value_enum_smallest_signed_evaluation_order = get_array<std::string>(cpp_cfg, "value_enum_smallest_signed_evaluation_order");
			for (auto& str : value_enum_smallest_signed_evaluation_order) {
				auto index = c.get_size_type_index(str);
				if (index < 0) {
					throw std::logic_error("value_enum_smallest_signed_evaluation_order must reference an existing size_type.");
				}
				c.value_enum_smallest_signed_evaluation_order.push_back(index);
			}
		}

		{
			auto string_table_layout = get_required<std::string>(cpp_cfg, "string_table_layout");
			c.string_table_layout = get_mapped<StringTableLayout>(StringTableLayoutMapped, "string_table_layout", string_table_layout);
			
			if (c.string_table_layout != StringTableLayout::None) {
				auto string_table_type = get_required<std::string>(cpp_cfg, "string_table_type");
				c.string_table_type = get_mapped<StringTableType>(StringTableTypeMapped, "string_table_type", string_table_type);
			}
		}

		c.bitwise_op_functions = get_required<bool>(cpp_cfg, "bitwise_op_functions");
		c.bounds_check_functions = get_required<bool>(cpp_cfg, "bounds_check_functions");
		c.density_functions = get_required<bool>(cpp_cfg, "density_functions");
		c.min_max_functions = get_required<bool>(cpp_cfg, "min_max_functions");
		c.bit_info_functions = get_required<bool>(cpp_cfg, "bit_info_functions");
		c.flag_helper_functions = get_required<bool>(cpp_cfg, "flag_helper_functions");
		c.packed_declaration_macros = get_required<bool>(cpp_cfg, "packed_declaration_macros");
	}
	catch (const std::exception& e) {

		std::string x = std::string("load_cpp_config: ") + e.what();
		throw std::logic_error(x.c_str());
	}
}

void load_csharp_config(enumbra::enumbra_config& c, toml::node_view<toml::node>& cpp_config)
{
	throw std::logic_error("load_csharp_config not implemented. Set generate_csharp to false.");
}

int64_t enumbra::cpp::cpp_config::get_size_type_index(std::string_view name)
{
	auto type = std::find_if(size_types.begin(), size_types.end(), [&name](enumbra::cpp::enum_size_type& size_type) {
		return size_type.name == name;
		});
	if (type != size_types.end()) {
		return type - size_types.begin();
	}
	return -1;
}
