#pragma once

#include "enumbra.h"
#include "cpp_utility.h"

#include <array>
#include <string>
#include <string_view>
#include <unordered_map>
#include <optional>
#include <fmt/format.h>
#include <fmt/args.h>

#include <type_traits>

using namespace std::string_view_literals;
using namespace std::string_literals;

struct string_lookup_table {
    size_t offset_str = 0;
    size_t offset_enum = 0;
    size_t count = 0;
    size_t size = 0;
    std::vector<std::string> names;
};
struct string_lookup_tables {
    std::vector<string_lookup_table> tables;
    std::vector<enumbra::enum_entry> entries;
};

struct value_enum_context {
    std::string enum_name;
    std::vector<enumbra::enum_entry> values;

    enumbra::enum_entry default_entry;
    enumbra::enum_entry min_entry;
    enumbra::enum_entry max_entry;
    int64_t entry_count = 0;
    std::string size_type_str;
    bool is_size_type_signed = false;
    int64_t size_type_bits = 0;
    int64_t bits_required_storage = 0;
    int64_t bits_required_transmission = 0;
    int64_t unique_entry_count = 0;
    bool is_range_contiguous = false;
    bool is_one_string_table = false;
    std::optional<int128> invalid_sentinel;

    string_lookup_tables string_tables;
};

struct flags_enum_context {

};

struct output_context {
    std::vector<size_t> header_guard_positions;

    std::string enum_ns;

    std::vector<value_enum_context> value_enums;
    std::vector<flags_enum_context> flags_enums;
};

class cpp_generator {
public:
    cpp_generator(const enumbra::enumbra_config& cfg, const enumbra::enum_meta_config& enum_meta);

	const std::string& generate_cpp_output();

private:
    const enumbra::cpp::cpp_config& cpp_cfg;
    const enumbra::enum_meta_config& enum_meta;

    void build_contexts();

    // Shared
    void emit_preamble();
    void emit_include_guard_begin();
    void emit_include_guard_end();
    void emit_includes();
    void emit_required_macros();
    void emit_optional_macros();
    void emit_templates();

    // Value enums
    void emit_ve_definition(const value_enum_context& e);
    void emit_ve_detail(const value_enum_context& e);
    void emit_ve_func_values(const value_enum_context& e);
    void emit_ve_func_from_integer(const value_enum_context& e);
    void emit_ve_func_is_valid(const value_enum_context& e);
    void emit_ve_func_enum_name(const value_enum_context& e);
    void emit_ve_func_to_string(const value_enum_context& e);
    void emit_ve_func_from_string_with_size(const value_enum_context& e);
    void emit_ve_func_from_string_cstr(const value_enum_context& e);

private:
    output_context ctx;
	std::string output; // Final output

    // Argument store
    std::unordered_map<std::string, std::string> store_map_;
    fmt::dynamic_format_arg_store<fmt::format_context> store_;

    void push(std::string_view key, std::string_view value)
    {
        store_map_.insert_or_assign(std::string(key), std::string(value));
        store_.push_back(fmt::arg(key.data(), value.data()));
    }
    void pop(std::string_view key)
    {
        auto removed = store_map_.erase(std::string(key));
        if(removed > 0)
        {
            store_.clear();
            for(auto& kv : store_map_)
            {
                store_.push_back(fmt::arg(kv.first.c_str(), kv.second.c_str()));
            }
        }
    }
    void clear_store() { 
        store_map_.clear(); 
        store_.clear(); 
    }

    // Write functions
	template <typename... Args>
	void write(std::string_view fmt, Args&... args) {
		fmt::vformat_to(std::back_inserter(output), fmt, fmt::make_format_args(args...));
	}

    // write line feed
	void wlf(int count = 1) {
		for (int i = 0; i < count; i++)
			output += "\n";
	}

    // write line - user provides args and doesn't use parameter store
	template <typename... Args>
	void wl(std::string_view fmt, Args&&... args) {
		write(fmt, args...);
        wlf();
	}

    // write line unformatted - for pre-formatted strings
    void wlu(std::string_view str) {
        output += str;
        wlf();
    }

    // write virtual line - using parameter store
    void wvl(std::string_view fmt) {
        fmt::vformat_to(std::back_inserter(output), fmt, store_);
        wlf();
    }
};
