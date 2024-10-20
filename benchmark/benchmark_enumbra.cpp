#include "benchmark_shared.h"
#include "build/enumbra_test.hpp"

int main() { return 0; }

void test_from_string()
{
	const int char_count = 36;
	const char buffer[] = "abcdefghijklmnopqrstuvwxyz1234567890";
	for (int i = 0; i < char_count; i++)
	{
		auto result = enumbra::from_string<enums::errc>(buffer, i);
	}
}

void test_to_string()
{
	for (auto v : enumbra::values<enums::errc>())
	{
		auto result = enumbra::to_string(v);
	}
}

void test_to_integer()
{
	for (auto v : enumbra::values<enums::errc>())
	{
		auto result = enumbra::to_underlying(v);
	}
}

void test_from_integer()
{
	for (int i = 0; i < 1024; i++) {
		auto result = enumbra::from_integer<enums::errc>(i);
		if (result.has_value())
		{

		}
		auto result2 = enumbra::from_integer_unsafe<enums::errc>(i);
	}
}

void count()
{
	auto result = enumbra::count<enums::errc>();
}
