// enumbra.cpp : Defines the entry point for the application.
//

#include "enumbra.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cxxopts.hpp>
#include "cpp_generator.h"

using namespace enumbra;

enumbra::enumbra_config load_enumbra_config(const std::string& config_file);
enumbra::enum_meta_config load_meta_config(enumbra::enumbra_config& enumbra_config, const std::string& config_file);
void parse_enumbra_cpp(enumbra::enumbra_config& enumbra_config, json& cpp_config);
void parse_enumbra_csharp(enumbra::enumbra_config& enumbra_config, json& csharp_config);
void parse_enum_meta(enumbra::enumbra_config& enumbra_config, enumbra::enum_meta_config& enum_config, json& meta_config);

void print_help(const cxxopts::Options& options)
{
	std::cout << options.help() << std::endl;
}

void print_version()
{
	std::cout << "enumbra v" << kEnumbraVersion << std::endl;
}

int main(int argc, char** argv)
{
	try {
		cxxopts::Options options("enumbra", "An enum code generator. https://github.com/Scaless/enumbra");
		options
			.custom_help("-c enumbra_config.json -s enum.json")
			.add_options()
			("h,help", "You are here.")
			("c,config", "[Required] Path to enumbra config file (enumbra_config.json).", cxxopts::value<std::string>())
			("s,source", "[Required] Path to enum config file (enum.json).", cxxopts::value<std::string>())
			("cppout", "[Required] Path to output C++ header file.", cxxopts::value<std::string>())
			("version", "Prints version information.")
			("p,print", "Prints output to the console.");

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
			throw std::logic_error("Enumbra Config File (-c|--config) argument is required.");
		}
		if (!result.count("s")) {
			throw std::logic_error("Enum Source File (-s|--source) argument is required.");
		}
		if (!result.count("cppout")) {
			throw std::logic_error("C++ Output File Path (--cppout) argument is required.");
		}

		auto config_file_path = result["c"].as<std::string>();
		auto source_file_path = result["s"].as<std::string>();
		auto cppout_file_path = result["cppout"].as<std::string>();

		if (!std::filesystem::exists(config_file_path)) {
			throw std::logic_error("Config file does not exist.");
		}
		if (!std::filesystem::exists(source_file_path)) {
			throw std::logic_error("Source file does not exist.");
		}

		auto loaded_enumbra_config = load_enumbra_config(config_file_path);
		auto enum_config = load_meta_config(loaded_enumbra_config, source_file_path);

		if (loaded_enumbra_config.generate_cpp)
		{
			cpp_generator cpp_gen;
			const std::string& generated_cpp = cpp_gen.generate_cpp_output(loaded_enumbra_config, enum_config);

			if (result.count("p")) {
				std::cout << generated_cpp << std::endl;
			}
			std::ofstream file(cppout_file_path);
			file << generated_cpp;

			std::cout << "The cpp file was successfully output at: " << cppout_file_path << std::endl;
		}
		if (loaded_enumbra_config.generate_csharp)
		{
			// TODO
		}
	}
	catch (const std::exception& e) {
		// TODO: print a better stacktrace
		std::cout << e.what();
		return -1;
	}

	return 0;
}


enumbra::enumbra_config load_enumbra_config(const std::string& config_file)
{
	enumbra::enumbra_config cfg;

	std::ifstream file(config_file);
	json data = json::parse(file, nullptr, true, true);

	auto configuration = data["enumbra_config"];
	auto cpp_config = configuration["cpp_generator"];
	auto csharp_config = configuration["csharp_generator"];

	if (cpp_config.is_object()) {
		parse_enumbra_cpp(cfg, cpp_config);
	}
	if (csharp_config.is_object()) {
		parse_enumbra_csharp(cfg, csharp_config);
	}

	return cfg;
}

enumbra::enum_meta_config load_meta_config(enumbra::enumbra_config& enumbra_config, const std::string& config_file)
{
	enumbra::enum_meta_config cfg;

	std::ifstream file(config_file);
	json data = json::parse(file, nullptr, true, true);

	auto enum_meta = data["enums"];

	parse_enum_meta(enumbra_config, cfg, enum_meta);

	return cfg;
}

