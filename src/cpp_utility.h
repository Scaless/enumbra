#pragma once

#include "enumbra.h"

#include <fmt/format.h>

struct Int128Format {
	int128 value;
	int64_t bits;
	bool bIsSigned;
};

std::string format_int128(Int128Format c);

// https://wgml.pl/blog/formatting-user-defined-types-fmt.html
template<>
struct fmt::formatter<Int128Format> {
	template<typename ParseContext>
	constexpr auto parse(ParseContext& ctx) {
		return ctx.begin();
	}

	auto format(Int128Format c, format_context& ctx) const {
		if (c.bIsSigned) {
			// If a value is the exact minimum of its storage type, we need to output it as an expression
			// because negative literals are actually a positive literal with a unary minus applied to them.
			// I'm not making this up. Fantastic language design.
			// https://stackoverflow.com/a/11270104
			const bool bIsMinValueForType =
				(c.bits == 8 && c.value == INT8_MIN) || (c.bits == 16 && c.value == INT16_MIN) ||
				(c.bits == 32 && c.value == INT32_MIN) || (c.bits == 64 && c.value == INT64_MIN);
			if (bIsMinValueForType) {
				return fmt::format_to(ctx.out(), "({0} - 1)", static_cast<int64_t>(c.value + 1));
			}

			return fmt::format_to(ctx.out(), "{0}", static_cast<int64_t>(c.value));
		}
		else {
			const uint64_t value = static_cast<uint64_t>(c.value);
			// TODO: Decide on a consistent behavior to format here, or make it customizable?
			if (value > 0xFF) {
				// Note: I find it disappointing that 0XFF {:#X} and 0xff {:#x} are fmt options,
				// but not 0xFF which I think is the most readable format :)
				return fmt::format_to(ctx.out(), "0x{0:X}", value);
			}
			else {
				return fmt::format_to(ctx.out(), "{0}", value);
			}
		}
	}
};

// Returns true if str can be used as a macro name.
// We limit to a-z, A-Z, 0-9, and underscore.
bool is_valid_macro_name(const std::string& str);

// Uppercase ascii string
std::string to_upper_ascii(const std::string& str);

// Log2 of unsigned int
constexpr int64_t log_2_unsigned(uint128 x) {
	if (x == 0) {
		return 0;
	}

	int targetlevel = 0;
	while (x >>= 1)
		++targetlevel;
	return targetlevel;
}

// Number of bits required to store an unsigned value
constexpr int64_t get_storage_bits_required(uint128 x) {
	return log_2_unsigned(x) + 1;
}

// Number of bits required to transmit a range of values
constexpr int64_t get_transmission_bits_required(uint128 x) {
	// Special case for a single value, which requires no bits to transmit
	if (x == 0) {
		return 0;
	}

	return log_2_unsigned(x) + 1;
}

// FNV-1a Hash - 64 bit
using fnv1a_64_hash = uint64_t;
constexpr fnv1a_64_hash fnv1a_64(const std::string_view str) noexcept {
	constexpr fnv1a_64_hash fnv1_basis_64 = 0xcbf29ce484222325ULL;
	constexpr fnv1a_64_hash fnv1_prime_64 = 0x00000100000001b3ULL;

	fnv1a_64_hash hash = fnv1_basis_64;

	for (const char c : str)
	{
		hash = (hash ^ static_cast<fnv1a_64_hash>(c)) * fnv1_prime_64;
	}

	return hash;
}
