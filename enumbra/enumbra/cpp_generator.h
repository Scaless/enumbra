#pragma once

#include "enumbra.h"
#include <string>

//#include <string_view>
//using namespace std::string_view_literals;
//constexpr std::wstring_view x = L""sv;

class cpp_generator {

public:
	std::string generate_cpp_output(const enumbra::enumbra_config& cfg, const enumbra::enum_definition& enum_def);

private:
	std::string LF; // Line Feed
};

