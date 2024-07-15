#pragma once

#include "enumbra.h"
#include <array>
#include <string>
#include <string_view>
#include <unordered_map>
#include <fmt/format.h>
#include <fmt/args.h>

using namespace std::string_view_literals;
using namespace std::string_literals;

class cpp_generator {
public:
	const std::string& generate_cpp_output(const enumbra::enumbra_config& cfg, const enumbra::enum_meta_config& enum_meta);

private:
	std::string Output; // Final output

    // Argument store
    std::unordered_map<std::string, std::string> store_map_;
    fmt::dynamic_format_arg_store<fmt::format_context> store_;

    void push(std::string_view key, std::string_view value)
    {
        store_map_.insert_or_assign(std::string(key), std::string(value));
        store_.clear();
        for(auto& kv : store_map_)
        {
            store_.push_back(fmt::arg(kv.first.c_str(), kv.second.c_str()));
        }
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
    void clear_store() { store_.clear(); }

	template <typename... Args>
	void write(std::string_view fmt, Args&&... args) {
		fmt::format_to(std::back_inserter(Output), fmt, std::forward<Args>(args)...);
	}

    // write line feed
	void wlf(int count = 1) {
		for (int i = 0; i < count; i++)
			write("\n");
	}

    // write line - user provides args and doesn't use parameter store
	template <typename... Args>
	void wl(std::string_view fmt, Args&&... args) {
		write(fmt, args...);
        wlf();
	}

    // write line unformatted - for pre-formatted strings which contain fmt characters
    void wl_unformatted(std::string_view str) {
        Output += str;
        wlf();
    }

    // write line using parameter store
    void wvl(std::string_view fmt) {
        fmt::vformat_to(std::back_inserter(Output), fmt, store_);
        wlf();
    }
};
