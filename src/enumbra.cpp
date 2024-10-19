// enumbra.cpp : Defines the entry point for the application.
//

#include "enumbra.h"
#include <cstdio>
#include <filesystem>
#include <charconv>
#include <fstream>
#include <cxxopts.hpp>
#include <absl/strings/strip.h>
#include "cpp_generator.h"

namespace {
    bool contains_whitespace(const std::string_view str)
    {
        for(auto& s : str) {
            if(std::isspace(s)) {
                return true;
            }
        }
        return false;
    }
}

using namespace enumbra;

enumbra::enumbra_config load_enumbra_config(const std::string &config_file);

enumbra::enum_meta_config load_meta_config(enumbra::enumbra_config &enumbra_config, const std::string &config_file);

void parse_enumbra_cpp(enumbra::enumbra_config &enumbra_config, json &cpp_cfg);

void parse_enumbra_csharp(enumbra::enumbra_config &enumbra_config, json &csharp_cfg);

void parse_enum_meta(enumbra::enumbra_config &enumbra_config, enumbra::enum_meta_config &enum_config, json &meta_config);

void print_help(const cxxopts::Options &options) {
    printf("%s\n", options.help().c_str());
}

void print_version() {
    printf("enumbra v%s\n", kEnumbraVersion);
}

int main(int argc, char **argv) {
    try {
        cxxopts::Options options("enumbra", "An enum code generator. https://github.com/Scaless/enumbra");
        options
                .custom_help("-c enumbra_config.json -s enum.json")
                .add_options()
                        ("h,help", "You are here.")
                        ("c,config", "[Required] Path to enumbra config file (enumbra_config.json).",
                         cxxopts::value<std::string>())
                        ("s,source", "[Required] Path to enum config file (enum.json).", cxxopts::value<std::string>())
                        ("cppout", "[Required] Path to output C++ header file.", cxxopts::value<std::string>())
                        ("version", "Prints version information.")
                        ("p,print", "Prints output to the console.");

        auto result = options.parse(argc, argv);

        if (result.count("h") || result.arguments().empty()) {
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

        if (loaded_enumbra_config.generate_cpp) {
            cpp_generator cpp_gen(loaded_enumbra_config, enum_config);
            const std::string &generated_cpp = cpp_gen.generate_cpp_output();

            if (result.count("p")) {
                printf("%s\n", generated_cpp.c_str());
            }
            std::ofstream file(cppout_file_path);
            file << generated_cpp;

            printf("The cpp file was successfully output to: %s\n", cppout_file_path.c_str());
        }
        if (loaded_enumbra_config.generate_csharp) {
            // TODO
        }
    }
    catch (const std::exception &e) {
        // TODO: print a better stacktrace
        printf("%s\n", e.what());
        return -1;
    }

    return 0;
}


enumbra::enumbra_config load_enumbra_config(const std::string &config_file) {
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

enumbra::enum_meta_config load_meta_config(enumbra::enumbra_config &enumbra_config, const std::string &config_file) {
    enumbra::enum_meta_config cfg;

    std::ifstream file(config_file);
    json data = json::parse(file, nullptr, true, true);

    auto enum_meta = data["enums"];

    parse_enum_meta(enumbra_config, cfg, enum_meta);

    return cfg;
}

void parse_enumbra_cpp(enumbra::enumbra_config &enumbra_config, json &cpp_cfg) {
    using namespace enumbra::cpp;
    try {
        cpp_config &c = enumbra_config.cpp_config;

        c.output_namespace = get_array<std::string>(cpp_cfg["output_namespace"]);
        c.time_generated_in_header = cpp_cfg["time_generated_in_header"].get<bool>();
        c.preamble_text = get_array<std::string>(cpp_cfg["preamble_text"]);
        c.include_guard_style = get_mapped<IncludeGuardStyle>(IncludeGuardStyleMapped, cpp_cfg["include_guard"]);
        c.additional_includes = get_array<std::string>(cpp_cfg["additional_includes"]);

        for (auto iter: cpp_cfg["size_types"]) {
            enum_size_type t;
            t.name = iter["name"].get<std::string>();
            t.bits = iter["bits"].get<int32_t>();
            t.is_signed = iter["is_signed"].get<bool>();
            t.type_name = iter["type_name"].get<std::string>();

            if (t.name.empty()) {
                throw std::logic_error("size type name cannot be empty");
            }
            if (t.bits > 64) {
                throw std::logic_error("size type bits cannot be greater than 64");
            }
            if (t.type_name.empty()) {
                throw std::logic_error("size type type_name cannot be empty");
            }

            if (t.is_signed) {
                // To get the maximum, we fill all bits except the sign bit
                t.max_possible_value = 0;
                for (int32_t i = 0; i < t.bits - 1; i++) {
                    t.max_possible_value |= (int128(1) << static_cast<int>(i));
                }

                // Minimum is found by inverting the max value, then subtract 1
                t.min_possible_value = -t.max_possible_value - 1;
            } else {
                // To get the maximum, we fill all bits
                t.max_possible_value = 0;
                for (int32_t i = 0; i < t.bits; i++) {
                    t.max_possible_value |= (int128(1) << static_cast<int>(i));
                }

                // Minimum always 0
                t.min_possible_value = 0;
            }

            c.size_types.push_back(t);
        }
        if (c.size_types.empty()) {
            throw std::logic_error("size_types array is required. See the enumbra documentation for details.");
        }

        std::string default_value_enum_size_type = cpp_cfg["default_value_enum_size_type"].get<std::string>();
        c.default_value_enum_size_type_index = c.get_size_type_index_from_name(default_value_enum_size_type);
        if (c.default_value_enum_size_type_index == SIZE_MAX) {
            throw std::logic_error("default_value_enum_size_type must reference an existing size_type.");
        }

        std::string default_flags_enum_size_type = cpp_cfg["default_flags_enum_size_type"].get<std::string>();
        c.default_flags_enum_size_type_index = c.get_size_type_index_from_name(default_flags_enum_size_type);
        if (c.default_flags_enum_size_type_index == SIZE_MAX) {
            throw std::logic_error("default_flags_enum_size_type must reference an existing size_type.");
        }

        std::vector<std::string> flags_enum_smallest_unsigned_evaluation_order = get_array<std::string>(
                cpp_cfg["flags_enum_smallest_unsigned_evaluation_order"]);
        for (auto &str: flags_enum_smallest_unsigned_evaluation_order) {
            auto index = c.get_size_type_index_from_name(str);
            if (index == SIZE_MAX) {
                throw std::logic_error(
                        "flags_enum_smallest_unsigned_evaluation_order must reference an existing size_type.");
            }
            c.flags_enum_smallest_unsigned_evaluation_order.push_back(index);
        }

        std::vector<std::string> value_enum_smallest_unsigned_evaluation_order = get_array<std::string>(
                cpp_cfg["value_enum_smallest_unsigned_evaluation_order"]);
        for (auto &str: value_enum_smallest_unsigned_evaluation_order) {
            auto index = c.get_size_type_index_from_name(str);
            if (index == SIZE_MAX) {
                throw std::logic_error(
                        "value_enum_smallest_unsigned_evaluation_order must reference an existing size_type.");
            }
            c.value_enum_smallest_unsigned_evaluation_order.push_back(index);
        }

        std::vector<std::string> value_enum_smallest_signed_evaluation_order = get_array<std::string>(
                cpp_cfg["value_enum_smallest_signed_evaluation_order"]);
        for (auto &str: value_enum_smallest_signed_evaluation_order) {
            auto index = c.get_size_type_index_from_name(str);
            if (index == SIZE_MAX) {
                throw std::logic_error(
                        "value_enum_smallest_signed_evaluation_order must reference an existing size_type.");
            }
            c.value_enum_smallest_signed_evaluation_order.push_back(index);
        }

        c.string_table_layout = get_mapped<StringTableLayout>(StringTableLayoutMapped, cpp_cfg["string_table_layout"]);

        c.min_max_functions = cpp_cfg["min_max_functions"].get<bool>();
        c.bit_info_functions = cpp_cfg["bit_info_functions"].get<bool>();
        c.enumbra_bitfield_macros = cpp_cfg["enumbra_bitfield_macros"].get<bool>();
    }
    catch (const std::exception &e) {

        std::string x = std::string("parse_enumbra_cpp_config: ") + e.what();
        throw std::logic_error(x.c_str());
    }
}

void parse_enumbra_csharp(enumbra::enumbra_config & /*enumbra_config*/, json & /*csharp_cfg*/) {
    throw std::logic_error("parse_enumbra_csharp not implemented.");
}

bool is_pow_2(int128 x) {
    return x && !(x & (x - 1));
}

int128 string_to_int128(const std::string_view s) {
    const bool bIsHexString = (s.size() >= 2) && (s[0] == '0') && ((s[1] == 'x') || (s[1] == 'X'));
    const bool bIsBinaryString = (s.size() >= 2) && (s[0] == '0') && ((s[1] == 'b'));
    if (bIsHexString) {
        // Unsigned Hex
        uint64_t unsigned_value;
        auto result = std::from_chars(s.data() + 2, s.data() + s.size(), unsigned_value, 16);
        const bool bConsumedEntireString = result.ptr == (s.data() + s.size());
        if (bConsumedEntireString && result.ec == std::errc{}) {
            return int128(unsigned_value);
        }
    } else if (bIsBinaryString) {
        // Unsigned Binary
        uint64_t unsigned_value;
        auto result = std::from_chars(s.data() + 2, s.data() + s.size(), unsigned_value, 2);
        const bool bConsumedEntireString = result.ptr == (s.data() + s.size());
        if (bConsumedEntireString && result.ec == std::errc{}) {
            return int128(unsigned_value);
        }
    } else {
        // Signed Base 10
        {
            int64_t signed_value;
            auto result = std::from_chars(s.data(), s.data() + s.size(), signed_value);
            const bool bConsumedEntireString = result.ptr == (s.data() + s.size());
            if (bConsumedEntireString && result.ec == std::errc{}) {
                return int128(signed_value);
            }
        }

        // Unsigned Base 10
        {
            uint64_t unsigned_value;
            auto result = std::from_chars(s.data(), s.data() + s.size(), unsigned_value);
            const bool bConsumedEntireString = result.ptr == (s.data() + s.size());
            if (bConsumedEntireString && result.ec == std::errc{}) {
                return int128(unsigned_value);
            }
        }
    }

    throw std::invalid_argument(fmt::format("\"{0}\" could not be parsed to an integer.", s));
}

// Throws if v will not be storable in the defined size_type
void validate_value_fits_in_size_type(const enumbra::cpp::enum_size_type &size_type, const int128 v) {
    if ((v < size_type.min_possible_value) || (v > size_type.max_possible_value)) {
        auto to_string_128 = [](int128 v) -> std::string {
            return (v < 0) ? fmt::format("{0}", static_cast<int64_t>(v)) : fmt::format("{0}", static_cast<uint64_t>(v));
        };
        auto to_string_128_hex = [](int128 v) -> std::string {
            return (v < 0) ? fmt::format("{0:#x}", static_cast<int64_t>(v)) : fmt::format("{0:#x}",
                                                                                          static_cast<uint64_t>(v));
        };
        throw std::logic_error(fmt::format("entry_value {0} ({1}) is out of the possible range {2} to {3}",
                                           to_string_128(v),
                                           to_string_128_hex(v),
                                           to_string_128(size_type.min_possible_value),
                                           to_string_128(size_type.max_possible_value)
        ));
    }
}

void
parse_enum_meta(enumbra::enumbra_config &enumbra_config, enumbra::enum_meta_config &enum_config, json &meta_config) {

    enum_config.block_name = meta_config["block_name"];
    enum_config.value_enum_default_value_style = get_mapped<ValueEnumDefaultValueStyle>(
            ValueEnumDefaultValueStyleMapped, meta_config["value_enum_default_value_style"]);
    enum_config.flags_enum_default_value_style = get_mapped<FlagsEnumDefaultValueStyle>(
            FlagsEnumDefaultValueStyleMapped, meta_config["flags_enum_default_value_style"]);

    for (auto &value_enum: meta_config["value_enums"]) {
        enum_definition def;
        def.name = value_enum["name"].get<std::string>();

        std::string size_type_string = value_enum.value("size_type", "");
        if (!size_type_string.empty()) {
            def.size_type_index = enumbra_config.cpp_config.get_size_type_index_from_name(size_type_string);
            if (def.size_type_index == SIZE_MAX) {
                throw std::logic_error(
                        "value_enum size_type does not exist in global types table: " + size_type_string);
            }
        } else {
            // use the default size_type
            def.size_type_index = enumbra_config.cpp_config.default_value_enum_size_type_index;
        }

        def.default_value_name = value_enum.value("default_value", "");

        int128 current_value = 0;
        for (auto &entry: value_enum["entries"]) {
            enum_entry ee;
            ee.name = entry["name"].get<std::string>();
            ee.description = entry.value("description", "");

            if(ee.name.empty()) {
                throw std::logic_error("enum value name is empty");
            }
            if(contains_whitespace((ee.name))) {
                throw std::logic_error("enum value name contains whitespace character");
            }

            auto entry_value = entry["value"];
            if (entry_value.is_null()) {
                ee.p_value = current_value;
            } else if (entry_value.is_string()) {
                ee.p_value = string_to_int128(entry_value.get<std::string>());
            } else if (entry_value.is_number_unsigned()) {
                ee.p_value = entry_value.get<uint64_t>();
            } else if (entry_value.is_number_integer()) {
                ee.p_value = entry_value.get<int64_t>();
            } else {
                throw std::logic_error("entry_value type is not valid");
            }

            auto size_type = enumbra_config.cpp_config.get_size_type_from_index(def.size_type_index);
            validate_value_fits_in_size_type(size_type, ee.p_value);

            current_value = ee.p_value + 1;
            def.values.push_back(ee);
        }

        std::sort(def.values.begin(), def.values.end(),
                  [](const enum_entry &a, const enum_entry &b) { return a.p_value < b.p_value; }
        );

        enum_config.value_enum_definitions.push_back(def);
    }


    for (auto &flags_enum: meta_config["flags_enums"]) {
        enum_definition def;
        def.name = flags_enum["name"].get<std::string>();
        std::string size_type_string = flags_enum.value("size_type", "");
        if (!size_type_string.empty()) {
            def.size_type_index = enumbra_config.cpp_config.get_size_type_index_from_name(size_type_string);
            if (def.size_type_index == SIZE_MAX) {
                throw std::logic_error(
                        "flags_enum size_type does not exist in global types table: " + size_type_string);
            }
        } else {
            // use the default size_type
            def.size_type_index = enumbra_config.cpp_config.default_flags_enum_size_type_index;
        }
        def.default_value_name = flags_enum.value("default_value", "");

        int64_t current_shift = 0;
        for (auto &entry: flags_enum["entries"]) {
            enum_entry ee;
            ee.name = entry["name"].get<std::string>();
            ee.description = entry.value("description", "");

            auto entry_value = entry["value"];
            if (entry_value.is_null()) {
                ee.p_value = 1LL << current_shift;
            } else if (entry_value.is_string()) {
                ee.p_value = string_to_int128(entry_value.get<std::string>());
            } else if (entry_value.is_number_unsigned()) {
                ee.p_value = entry_value.get<uint64_t>();
            } else if (entry_value.is_number_integer()) {
                ee.p_value = entry_value.get<int64_t>();
            } else {
                throw std::logic_error("entry_value type is not valid");
            }

            auto &size_type = enumbra_config.cpp_config.get_size_type_from_index(def.size_type_index);
            validate_value_fits_in_size_type(size_type, ee.p_value);

            if (!is_pow_2(ee.p_value)) {
                throw std::logic_error(
                        "flags_enum value is not a power of 2 (1 bit set). Non-pow2 values are not currently supported.");
            }
            current_shift++;
            while ((1LL << current_shift) < ee.p_value) {
                current_shift++;
            }
            def.values.push_back(ee);
        }

        std::sort(def.values.begin(), def.values.end(),
                  [](const enum_entry &a, const enum_entry &b) { return a.p_value < b.p_value; }
        );

        enum_config.flag_enum_definitions.push_back(def);
    }
}

size_t enumbra::cpp::cpp_config::get_size_type_index_from_name(std::string_view name) {
    for (size_t i = 0; i < size_types.size(); i++) {
        if (size_types[i].name == name) {
            return i;
        }
    }
    return SIZE_MAX;
}

const enumbra::cpp::enum_size_type &enumbra::cpp::cpp_config::get_size_type_from_index(size_t index) const {
    // Will throw if index out of bounds
    return size_types.at(index);
}