void parse_enumbra_cpp(enumbra::enumbra_config& enumbra_config, json& json_cfg)
{
	using namespace enumbra::cpp;
	try {
		cpp_config& c = enumbra_config.cpp_config;

		c.output_namespace = get_array<std::string>(json_cfg["output_namespace"]);
		c.line_ending_style = get_mapped<LineEndingStyle>(LineEndingStyleMapped, json_cfg["output_line_ending_style"]);
		c.time_generated_in_header = json_cfg["time_generated_in_header"].get<bool>();
		c.output_tab_characters = json_cfg["output_tab_characters"].get<std::string>();
		c.preamble_text = get_array<std::string>(json_cfg["preamble_text"]);
		c.include_guard_style = get_mapped<IncludeGuardStyle>(IncludeGuardStyleMapped, json_cfg["include_guard"]);
		c.additional_includes = get_array<std::string>(json_cfg["additional_includes"]);

		c.value_enum_name_prefix = json_cfg.value("value_enum_name_prefix", "");
		c.flags_enum_name_prefix = json_cfg.value("flags_enum_name_prefix", "");
		c.value_enum_name_postfix = json_cfg.value("value_enum_name_postfix", "");
		c.flags_enum_name_postfix = json_cfg.value("flags_enum_name_postfix", "");

		c.value_enum_value_prefix = json_cfg.value("value_enum_value_prefix", "");
		c.flags_enum_value_prefix = json_cfg.value("flags_enum_value_prefix", "");
		c.value_enum_value_postfix = json_cfg.value("value_enum_value_postfix", "");
		c.flags_enum_value_postfix = json_cfg.value("flags_enum_value_postfix", "");

		for (auto iter : json_cfg["size_types"])
		{
			enum_size_type t;
			t.name = iter["name"].get<std::string>();
			t.bits = iter["bits"].get<int64_t>();
			t.is_signed = iter["is_signed"].get<bool>();
			t.type_name = iter["type_name"].get<std::string>();
			c.size_types.push_back(t);
		}
		if (c.size_types.size() == 0) {
			throw std::logic_error("size_types array is required. See the enumbra documentation for details.");
		}

		std::string default_value_enum_size_type = json_cfg["default_value_enum_size_type"].get<std::string>();
		c.default_value_enum_size_type_index = c.get_size_type_index_from_name(default_value_enum_size_type);
		if (c.default_value_enum_size_type_index == SIZE_MAX) {
			throw std::logic_error("default_value_enum_size_type must reference an existing size_type.");
		}

		std::string default_flags_enum_size_type = json_cfg["default_flags_enum_size_type"].get<std::string>();
		c.default_flags_enum_size_type_index = c.get_size_type_index_from_name(default_flags_enum_size_type);
		if (c.default_flags_enum_size_type_index == SIZE_MAX) {
			throw std::logic_error("default_flags_enum_size_type must reference an existing size_type.");
		}

		std::vector<std::string> flags_enum_smallest_unsigned_evaluation_order = get_array<std::string>(json_cfg["flags_enum_smallest_unsigned_evaluation_order"]);
		for (auto& str : flags_enum_smallest_unsigned_evaluation_order) {
			auto index = c.get_size_type_index_from_name(str);
			if (index == SIZE_MAX) {
				throw std::logic_error("flags_enum_smallest_unsigned_evaluation_order must reference an existing size_type.");
			}
			c.flags_enum_smallest_unsigned_evaluation_order.push_back(index);
		}

		std::vector<std::string> value_enum_smallest_unsigned_evaluation_order = get_array<std::string>(json_cfg["value_enum_smallest_unsigned_evaluation_order"]);
		for (auto& str : value_enum_smallest_unsigned_evaluation_order) {
			auto index = c.get_size_type_index_from_name(str);
			if (index == SIZE_MAX) {
				throw std::logic_error("value_enum_smallest_unsigned_evaluation_order must reference an existing size_type.");
			}
			c.value_enum_smallest_unsigned_evaluation_order.push_back(index);
		}

		std::vector<std::string> value_enum_smallest_signed_evaluation_order = get_array<std::string>(json_cfg["value_enum_smallest_signed_evaluation_order"]);
		for (auto& str : value_enum_smallest_signed_evaluation_order) {
			auto index = c.get_size_type_index_from_name(str);
			if (index == SIZE_MAX) {
				throw std::logic_error("value_enum_smallest_signed_evaluation_order must reference an existing size_type.");
			}
			c.value_enum_smallest_signed_evaluation_order.push_back(index);
		}

		c.string_table_layout = get_mapped<StringTableLayout>(StringTableLayoutMapped, json_cfg["string_table_layout"]);
		if (c.string_table_layout != StringTableLayout::None) {
			c.string_table_type = get_mapped<StringTableType>(StringTableTypeMapped, json_cfg["string_table_type"]);
		}

		c.min_max_functions = json_cfg["min_max_functions"].get<bool>();
		c.bit_info_functions = json_cfg["bit_info_functions"].get<bool>();
		c.enumbra_bitfield_macros = json_cfg["enumbra_bitfield_macros"].get<bool>();
	}
	catch (const std::exception& e) {

		std::string x = std::string("parse_enumbra_cpp_config: ") + e.what();
		throw std::logic_error(x.c_str());
	}
}

