#pragma once

#include "enumbra.h"
#include <array>
#include <string>
#include <string_view>
#include <fmt/format.h>

using namespace std::string_view_literals;
using namespace std::string_literals;

class cpp_generator {
public:
	const std::string& generate_cpp_output(const enumbra::enumbra_config& cfg, const enumbra::enum_meta_config& enum_meta);

private:
	std::string LF; // Line Feed as configured in enumbra_config
	std::string TAB; // Tabs as configured in enumbra_config
	std::string Output; // Final output

	template <typename... Args>
	void write(std::string_view fmt, Args&&... args) {
		fmt::format_to(std::back_inserter(Output), fmt, std::forward<Args>(args)...);
	}

	void write_linefeed(int count = 1) {
		for (int i = 0; i < count; i++)
			write("{}", LF);
	}

	void write_tab(int count = 1) {
		for (int i = 0; i < count; i++)
			write("{}", TAB);
	}

	template <typename... Args>
	void write_line(std::string_view fmt, Args&&... args) {
		write(fmt, args...);
		write_linefeed();
	}

	template <typename... Args>
	void write_line_tabbed(int tab_count, std::string_view fmt, Args&&... args) {
		for (int i = 0; i < tab_count; i++)
			write("{}", TAB);
		write(fmt, args...);
		write_linefeed();
	}
};
