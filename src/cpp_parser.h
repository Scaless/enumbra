#pragma once

#include <string>
#include <cctype> // For std::isspace
#include <absl/numeric/int128.h>
#include <absl/strings/str_format.h>
#include "cpp_parser_gpt_enums.h"
#include "cpp_parser_gpt_namespace.h"
#include "cpp_parser_gpt_comments.h"
#include "cpp_parser_gpt_values.h"
#include "cpp_parser_gpt_eval.h"

using int128 = absl::int128;
using uint128 = absl::uint128;

enum class enum_type {
	enum_raw,
	enum_class
};

struct cpp_parse_result {
	enum_type type;
    bool fail = true;
};

std::string strip_whitespace(const std::string& input) {
    std::string out;
    for (char c : input) {
        if (!std::isspace(c)) {
            out += c;
        }
    }
    return out;
}

std::string replace_substring(const std::string& input, const std::string& from, const std::string& to) {
    std::string out = input;
    size_t current = 0;
    while ((current = out.find(from, current)) != std::string::npos) {
        // If found, replace and move past it
        out.replace(current, from.length(), to);
        current += to.length();
    }
    return out;
}

enum {
    V = 0
};

inline cpp_parse_result parse_cpp_for_enums(const std::string& /*file_data*/) {
	cpp_parse_result res = {};

    const char* path = "C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.22621.0\\um\\d3d11.h";
    //const char* path = "C:\\Repos\\enumbra\\examples\\imgui.h";
    //const char* path = "C:\\Repos\\enumbra\\examples\\enumbra_test.hpp";

    std::ifstream file(path, std::ios::in | std::ios::binary);
    std::string content{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };

    std::string stripped = strip_comments(content);

    std::unordered_map<std::string, std::string> macros;
    //macros.insert_or_assign("IMGUI_DISABLE_OBSOLETE_FUNCTIONS", "1");
    //macros.insert_or_assign("IMGUI_DISABLE_OBSOLETE_KEYIO", "1");

    try {
        auto namespace_list = extractNamespaces(stripped);

        const auto FindNamespaceForEnum = [&namespace_list](size_t enum_start_pos) -> std::string {
            std::string out;
            for (auto& n : namespace_list) {
                if (enum_start_pos > n.startPos && enum_start_pos < n.endPos) {
                    out = n.name;
                }
            }
            return out;
        };

        // Added as we parse
        std::unordered_map<std::string, std::string> global_namespace_enums;

        std::unordered_map<std::string, std::string> builtin_replace;
        builtin_replace.insert_or_assign("NK_FLAG", "1<<");

        auto result = parse_enums(stripped, macros);
        for (auto& parsed : result)
        {
            if (parsed.name.empty())
            {
                parsed.name = "[unnamed enum]";
            }

            auto ns = FindNamespaceForEnum(parsed.position);
            if (ns.empty()) {
                printf("%s\n", parsed.name.c_str());
            }
            else {
                printf("::%s::%s\n", ns.c_str(), parsed.name.c_str());
            }

            // Try literal parsing
            for (auto& val : parsed.values) {
                if (val.value.empty()) continue;

                val.value = strip_whitespace(val.value);
                try {
                    val.parsed_value = convertLiteral(val.value);
                    auto x = absl::StrFormat("%d", val.parsed_value.value());
                    //printf("    %s = %s\n", val.name.c_str(), x.c_str());
                }
                catch (...) {
                    //printf("[x] %s = %s (unable to parse)\n", val.name.c_str(), val.value.c_str());
                }
            }

            // Get fancy
            const auto FancyEval = [&]() {
                for (auto& val : parsed.values) {
                    if (val.parsed_value.has_value()) continue;
                    if (val.value.empty()) continue;

                    val.value = strip_whitespace(val.value);

                    // Common idiom is to calculate based on other enum values
                    // Sort them by largest to smallest, so we match larger strings first. An enum might have values like:
                    // enum class { Name = 0, NameBig = 1, NameBiggest = 2 }
                    // If we had just sorted in order, we would hit 'Name' first. 
                    std::vector<EnumValue> tempSorted = parsed.values;
                    std::sort(tempSorted.begin(), tempSorted.end(), [](EnumValue& e1, EnumValue& e2) { return e1.name.size() > e2.name.size(); });
                    for (auto& val2 : tempSorted) {
                        if ((val.name == val2.name) || !val2.parsed_value.has_value()) continue;

                        val.value = replace_substring(val.value, val2.name, absl::StrFormat("%d", val2.parsed_value.value()));
                    }

                    for (auto& preset : builtin_replace) {
                        if (!val.parsed_value.has_value()) {
                            val.value = replace_substring(val.value, preset.first, preset.second);
                        }
                    }

                    for (auto& gne : global_namespace_enums) {
                        if (!val.parsed_value.has_value()) {
                            val.value = replace_substring(val.value, gne.first, gne.second);
                        }
                    }

                    // Evaluate simple expressions
                    try {
                        val.parsed_value = evaluate(val.value);
                        //auto x = absl::StrFormat("%d", val.parsed_value.value());
                        //printf("    %s = %s\n", val.name.c_str(), x.c_str());
                    }
                    catch (...) {
                        //printf("[x] %s = %s (unable to evaluate)\n", val.name.c_str(), val.value.c_str());
                    }
                }
            };

            FancyEval();

            // Calculate implicit values
            for (int e = 0; e < 10; e++) { // TODO: We have to split these loops out to run once for each, rather than all at once
                for (size_t i = 0; i < parsed.values.size(); i++) {
                    auto& current = parsed.values[i];
                    if (!current.value.empty()) continue;

                    if (i == 0) {
                        current.parsed_value = 0;
                    }
                    else {
                        auto& previous = parsed.values[i - 1];
                        if (previous.parsed_value.has_value()) {
                            current.parsed_value = previous.parsed_value.value() + 1;
                        }
                    }
                }

                FancyEval();
            }

            for (auto& val : parsed.values) {
                if (val.parsed_value.has_value()) {
                    auto x = absl::StrFormat("%d", val.parsed_value.value());
                    printf("    %s = %s\n", val.name.c_str(), x.c_str());

                    global_namespace_enums.insert_or_assign(val.name, x);
                }
                else {
                    printf("[x] %s = %s (unable to evaluate)\n", val.name.c_str(), val.value.c_str());
                }
            }
        }
        
    }
    catch (const std::regex_error& e) {
        auto x = e.what();
        printf("%s", x);
    }
    catch (const std::invalid_argument& e) {
        auto x = e.what();
        printf("%s", x);
    }
    catch (const std::exception& e) {
        auto x = e.what();
        printf("%s", x);
    }

	return res;
}

