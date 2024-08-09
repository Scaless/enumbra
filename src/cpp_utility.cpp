#include "cpp_utility.h"


std::string format_int128(Int128Format c) {
    if (c.bIsSigned) {
        // If a value is the exact minimum of its storage type, we need to output it as an expression
        // because negative literals are actually a positive literal with a unary minus applied to them.
        // I'm not making this up. Fantastic language design.
        // https://stackoverflow.com/a/11270104
        const bool bIsMinValueForType =
            (c.bits == 8 && c.value == INT8_MIN) || (c.bits == 16 && c.value == INT16_MIN) ||
            (c.bits == 32 && c.value == INT32_MIN) || (c.bits == 64 && c.value == INT64_MIN);
        if (bIsMinValueForType) {
            return fmt::format("({0} - 1)", static_cast<int64_t>(c.value + 1));
        }

        return fmt::format("{0}", static_cast<int64_t>(c.value));
    } else {
        const uint64_t value = static_cast<uint64_t>(c.value);
        // TODO: Decide on a consistent behavior to format here, or make it customizable?
        if (value > 0xFF) {
            // Note: I find it disappointing that 0XFF {:#X} and 0xff {:#x} are fmt options,
            // but not 0xFF which I think is the most readable format :)
            return fmt::format("0x{0:X}", value);
        } else {
            return fmt::format("{0}", value);
        }
    }
}

std::string to_upper(const std::string &str) {
    std::string copy = str;
    for (auto &c: copy) {
        c = std::toupper(c, std::locale("en_US.utf8"));
    }
    return copy;
}