void parse_enumbra_csharp(enumbra::enumbra_config& /*enumbra_config*/, json& /*csharp_config*/)
{
	throw std::logic_error("parse_enumbra_csharp not implemented.");
}

bool is_pow_2(int128 x)
{
	return x && !(x & (x - 1));
}

void parse_enum_meta(enumbra::enumbra_config& enumbra_config, enumbra::enum_meta_config& enum_config, json& meta_config) {

	enum_config.block_name = meta_config["block_name"];
	enum_config.value_enum_default_value_style = get_mapped<ValueEnumDefaultValueStyle>(ValueEnumDefaultValueStyleMapped, meta_config["value_enum_default_value_style"]);
	enum_config.flags_enum_default_value_style = get_mapped<FlagsEnumDefaultValueStyle>(FlagsEnumDefaultValueStyleMapped, meta_config["flags_enum_default_value_style"]);

	for (auto& value_enum : meta_config["value_enums"])
	{
		enum_definition def;
		def.name = value_enum["name"].get<std::string>();

		std::string size_type_string = value_enum.value("size_type", "");
		if (size_type_string != "")
		{
			def.size_type_index = enumbra_config.cpp_config.get_size_type_index_from_name(size_type_string);
			if (def.size_type_index == SIZE_MAX) {
				throw std::logic_error("value_enum size_type does not exist in global types table: " + size_type_string);
			}
		}
		else
		{
			// use the default size_type
			def.size_type_index = enumbra_config.cpp_config.default_value_enum_size_type_index;
		}

		def.default_value_name = value_enum.value("default_value", "");

		int128 current_value = 0;
		for (auto& entry : value_enum["entries"])
		{
			enum_entry ee;
			ee.name = entry["name"].get<std::string>();
			ee.description = entry.value("description", "");

			auto entry_value = entry["value"];
			if (entry_value.is_null())
			{
				ee.p_value = current_value;
			}
			else if (entry_value.is_string())
			{
				// TODO: Parse as string
				throw std::logic_error("string values are not yet supported");
			}
			else if (entry_value.is_number_unsigned())
			{
				ee.p_value = entry_value.get<uint64_t>();
			}
			else if (entry_value.is_number_integer())
			{
				ee.p_value = entry_value.get<int64_t>();
			}
			else
			{
				throw std::logic_error("entry_value type is not valid");
			}

			current_value = ee.p_value + 1;
			def.values.push_back(ee);
		}

		enum_config.value_enum_definitions.push_back(def);
	}

	for (auto& flags_enum : meta_config["flags_enums"])
	{
		enum_definition def;
		def.name = flags_enum["name"].get<std::string>();
		std::string size_type_string = flags_enum.value("size_type", "");
		if (size_type_string != "")
		{
			def.size_type_index = enumbra_config.cpp_config.get_size_type_index_from_name(size_type_string);
			if (def.size_type_index == SIZE_MAX) {
				throw std::logic_error("flags_enum size_type does not exist in global types table: " + size_type_string);
			}
		}
		else
		{
			// use the default size_type
			def.size_type_index = enumbra_config.cpp_config.default_flags_enum_size_type_index;
		}
		def.default_value_name = flags_enum.value("default_value", "");

		int64_t current_shift = 0;
		for (auto& entry : flags_enum["entries"])
		{
			enum_entry ee;
			ee.name = entry["name"].get<std::string>();
			ee.description = entry.value("description", "");

			auto entry_value = entry["value"];
			if (entry_value.is_null())
			{
				ee.p_value = 1LL << current_shift;
			}
			else if (entry_value.is_string())
			{
				// TODO: Parse as string
				throw std::logic_error("string values are not yet supported");
			}
			else if (entry_value.is_number_unsigned())
			{
				ee.p_value = entry_value.get<uint64_t>();
			}
			else if (entry_value.is_number_integer())
			{
				ee.p_value = entry_value.get<int64_t>();
			}
			else
			{
				throw std::logic_error("entry_value type is not valid");
			}

			if (!is_pow_2(ee.p_value)) {
				throw std::logic_error("flags_enum value is not a power of 2 (1 bit set). Non-pow2 values are not currently supported.");
			}
			current_shift++;
			while ((1LL << current_shift) < ee.p_value) {
				current_shift++;
			}
			def.values.push_back(ee);
		}

		enum_config.flag_enum_definitions.push_back(def);
	}

	enum_config = enum_config;
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
