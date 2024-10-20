#include "benchmark_shared.h"
#include "magic_enum/magic_enum_all.hpp"

int main() { return 0; }

void test_from_string()
{
	const int char_count = 36;
	const char buffer[] = "abcdefghijklmnopqrstuvwxyz1234567890";
	for (int i = 0; i < char_count; i++)
	{
		auto result = magic_enum::enum_cast<errc>(std::string_view(buffer, i));
	}
}

void test_to_string()
{
	for (auto v : magic_enum::enum_values<errc>())
	{
		auto result = magic_enum::enum_name<errc>(v);
	}
}

void test_to_integer()
{
	for (auto v : magic_enum::enum_values<errc>())
	{
		auto result = magic_enum::enum_integer(v);
	}
}

void test_from_integer()
{
	for (int i = 0; i < 1024; i++) {
		auto result = magic_enum::enum_cast<errc>(i);
		if (result.has_value())
		{

		}
	}
}


void count()
{
	auto result = magic_enum::enum_count<errc>();
}
