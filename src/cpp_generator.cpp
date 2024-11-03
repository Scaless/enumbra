#include "cpp_generator.h"
#include <set>
#include <vector>
#include <map>

using namespace enumbra;
using namespace enumbra::cpp;

uint64_t get_flags_enum_value(const FlagsEnumDefaultValueStyle &style, const enum_definition &definition) {
    switch (style) {
        case FlagsEnumDefaultValueStyle::Zero:
            return 0;
        case FlagsEnumDefaultValueStyle::Min: {
            auto m =
                std::min_element(definition.values.begin(), definition.values.end(),
                                 [](const enum_entry &lhs, const enum_entry &rhs) {
                                     return lhs.p_value < rhs.p_value;
                                 });
            if (m != definition.values.end()) {
                return static_cast<uint64_t>(m->p_value);
            } else {
                throw std::logic_error("get_flags_enum_value: FlagsEnumDefaultValueStyle::Min failed somehow.");
            }
        }
        case FlagsEnumDefaultValueStyle::Max: {
            auto m =
                std::max_element(definition.values.begin(), definition.values.end(),
                                 [](const enum_entry &lhs, const enum_entry &rhs) {
                                     return lhs.p_value < rhs.p_value;
                                 });
            if (m != definition.values.end()) {
                return static_cast<uint64_t>(m->p_value);
            } else {
                throw std::logic_error("get_flags_enum_value: FlagsEnumDefaultValueStyle::Max failed somehow.");
            }
        }
        case FlagsEnumDefaultValueStyle::UsedBitsSet: {
            uint64_t bits = 0;
            for (auto &v: definition.values) {
                bits |= static_cast<uint64_t>(v.p_value);
            }
            return bits;
        }
        case FlagsEnumDefaultValueStyle::First: {
            return static_cast<uint64_t>(definition.values.front().p_value);
        }
        case FlagsEnumDefaultValueStyle::Last: {
            return static_cast<uint64_t>(definition.values.back().p_value);
        }
        default:
            throw std::logic_error("get_flags_enum_value: Invalid FlagsEnumDefaultValueStyle");
    }
}

enum_entry get_value_enum_entry(const ValueEnumDefaultValueStyle &style, const enum_definition &definition) {
    switch (style) {
        case ValueEnumDefaultValueStyle::Min: {
            auto m =
                std::min_element(definition.values.begin(), definition.values.end(),
                                 [](const enum_entry &lhs, const enum_entry &rhs) {
                                     return lhs.p_value < rhs.p_value;
                                 });
            if (m != definition.values.end()) {
                return *m;
            } else {
                throw std::logic_error(
                    "get_value_enum_default_value_style_string: ValueEnumDefaultValueStyle::Min failed somehow.");
            }
        }
        case ValueEnumDefaultValueStyle::Max: {
            auto m =
                std::max_element(definition.values.begin(), definition.values.end(),
                                 [](const enum_entry &lhs, const enum_entry &rhs) {
                                     return lhs.p_value < rhs.p_value;
                                 });
            if (m != definition.values.end()) {
                return *m;
            } else {
                throw std::logic_error(
                    "get_value_enum_default_value_style_string: ValueEnumDefaultValueStyle::Max failed somehow.");
            }
        }
        case ValueEnumDefaultValueStyle::First:
            return definition.values.front();
        case ValueEnumDefaultValueStyle::Last:
            return definition.values.back();
        default:
            throw std::logic_error("value_enum_default_value_style: Invalid ValueEnumDefaultValueStyle");
    }
}

// Enum names must be unique across value and flag enums
bool enum_meta_has_unique_enum_names(const enumbra::enum_meta_config &enum_meta) {
    std::set<std::string> seen_names;
    for (auto &v: enum_meta.value_enum_definitions) {
        auto seen = seen_names.find(v.name);
        if (seen != seen_names.end()) {
            throw std::logic_error("enum_meta_has_unique_enum_names: Value-Enum name is not unique (name = " + *seen +
                                   ")");
        }
        seen_names.insert(v.name);
    }
    for (auto &v: enum_meta.flag_enum_definitions) {
        auto seen = seen_names.find(v.name);
        if (seen != seen_names.end()) {
            throw std::logic_error("enum_meta_has_unique_enum_names: Flags-Enum name is not unique (name = " + *seen +
                                   ")");
        }
        seen_names.insert(v.name);
    }
    return true;
}

bool is_value_set_contiguous(const std::set<int128> values) {
    int128 value = *values.begin();
    bool skip_first = true;
    for (auto &u: values) {
        if (skip_first) {
            skip_first = false;
            continue;
        }
        if (u != (value + 1)) {
            return false;
        }
        value++;
    }
    return true;
}

bool is_flags_set_contiguous(const std::set<int128> flags) {
    int64_t check_bit = get_storage_bits_required(*flags.begin());
    bool skip_first = true;
    for (auto &u: flags) {
        if (skip_first) {
            skip_first = false;
            continue;
        }
        if (u != (1LL << check_bit)) {
            return false;
        }
        check_bit++;
    }
    return true;
}

cpp_generator::cpp_generator(const enumbra_config &cfg, const enum_meta_config &enum_meta)
    : cpp_cfg(cfg.cpp_config), enum_meta(enum_meta) {}


void cpp_generator::build_contexts() {
    ctx = output_context();

    // Construct the full namespace for templates
    for (size_t i = 0; i < cpp_cfg.output_namespace.size(); i++) {
        ctx.enum_ns += cpp_cfg.output_namespace[i];
        if (i != (cpp_cfg.output_namespace.size() - 1)) {
            ctx.enum_ns += "::";
        }
    }
}

const std::string &cpp_generator::generate_cpp_output() {

    build_contexts();

    // Precondition checks
    // 1. Enum names must be unique
    enum_meta_has_unique_enum_names(enum_meta);

    emit_preamble();

    emit_include_guard_begin();

    emit_includes();

    emit_required_macros();

    emit_optional_macros();

    emit_templates();

    // Flags Enums Precondition Checks
    for (auto &e: enum_meta.flag_enum_definitions) {
        // 1. Names of contained values must be unique
        std::set<std::string> seen_names;
        for (auto &v: e.values) {
            auto res = seen_names.insert(v.name);
            if (!res.second) {
                throw std::logic_error(
                    fmt::format("Enum Value Name is not unique (Enum = {}, Name = {})", e.name, v.name));
            }
        }
        seen_names.clear();

        // 2. Enum must have at least 1 value
        if (e.values.empty()) {
            throw std::logic_error(fmt::format("Enum does not contain any values (Enum = {})", e.name));
        }

        // 3. Determine if all values are unique, or if some enum value names overlap.
        // TODO: Properly define rules for value aliases
        std::set<int128> unique_values;
        for (auto &v: e.values) {
            unique_values.insert(v.p_value);
        }
        if (e.values.size() != unique_values.size()) {
            throw std::logic_error(
                fmt::format("Values in enum do not have unique values (Enum = {})", e.name));
        }
    }

    // Build value enum contexts
    for (auto &e: enum_meta.value_enum_definitions) {
        clear_store();

        // Value Enum Precondition checks

        // 1. Names of contained values must be unique
        std::set<std::string> seen_names;
        for (auto &v: e.values) {
            auto seen = seen_names.find(v.name);
            if (seen != seen_names.end()) {
                throw std::runtime_error("Enum Value Name is not unique (name = " + *seen + ")");
            }
            seen_names.insert(v.name);
        }
        seen_names.clear();

        // 2. Enum must have at least 1 value
        if (e.values.empty()) {
            throw std::runtime_error(fmt::format("Enum does not contain any values (name = {})", e.name));
        }

        // 3. Enum values must be unique
        std::set<int128> unique_values;
        for (auto &v: e.values) { unique_values.insert(v.p_value); }
        if (unique_values.size() != e.values.size()) {
            throw std::runtime_error(fmt::format("Enum contains duplicate values (name = {})", e.name));
        }

        // Build Context
        value_enum_context new_context;

        new_context.enum_name = e.name;
        new_context.values = e.values;

        new_context.default_entry = get_value_enum_entry(enum_meta.value_enum_default_value_style, e);
        new_context.min_entry = get_value_enum_entry(ValueEnumDefaultValueStyle::Min, e);
        new_context.max_entry = get_value_enum_entry(ValueEnumDefaultValueStyle::Max, e);
        new_context.entry_count = e.values.size();
        new_context.size_type_str = cpp_cfg.get_size_type_from_index(e.size_type_index).type_name;
        new_context.is_size_type_signed = cpp_cfg.get_size_type_from_index(e.size_type_index).is_signed;
        new_context.size_type_bits = cpp_cfg.get_size_type_from_index(e.size_type_index).bits;

        new_context.unique_entry_count = static_cast<int64_t>(unique_values.size());
        new_context.is_range_contiguous = is_value_set_contiguous(unique_values);

        const int64_t max_abs_representable_signed =
            std::max(
                std::abs(static_cast<int64_t>(new_context.min_entry.p_value) - 1),
                static_cast<int64_t>(new_context.max_entry.p_value)
            );
        const uint64_t max_abs_representable =
            new_context.is_size_type_signed
            ? max_abs_representable_signed
            : static_cast<uint64_t>(new_context.max_entry.p_value);

        new_context.bits_required_storage = get_storage_bits_required(max_abs_representable);
        new_context.bits_required_transmission = get_transmission_bits_required(
            new_context.max_entry.p_value - new_context.min_entry.p_value);

        // Because of the way signed integers map to bit fields, a bit field may require an additional
        // bit of storage to accommodate the sign bit even if it is unused. For example, given the following enum:
        //   enum class ESignedValueBits : int8_t { A = 0, B = 1, C = 2, D = 3 }
        // To properly store and assign to this enum, we need 3 bits:
        //   int8_t Value : 1; // maps to the range -1 - 0, unexpected!
        //   int8_t Value : 2; // maps to the range -2 - 1, still not big enough
        //   int8_t Value : 3; // maps to the range -4 - 3, big enough, but we're wasting space
        // For this reason, when utilizing packed enums it is recommended to always prefer an unsigned underlying
        // type unless your enum actually contains negative values.
        if (new_context.is_size_type_signed && (new_context.max_entry.p_value > 0)) {
            uint64_t signed_range_max = 0;
            for (int64_t i = 0; i < new_context.bits_required_storage - 1; i++) {
                signed_range_max |= 1ULL << i;
            }
            if (static_cast<uint64_t>(new_context.max_entry.p_value) > signed_range_max) {
                new_context.bits_required_storage += 1;
            }
        }

        // Determine sentinel values
        {

            const auto GetAvailableSentinel = [&]() -> std::optional<int128> {
                // TODO: Benchmark for best sentinel to use, or if it's even worth using one at all.

                // 0 is probably the most efficient sentinel to use if it's not already a valid value.
                if(unique_values.find(0) == unique_values.end()) {
                    return 0;
                }

                // If signed, try -1
                if(new_context.is_size_type_signed) {
                    if (unique_values.find(-1) == unique_values.end()) {
                        return -1;
                    }
                }

                // If unsigned, try all bits set
                if(!new_context.is_size_type_signed) {
                    int128 AllBitsMask = 0;
                    for(int i = 0; i < new_context.size_type_bits; i++) {
                        AllBitsMask |= (1ULL << i);
                    }
                    if (unique_values.find(AllBitsMask) == unique_values.end()) {
                        return AllBitsMask;
                    }
                }

                // Just use a separate bool.
                return std::nullopt;
            };

            new_context.invalid_sentinel = GetAvailableSentinel();
        }

        // Generate string lookup tables
        auto generate_string_lookup_tables = [&]() -> string_lookup_tables {
            string_lookup_tables output;

            // Put strings into buckets by size
            std::map<size_t, std::vector<enum_entry>> buckets_by_size;
            for (auto &ed: e.values) {
                buckets_by_size[ed.name.length()].push_back(ed);
            }

            size_t offset_str = 0;
            size_t offset_enum = 0;
            for (auto &bucket: buckets_by_size) {
                const size_t count = bucket.second.size();

                std::vector<std::string> names;
                for (auto &entry: bucket.second) {
                    output.entries.push_back(entry);
                    names.push_back(entry.name);
                }

                output.tables.emplace_back(string_lookup_table{offset_str, offset_enum, count, bucket.first, names});

                // TODO: Handle Padding
                offset_str += (bucket.second.front().name.length() * count) + (1 * count);
                offset_enum += bucket.second.size();
            }

            return output;
        };
        new_context.string_tables = generate_string_lookup_tables();

        new_context.is_one_string_table = std::equal(
            e.values.cbegin(), e.values.cend(),
            new_context.string_tables.entries.cbegin(), new_context.string_tables.entries.cend()
        );

        ctx.value_enums.emplace_back(new_context);
    }

    // VALUE ENUM DEFINITIONS
    for (auto &e: ctx.value_enums) {
        clear_store();

        // Build fmt args
        push("enum_ns", ctx.enum_ns);
        push("enum_ns_size", std::to_string(ctx.enum_ns.size()));
        push("enum_name", e.enum_name);
        push("enum_name_size", std::to_string(e.enum_name.size()));
        push("enum_detail_ns", fmt::format("::{}::detail::{}", ctx.enum_ns, e.enum_name));
        push("enum_name_fq", fmt::format("::{}::{}", ctx.enum_ns, e.enum_name));
        push("enum_name_fq_size", std::to_string(fmt::format("::{}::{}", ctx.enum_ns, e.enum_name).size()));
        push("enum_name_fq_no_global", fmt::format("{}::{}", ctx.enum_ns, e.enum_name));
        push("enum_name_fq_no_global_size", std::to_string(fmt::format("{}::{}", ctx.enum_ns, e.enum_name).size()));
        push("size_type", e.size_type_str);
        push("entry_count", std::to_string(e.entry_count));
        push("max_v", format_int128({e.max_entry.p_value, e.size_type_bits, e.is_size_type_signed}));
        push("min_v", format_int128({e.min_entry.p_value, e.size_type_bits, e.is_size_type_signed}));

        emit_ve_definition(e);

        // Helper specializations
        wvl("template<> struct enumbra::detail::base_helper<{enum_name_fq}> : enumbra::detail::type_info<true, true, false> {{ }};");

        const std::string value_helper_str =
            fmt::format(
                "template<> struct enumbra::detail::enum_helper<::{0}::{1}> : enumbra::detail::enum_info<{2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}, {10}, {11}> {{ }};",
                ctx.enum_ns,
                e.enum_name,
                e.size_type_str,
                Int128Format{e.min_entry.p_value, e.size_type_bits, e.is_size_type_signed},
                Int128Format{e.max_entry.p_value, e.size_type_bits, e.is_size_type_signed},
                Int128Format{e.default_entry.p_value, e.size_type_bits, e.is_size_type_signed},
                e.unique_entry_count,
                e.is_range_contiguous ? "true" : "false",
                e.bits_required_storage,
                e.bits_required_transmission,
                e.invalid_sentinel.has_value() ? "true" : "false",
                Int128Format{e.invalid_sentinel.value_or(0), e.size_type_bits, e.is_size_type_signed}
            );
        wlu(value_helper_str);
        wlf();

        emit_ve_detail(e);
        emit_ve_func_values(e);
        emit_ve_func_from_integer(e);
        emit_ve_func_is_valid(e);
        emit_ve_func_enum_name(e);

        emit_ve_func_to_string(e);
        emit_ve_func_from_string_with_size(e);
        emit_ve_func_from_string_cstr(e);

        wlf();
    }
    wlf();

    

    // Flags ENUM DEFINITIONS
    for (auto &e: enum_meta.flag_enum_definitions) {
        clear_store();

        // Get references and metadata for relevant enum values that we will need
        const uint64_t min_value = 0; // The minimum for a flags entry is always 0 - no bits set
        uint64_t max_value = 0;
        for (auto &v: e.values) {
            if (v.p_value < 0) {
                throw std::logic_error(fmt::format("Flags-Enum value is less than 0. Flags-Enum values are required to be unsigned. (Enum = {})", e.name));
            }
            max_value |= static_cast<uint64_t>(v.p_value);
        }

        std::set<int128> unique_values;
        for (auto &v: e.values) {
            unique_values.insert(v.p_value);
        }
        const size_t unique_entry_count = unique_values.size();

        const uint64_t default_value = get_flags_enum_value(enum_meta.flags_enum_default_value_style, e);

        // const size_t entry_count = e.values.size();
        const size_t bits_required_storage = get_storage_bits_required(max_value);
        const size_t bits_required_transmission = bits_required_storage;
        const std::string size_type = cpp_cfg.get_size_type_from_index(e.size_type_index).type_name;
        const bool is_size_type_signed = cpp_cfg.get_size_type_from_index(e.size_type_index).is_signed;
        const int64_t type_bits = cpp_cfg.get_size_type_from_index(e.size_type_index).bits;
        if (is_size_type_signed) {
            throw std::logic_error("Size type for flags enum is signed. enumbra requires that flags use an unsigned type.");
        }

        // Determine if range is contiguous
        // Enables some minor optimizations for range-checking values if true
        // TODO: Enforce if flag is set
        bool is_contiguous = is_flags_set_contiguous(unique_values);

        const std::string enum_name_fq = fmt::format("::{}::{}", ctx.enum_ns, e.name);

        push("enum_ns", ctx.enum_ns);
        push("enum_name", e.name);
        push("enum_detail_ns", fmt::format("::{}::detail::{}", ctx.enum_ns, e.name));
        push("enum_name_fq", enum_name_fq);
        push("size_type", size_type);
        push("unique_entry_count", std::to_string(unique_entry_count));
        push("max_value", fmt::format("{0:#x}", max_value));

        // START NAMESPACE
        for (const auto &ns: cpp_cfg.output_namespace) {
            wl("namespace {} {{", ns);
        }

        // Definition
        {
            wvl("enum class {enum_name} : {size_type} {{");
            for (const auto &v: e.values) {
                wl("{} = {},", v.name, Int128Format{v.p_value, type_bits, is_size_type_signed});
            }
            wlu("};");
        }
        wlf();

        wvl("namespace detail::{enum_name} {{");
        wvl("constexpr {enum_name_fq} flags_arr[{unique_entry_count}] =");
        wlu("{");
        for (const auto &v: e.values) {
            wl("{0}::{1},", enum_name_fq, v.name);
        }
        wlu("};");
        wlu("}");
        wlf();

        // END NAMESPACE
        for (auto ns = cpp_cfg.output_namespace.rbegin(); ns != cpp_cfg.output_namespace.rend(); ++ns) {
            wl("}} // namespace {}", *ns);
        }
        wlf();

        wvl("namespace enumbra {{");

        wlu("template<>");
        wvl("constexpr auto& flags<{enum_name_fq}>() noexcept");
        wlu("{");
        wvl("return {enum_detail_ns}::flags_arr;");
        wlu("}");
        wlf();

        wlu("template<>");
        wvl("constexpr bool is_valid<{enum_name_fq}>({enum_name_fq} e) noexcept {{ ");
        wvl("return (static_cast<{size_type}>(e) | static_cast<{size_type}>({max_value})) == static_cast<{size_type}>({max_value});");
        wlu("}");
        wlf();

        //// Functions
        
        wvl("template<> constexpr void clear({enum_name_fq}& value) noexcept {{ value = static_cast<{enum_name_fq}>(0); }}");
        wvl("template<> constexpr bool test({enum_name_fq} value, {enum_name_fq} flags) noexcept {{ return (static_cast<{size_type}>(flags) & static_cast<{size_type}>(value)) == static_cast<{size_type}>(flags); }}");
        wvl("template<> constexpr void set({enum_name_fq}& value, {enum_name_fq} flags) noexcept {{ value = static_cast<{enum_name_fq}>(static_cast<{size_type}>(value) | static_cast<{size_type}>(flags)); }}");
        wvl("template<> constexpr void unset({enum_name_fq}& value, {enum_name_fq} flags) noexcept {{ value = static_cast<{enum_name_fq}>(static_cast<{size_type}>(value) & (~static_cast<{size_type}>(flags))); }}");
        wvl("template<> constexpr void toggle({enum_name_fq}& value, {enum_name_fq} flags) noexcept {{ value = static_cast<{enum_name_fq}>(static_cast<{size_type}>(value) ^ static_cast<{size_type}>(flags)); }}");
        
        wvl("template<> constexpr bool has_all({enum_name_fq} value) noexcept {{ return (static_cast<{size_type}>(value) & static_cast<{size_type}>({max_value})) == static_cast<{size_type}>({max_value}); }}");
        wvl("template<> constexpr bool has_any({enum_name_fq} value) noexcept {{ return (static_cast<{size_type}>(value) & static_cast<{size_type}>({max_value})) > 0; }}");
        wvl("template<> constexpr bool has_none({enum_name_fq} value) noexcept {{ return (static_cast<{size_type}>(value) & static_cast<{size_type}>({max_value})) == 0; }}");
        wvl("template<> constexpr bool has_single({enum_name_fq} value) noexcept {{ {size_type} n = static_cast<{size_type}>(static_cast<{size_type}>(value) & {max_value}); return n && !(n & (n - 1)); }}");
        wlf();

        wlu("} // namespace enumbra");
        wlf();

        // Helper specializations
        wl("template<> struct enumbra::detail::base_helper<{0}> : enumbra::detail::type_info<true, false, true> {{ }};", enum_name_fq);

        wl("template<> struct enumbra::detail::enum_helper<{0}> : enumbra::detail::enum_info<{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}, {10}> {{ }};",
            enum_name_fq,
            size_type,
            Int128Format{ min_value, type_bits, is_size_type_signed },
            Int128Format{ max_value, type_bits, is_size_type_signed },
            Int128Format{ default_value, type_bits, is_size_type_signed },
            unique_entry_count,
            is_contiguous ? "true" : "false",
            bits_required_storage,
            bits_required_transmission,
            "false", // TODO: We could use an unused flag bit to signal invalid
            0
        );
        
        wlf();

        // Operator Overloads need to be outside of enumbra::
        for (const auto& ns : cpp_cfg.output_namespace) {
            wl("namespace {} {{", ns);
        }
        wvl("constexpr {enum_name_fq} operator~(const {enum_name_fq} a) noexcept {{ return static_cast<{enum_name_fq}>(~static_cast<{size_type}>(a)); }}");
        wvl("constexpr {enum_name_fq} operator|(const {enum_name_fq} a, const {enum_name_fq} b) noexcept {{ return static_cast<{enum_name_fq}>(static_cast<{size_type}>(a) | static_cast<{size_type}>(b)); }}");
        wvl("constexpr {enum_name_fq} operator&(const {enum_name_fq} a, const {enum_name_fq} b) noexcept {{ return static_cast<{enum_name_fq}>(static_cast<{size_type}>(a) & static_cast<{size_type}>(b)); }}");
        wvl("constexpr {enum_name_fq} operator^(const {enum_name_fq} a, const {enum_name_fq} b) noexcept {{ return static_cast<{enum_name_fq}>(static_cast<{size_type}>(a) ^ static_cast<{size_type}>(b)); }}");
        wvl("constexpr {enum_name_fq}& operator|=({enum_name_fq}& a, const {enum_name_fq} b) noexcept {{ return a = a | b; }}");
        wvl("constexpr {enum_name_fq}& operator&=({enum_name_fq}& a, const {enum_name_fq} b) noexcept {{ return a = a & b; }}");
        wvl("constexpr {enum_name_fq}& operator^=({enum_name_fq}& a, const {enum_name_fq} b) noexcept {{ return a = a ^ b; }}");
        for (auto ns = cpp_cfg.output_namespace.rbegin(); ns != cpp_cfg.output_namespace.rend(); ++ns) {
            wl("}} // namespace {}", *ns);
        }

        wlf();

        // Calc max string size
        size_t max_to_string_size = 0;
        for (const auto& v : e.values) {
            max_to_string_size += v.name.size();
        }
        max_to_string_size += (e.values.size() - 1); // Add room for separators

        // We align to 16 bytes, taking into account the int32 size field
        max_to_string_size += 4;
        max_to_string_size += (16 - (max_to_string_size % 16)) % 16;
        max_to_string_size -= 4;
        push("max_to_string_size", std::to_string(max_to_string_size));

        // Start ns
        wlu("namespace enumbra {");
        wlf();

        wvl("constexpr ::enumbra::stack_string<{max_to_string_size}> to_string(const {enum_name_fq} v) noexcept {{");
        wvl("::enumbra::stack_string<{max_to_string_size}> output;");
        bool first = true;
        for (const auto& v : e.values) {
            wl("if (static_cast<{0}>(v & {1}::{2}) > 0) {{", size_type, enum_name_fq, v.name);
            if (!first) {
                wlu("if (!output.empty()) { output.append('|'); }");
            }
            first = false;
            wl("output.append<{0}>(\"{1}\");", v.name.size(), v.name);
            wlu("}");
        }
        wlu("return output;");
        wlu("}");

        wlf();

        std::map<size_t, std::set<std::string>> names_sorted_by_length;
        for (const auto& v : e.values) {
            names_sorted_by_length[v.name.size()].insert(v.name);
        }

        wlu("template<>");
        wvl("constexpr ::enumbra::optional_value<{enum_name_fq}> from_string<{enum_name_fq}>(const char* str, int len) noexcept {{");
        wlu("if (len < 0) { return {}; } // Invalid size");
        wlu("const char* start = str;");
        wlu("const char* end = start;");
        wvl("{enum_name_fq} output = {{}};");
        wlu("for (int i = 0; i < len; ++i) {");
        wlu("if (str[i] == '\\0') { return {}; } // Invalid: null in string");
        wlu("end++;");
        wlu("if ((i == (len - 1)) || (*end == '|')) {");
        wlu("const auto check_len = end - start;");

        bool firstSize = true;
        for (auto& [length, names] : names_sorted_by_length) {
            wl("{} (check_len == {}) {{", firstSize ? "if" : "else if", length);
            firstSize = false;

            bool firstName = true;
            for (auto& name : names) {
                const char* ifelsename = firstName ? "if" : "else if";
                wl("{} (::enumbra::detail::streq_fixed_size<{}>(start, \"{}\")) {{ output |= {}::{}; }}", ifelsename, length, name, enum_name_fq, name);
                firstName = false;
            }
            wlu("else { return {}; }");
            wlu("}");
        }

        wlu("else { return {}; }");
        wlu("start = end + 1;");
        wlu("}");
        wlu("}");
        wvl("return ::enumbra::optional_value<{enum_name_fq}>(output);");
        wlu("}");

        wlf();

        wlu("template<>");
        wvl("constexpr ::enumbra::optional_value<{enum_name_fq}> from_string<{enum_name_fq}>(const char* str) noexcept {{");
        wlu("    const int len = ::enumbra::detail::strlen(str);");
        wvl("    return ::enumbra::from_string<{enum_name_fq}>(str, len);");
        wlu("}");

        // End ns
        wlu("} // namespace enumbra");
        wlf();
    }

    emit_include_guard_end();
    
    // Hash the whole file to generate a unique include guard
    const fnv1a_64_hash hash = fnv1a_64(output);
    std::string hash_str = fmt::format("ENUMBRA_{0:X}_H", hash);
    for (const size_t location : ctx.header_guard_positions) {
        output.replace(output.begin() + location, output.begin() + location + hash_str.size(), hash_str);
    }

    return output;
}

void cpp_generator::emit_required_macros() {
    // Increment this if macros below are modified.
    const int enumbra_required_macros_version = 9;
    const std::string macro_strings = R"(
#if !defined(ENUMBRA_REQUIRED_MACROS_VERSION)
#define ENUMBRA_REQUIRED_MACROS_VERSION {0}

// Find out what language version we're using
// 2024-07-04:MSVC Doesn't officially support C++23 yet
#if (__cplusplus >= 202302L)
#define ENUMBRA_CPP_VERSION 23
#elif ((defined(_MSVC_LANG) && _MSVC_LANG >= 202002L)) || (__cplusplus >= 202002L)
#define ENUMBRA_CPP_VERSION 20
#elif ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)) || (__cplusplus >= 201703L)
#define ENUMBRA_CPP_VERSION 17
#else
#error Headers generated by enumbra require a compiler that supports C++17 or higher.
#endif

#if defined(__clang__)
#define ENUMBRA_COMPILER_CLANG
#elif defined(__GNUG__)
#define ENUMBRA_COMPILER_GCC
#elif defined(_MSC_VER)
#define ENUMBRA_COMPILER_MSVC
#else
#define ENUMBRA_COMPILER_UNKNOWN
#endif

#else // check existing version supported
#if (ENUMBRA_REQUIRED_MACROS_VERSION + 0) == 0
#error ENUMBRA_REQUIRED_MACROS_VERSION has been defined without a proper version number. Check your build system.
#elif (ENUMBRA_REQUIRED_MACROS_VERSION + 0) < {0}
#error An included header was generated using a newer version of enumbra. Regenerate your headers using the same version.
#elif (ENUMBRA_REQUIRED_MACROS_VERSION + 0) > {0}
#error An included header was generated using an older version of enumbra. Regenerate your headers using the same version.
#endif // end check existing version supported
#endif // ENUMBRA_REQUIRED_MACROS_VERSION)";

    write(macro_strings, enumbra_required_macros_version);
    wlf();
}

void cpp_generator::emit_preamble() {
    wlu("// THIS FILE WAS GENERATED BY A TOOL: https://github.com/Scaless/enumbra");
    wlu("// It is highly recommended that you not make manual edits to this file,");
    wlu("// as they will be overwritten when the file is re-generated.");
    if (cpp_cfg.time_generated_in_header) {
        std::time_t time_point = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        char time_buf[255];
        ctime_s(time_buf, 255, &time_point);
        wl("// Generated by enumbra v{} on {}", kEnumbraVersion, time_buf);
    } else {
        wl("// Generated by enumbra v{}", kEnumbraVersion);
    }
    wlf();

    // Custom preamble
    for (const auto &line: cpp_cfg.preamble_text) {
        wl(line);
    }
    if (!cpp_cfg.preamble_text.empty()) {
        wlf();
    }
}

void cpp_generator::emit_include_guard_begin() {
    ctx.header_guard_positions.push_back(output.size() + 8);
    wlu("#ifndef ENUMBRA_0000000000000000_H");
    ctx.header_guard_positions.push_back(output.size() + 8);
    wlu("#define ENUMBRA_0000000000000000_H");
    wlf();
}

void cpp_generator::emit_include_guard_end()
{
    ctx.header_guard_positions.push_back(output.size() + 10);
    wlu("#endif // ENUMBRA_0000000000000000_H");
}

void cpp_generator::emit_includes() {
    for (const auto &inc: cpp_cfg.additional_includes) {
        wl("#include {}", inc);
    }
}

void cpp_generator::emit_optional_macros() {
    if (cpp_cfg.enumbra_bitfield_macros) {
        // Increment this if macros below are modified.
        const int enumbra_optional_macros_version = 10;
        std::string macro_strings = R"(
#ifndef ENUMBRA_NO_OPTIONAL_MACROS
#if !defined(ENUMBRA_OPTIONAL_MACROS_VERSION)
#define ENUMBRA_OPTIONAL_MACROS_VERSION {0}

// Bitfield convenience functions
#define ENUMBRA_CLEAR(Field) do {{ decltype(Field) _field_ = Field; ::enumbra::clear(_field_); (Field) = _field_; }} while (0)
#define ENUMBRA_SET(Field, Value) do {{ decltype(Field) _field_ = Field; ::enumbra::set(_field_, Value); (Field) = _field_; }} while (0)
#define ENUMBRA_UNSET(Field, Value) do {{ decltype(Field) _field_ = Field; ::enumbra::unset(_field_, Value); (Field) = _field_; }} while (0)
#define ENUMBRA_TOGGLE(Field, Value) do {{ decltype(Field) _field_ = Field; ::enumbra::toggle(_field_, Value); (Field) = _field_; }} while (0)

// Bit field storage helper
#define ENUMBRA_PACK_UNINITIALIZED(Enum, Name) Enum Name : ::enumbra::bits_required_storage<Enum>()
#define ENUMBRA_INIT(Name, InitValue) Name(::enumbra::default_value<decltype(Name)>())
#define ENUMBRA_INIT_DEFAULT(Name) Name(::enumbra::default_value<decltype(Name)>())

// Iterate flags in a switch
#define ENUMBRA_FLAGS_SWITCH_BEGIN(var) do {{ for (const auto flag : ::enumbra::flags<decltype(var)>()) {{ if (::enumbra::has_any(var & flag)) {{ switch (var & flag)
#define ENUMBRA_FLAGS_SWITCH_END }} }} }} while (0)

#if ENUMBRA_CPP_VERSION >= 20
// Bit field storage helper with type-checked member initialization
#define ENUMBRA_PACK_INIT(Enum, Name, InitValue) Enum Name : ::enumbra::bits_required_storage<Enum>() {{ InitValue }}
// Bit field storage helper with default value initialization
#define ENUMBRA_PACK_INIT_DEFAULT(Enum, Name) Enum Name : ::enumbra::bits_required_storage<Enum>() {{ ::enumbra::default_value<Enum>() }}
#endif

#else // check existing version supported
#if (ENUMBRA_OPTIONAL_MACROS_VERSION + 0) == 0
#error ENUMBRA_OPTIONAL_MACROS_VERSION has been defined without a proper version number. Check your build system.
#elif (ENUMBRA_OPTIONAL_MACROS_VERSION + 0) < {0}
#error An included header was generated using a newer version of enumbra. Regenerate your headers using the same version.
#elif (ENUMBRA_OPTIONAL_MACROS_VERSION + 0) > {0}
#error An included header was generated using an older version of enumbra. Regenerate your headers using the same version.
#endif // end check existing version supported
#endif // ENUMBRA_OPTIONAL_MACROS_VERSION
#endif)";

        write(macro_strings, enumbra_optional_macros_version);
        wlf();
    }
}

void cpp_generator::emit_templates() {
    // Increment this if templates below are modified.
    const int enumbra_templates_version = 31;
    const std::string str_templates = R"(
#if !defined(ENUMBRA_BASE_TEMPLATES_VERSION)
#define ENUMBRA_BASE_TEMPLATES_VERSION {0}
namespace enumbra {{
    namespace detail {{
        // Re-Implementation of std:: features to avoid including std headers
        template<bool B, class T, class F>
        struct conditional {{ using type = T; }};
        template<class T, class F>
        struct conditional<false, T, F> {{ using type = F; }};
        template<bool B, class T, class F>
        using conditional_t = typename conditional<B, T, F>::type;

 #if defined(__cpp_lib_is_constant_evaluated)
        // Supported on clang/gcc/MSVC in C++17, even though it's only in the C++20 standard. 
        constexpr bool is_constant_evaluated() noexcept {{ return __builtin_is_constant_evaluated(); }}
#else
        constexpr bool is_constant_evaluated() noexcept {{ return false; }}
#endif

        // Type info
        template<bool is_enumbra, bool is_value_enum, bool is_flags_enum>
        struct type_info {{
            static constexpr bool enumbra_type = is_enumbra;
            static constexpr bool enumbra_value_enum = is_value_enum;
            static constexpr bool enumbra_flags_enum = is_flags_enum;
        }};

        // Enum info
        template<
            typename underlying_type,
            underlying_type min_v,
            underlying_type max_v,
            underlying_type default_v,
            int count_v,
            bool is_contiguous_v,
            int bits_required_storage_v,
            int bits_required_transmission_v,
            bool has_invalid_sentinel_v,
            underlying_type invalid_sentinel_v
        >
        struct enum_info {{
            using underlying_t = underlying_type;
            static constexpr underlying_type min = min_v;
            static constexpr underlying_type max = max_v;
            static constexpr underlying_type default_value = default_v;
            static constexpr int count = count_v;
            static constexpr bool is_contiguous = is_contiguous_v;
            static constexpr int bits_required_storage = bits_required_storage_v;
            static constexpr int bits_required_transmission = bits_required_transmission_v;
            static constexpr bool has_invalid_sentinel = has_invalid_sentinel_v;
            static constexpr underlying_type invalid_sentinel = invalid_sentinel_v;
        }};

        // Default template for non-enumbra types
        template<class T>
        struct base_helper : type_info<false, false, false> {{ }};
        template<class T>
        struct enum_helper;

        // Compare strings with sizes only known at runtime
        constexpr bool streq_known_size(const char* a, const char* b, int len) noexcept {{
            for(int i = 0; i < len; ++i) {{ if(a[i] != b[i]) {{ return false; }} }}
            return true;
        }}
        // Compare strings with sizes known at compile time
        template<int length>
        constexpr bool streq_fixed_size(const char* a, const char* b) noexcept {{
            static_assert(length > 0);
            for(int i = 0; i < length; ++i) {{ if(a[i] != b[i]) {{ return false; }} }}
            return true;
        }}
        // C-style string length
        constexpr int strlen(const char* a) noexcept {{
            if (a == nullptr) {{ return 0; }}
            int count = 0;
            while (a[count] != 0) {{ count++; }}
            return count;
        }}
    }} // end namespace enumbra::detail

    template<class T>
    constexpr bool is_enumbra_enum = detail::base_helper<T>::enumbra_type;
    template<class T>
    constexpr bool is_enumbra_value_enum = detail::base_helper<T>::enumbra_value_enum;
    template<class T>
    constexpr bool is_enumbra_flags_enum = detail::base_helper<T>::enumbra_flags_enum;

    template<class T>
    constexpr T min() {{
        static_assert(is_enumbra_enum<T>, "T is not an enumbra enum");
        return static_cast<T>(detail::enum_helper<T>::min);
    }}

    template<class T>
    constexpr T max() {{
        static_assert(is_enumbra_enum<T>, "T is not an enumbra enum");
        return static_cast<T>(detail::enum_helper<T>::max);
    }}

    template<class T>
    constexpr T default_value() {{
        static_assert(is_enumbra_enum<T>, "T is not an enumbra enum");
        return static_cast<T>(detail::enum_helper<T>::default_value);
    }}

    template<class T>
    constexpr int count() {{
        static_assert(is_enumbra_enum<T>, "T is not an enumbra enum");
        return detail::enum_helper<T>::count;
    }}

    template<class T>
    constexpr bool is_contiguous() {{
        static_assert(is_enumbra_enum<T>, "T is not an enumbra enum");
        return detail::enum_helper<T>::is_contiguous;
    }}

    template<class T>
    constexpr int bits_required_storage() {{
        static_assert(is_enumbra_enum<T>, "T is not an enumbra enum");
        return detail::enum_helper<T>::bits_required_storage;
    }}
 
    template<class T>
    constexpr int bits_required_transmission() {{
        static_assert(is_enumbra_enum<T>, "T is not an enumbra enum");
        return detail::enum_helper<T>::bits_required_transmission;
    }}

    template<class T, class underlying_type>
    constexpr T from_integer_unsafe(underlying_type e) noexcept {{
        static_assert(is_enumbra_enum<T>, "T is not an enumbra enum");
        return static_cast<T>(e);
    }}

    template<class T>
    constexpr auto to_underlying(T e) noexcept {{
        static_assert(is_enumbra_enum<T>, "T is not an enumbra enum");
        return static_cast<typename detail::enum_helper<T>::underlying_t>(e);
    }}

    namespace detail {{
        template<class T>
        struct optional_result_base_bool 
        {{
        private:
            using bool_type = 
                detail::conditional_t<sizeof(T) == 1, char,
                detail::conditional_t<sizeof(T) == 2, short,
                detail::conditional_t<sizeof(T) == 4, int,
                detail::conditional_t<sizeof(T) == 8, long long,
                void /* invalid size */>>>>;
        protected:
            bool_type success = 0;
        }};
        
        struct optional_result_base_inplace {{ }};
    }}

    template<class T, bool use_invalid_sentinel = detail::enum_helper<T>::has_invalid_sentinel>
    struct optional_value : detail::conditional_t<use_invalid_sentinel, detail::optional_result_base_inplace, detail::optional_result_base_bool<T>>
    {{
    private:
        T v = static_cast<T>(detail::enum_helper<T>::invalid_sentinel);
    public:
        constexpr optional_value() : v(static_cast<T>(detail::enum_helper<T>::invalid_sentinel)) {{ }}

        constexpr explicit optional_value(T value) : v(value) {{
            if constexpr(!use_invalid_sentinel) {{
                this->success = 1;
            }}
        }}

        constexpr explicit operator bool() const noexcept {{
            if constexpr (use_invalid_sentinel) {{
                return v != static_cast<T>(detail::enum_helper<T>::invalid_sentinel);
            }} else {{
                return this->success > 0;
            }}
        }}

        constexpr bool has_value() const {{ return operator bool(); }}
        constexpr T value() const {{ return v; }}
        constexpr T value_or(T default_value) const {{ return operator bool() ? v : default_value; }}
    }};

    struct string_view {{
        using size_type = detail::conditional_t<sizeof(void*) == 4, int, long long>;

        const char* str = nullptr;
        size_type size = 0;

        constexpr bool empty() const {{ return size == 0; }}
    }};

	template<int buf_size>
	struct stack_string {{
		static_assert(buf_size > 0, "invalid buf_size");
		static_assert(((buf_size + sizeof(int)) % 16) == 0, "invalid buf_size");

		template<int length>
		constexpr void append(const char* from) {{
			for (int i = 0; i < length; ++i) {{ 
				buffer[data_size+i] = from[i]; 
			}}
			data_size += length;
		}}

		constexpr void append(char c) {{
			buffer[data_size] = c;
			data_size += 1;
		}}

		constexpr int size() {{ return data_size; }}
		constexpr bool empty() {{ return data_size == 0; }}
		constexpr string_view sv() {{ return {{ &buffer[0], data_size }}; }}
	private:
		int data_size = 0;
		char buffer[buf_size] = {{}};
	}};

    // Begin Default Templates
    template<class T>
    constexpr optional_value<T> from_string(const char* str, int len) noexcept = delete;

    template<class T>
    constexpr optional_value<T> from_string(const char* str) noexcept = delete;

    template<class T, class underlying_type>
    constexpr optional_value<T> from_integer(underlying_type value) noexcept = delete;

    template<class T>
    constexpr auto& values() noexcept = delete;

    template<class T>
    constexpr auto& flags() noexcept = delete;

    template<class T>
    constexpr bool is_valid(T e) noexcept = delete;

    template<class T>
    constexpr string_view enum_name() noexcept = delete;

    template<class T>
    constexpr string_view enum_name_with_namespace() noexcept = delete;

    template<class T>
    constexpr string_view enum_namespace() noexcept = delete;

    template<class T>
    constexpr void clear(T& value) noexcept = delete;

    template<class T>
    constexpr bool test(T value, T flags) noexcept = delete;

    template<class T>
    constexpr void set(T& value, T flags) noexcept = delete;

    template<class T>
    constexpr void unset(T& value, T flags) noexcept = delete;

    template<class T>
    constexpr void toggle(T& value, T flags) noexcept = delete;

    template<class T>
    constexpr bool has_all(T value) noexcept = delete;

    template<class T>
    constexpr bool has_any(T value) noexcept = delete;

    template<class T>
    constexpr bool has_none(T value) noexcept = delete;

    template<class T>
    constexpr bool has_single(T value) noexcept = delete;

    template<typename Value, typename Func>
    constexpr void flags_switch(Value v, Func&& func) {{
        static_assert(::enumbra::is_enumbra_flags_enum<Value>, "Value is not an enumbra flags enum");
        for (const Value flag : ::enumbra::flags<Value>()) {{
            if (::enumbra::has_any(v & flag)) {{
                func(v & flag);
            }}
        }}
    }}

    // End Default Templates
}} // end namespace enumbra
#else // check existing version supported
#if (ENUMBRA_BASE_TEMPLATES_VERSION + 0) == 0
#error ENUMBRA_BASE_TEMPLATES_VERSION has been defined without a proper version number. Check your build system.
#elif (ENUMBRA_BASE_TEMPLATES_VERSION + 0) < {0}
#error An included header was generated using a newer version of enumbra. Regenerate your headers using same version of enumbra.
#elif (ENUMBRA_BASE_TEMPLATES_VERSION + 0) > {0}
#error An included header was generated using an older version of enumbra. Regenerate your headers using same version of enumbra.
#endif // check existing version supported
#endif // ENUMBRA_BASE_TEMPLATES_VERSION)";

    wl(str_templates, enumbra_templates_version);
    wlf();
}

void cpp_generator::emit_ve_definition(const value_enum_context &e) {
    // START NAMESPACE
    wvl("namespace {enum_ns} {{");

    // Definition
    wvl("enum class {enum_name} : {size_type} {{");
    for (const auto &v: e.values) {
        wl("{} = {},", v.name, Int128Format{v.p_value, e.size_type_bits, e.is_size_type_signed});
    }
    wlu("};");

    // END NAMESPACE
    wlu("}");
    wlf();
}

void cpp_generator::emit_ve_detail(const value_enum_context &e) {
    
    const auto& enum_name_fq = store_map_.at("enum_name_fq");

    // begin detail namespace
    wvl("namespace {enum_ns}::detail::{enum_name} {{");

    // values_arr
    wvl("constexpr {enum_name_fq} values_arr[{entry_count}] =");
    wlu("{");
    for (const auto &v: e.values) {
        wl("{0}::{1},", enum_name_fq, v.name);
    }
    wlu("};");

    if (e.values.size() > 1) {
        // enum_strings
        size_t total_char_count = 0;
        for (auto &s: e.string_tables.entries) {
            total_char_count += s.name.size();
            total_char_count += 1; // null terminator
        }
        total_char_count += 1; // Final terminator

        wl("constexpr const char enum_strings[{0}] = {{", total_char_count);
        for (auto &s: e.string_tables.entries) {
            wl(R"("{0}\0")", s.name);
        }
        wlu("};");

        if (!e.is_one_string_table) {
            // enum_string_values
            wvl("constexpr {enum_name_fq} enum_string_values[{entry_count}] = {{");
            for (auto &v: e.string_tables.entries) {
                wl("{0}::{1},", enum_name_fq, v.name);
            }
            wlu("};");
        }
    }

    // End detail namespace
    wlu("}");

    wlf();
}

void cpp_generator::emit_ve_func_values(const value_enum_context &) {
    wlu("template<>");
    wvl("constexpr auto& enumbra::values<::{enum_ns}::{enum_name}>() noexcept");
    wlu("{");
    wvl("return {enum_detail_ns}::values_arr;");
    wlu("}");
    wlf();
}

void cpp_generator::emit_ve_func_from_integer(const value_enum_context &e) {
    // NOTE: If you modify this function, also check if changes are needed for emit_ve_func_is_valid
    
    // from_integer variations
    wlu("template<>");
    wvl("constexpr ::enumbra::optional_value<{enum_name_fq}> enumbra::from_integer<{enum_name_fq}>({size_type} v) noexcept {{ ");
    if (e.values.size() == 1) {
        wvl("if({max_v} == v) {{ return ::enumbra::optional_value<{enum_name_fq}>(static_cast<{enum_name_fq}>(v)); }}");
        wvl("return {{}};");
        wlu("}");
    } else if (e.is_range_contiguous) {
        // Unsigned values can't go below 0 so we just need to check that we're <= max
        if ((e.min_entry.p_value == 0) && !e.is_size_type_signed) {
            wvl("if(v <= {max_v}) {{ return ::enumbra::optional_value<{enum_name_fq}>(static_cast<{enum_name_fq}>(v)); }}");
            wvl("return {{}};");
            wlu("}");
        } else {
            wvl("if(({min_v} <= v) && (v <= {max_v})) {{ return ::enumbra::optional_value<{enum_name_fq}>(static_cast<{enum_name_fq}>(v)); }}");
            wvl("return {{}};");
            wlu("}");
        }
    } else {
        wvl("for(auto value : values<{enum_name_fq}>()) {{");
        wvl("if(value == static_cast<{enum_name_fq}>(v)) {{ return ::enumbra::optional_value<{enum_name_fq}>(static_cast<{enum_name_fq}>(v)); }}");
        wlu("}");
        wvl("return {{}};");
        wlu("}");
    }
    wlf();
}

void cpp_generator::emit_ve_func_is_valid(const value_enum_context& e)
{
    // NOTE: If you modify this function, also check if changes are needed for emit_ve_func_from_integer

    wlu("template<>");
    wvl("constexpr bool ::enumbra::is_valid<{enum_name_fq}>({enum_name_fq} e) noexcept {{ ");
    
    if (e.values.size() == 1) {
        wvl("return {max_v} == static_cast<{size_type}>(e);");
    }
    else if (e.is_range_contiguous) {
        // Unsigned values can't go below 0 so we just need to check that we're <= max
        if ((e.min_entry.p_value == 0) && !e.is_size_type_signed) {
            wvl("return static_cast<{size_type}>(e) <= {max_v};");
        }
        else {
            wvl("return ({min_v} <= static_cast<{size_type}>(e)) && (static_cast<{size_type}>(e) <= {max_v});");
        }
    }
    else {
        wvl("for(auto value : values<{enum_name_fq}>()) {{");
        wvl("if(value == e) {{ return true; }}");
        wlu("}");
        wlu("return false;");
    }
    
    wlu("}");
    wlf();
}

void cpp_generator::emit_ve_func_enum_name(const value_enum_context& /*e*/)
{
    wlu("template<>");
    wvl("constexpr ::enumbra::string_view enumbra::enum_name<{enum_name_fq}>() noexcept {{ ");
    wvl("return {{ \"{enum_name}\", {enum_name_size} }};");
    wlu("}");
    
    wlf();

    wlu("template<>");
    wvl("constexpr ::enumbra::string_view enumbra::enum_name_with_namespace<{enum_name_fq}>() noexcept {{ ");
    wvl("return {{ \"{enum_name_fq_no_global}\", {enum_name_fq_no_global_size} }};");
    wlu("}");

    wlf();

    wlu("template<>");
    wvl("constexpr ::enumbra::string_view enumbra::enum_namespace<{enum_name_fq}>() noexcept {{ ");
    wvl("return {{ \"{enum_ns}\", {enum_ns_size} }};");
    wlu("}");

    wlf();
}

void cpp_generator::emit_ve_func_to_string(const value_enum_context &e) {
    
    const auto& enum_name_fq = store_map_.at("enum_name_fq");
    const auto& enum_ns = store_map_.at("enum_ns");
    
    // START NAMESPACE
    wlu("namespace enumbra {");

    wvl("constexpr ::enumbra::string_view to_string(const {enum_name_fq} v) noexcept {{");
    wlu("switch (v) {");
    if (e.string_tables.entries.size() == 1) {
        for (auto &v: e.values) {
            wl("case {0}::{1}: return {{ \"{1}\", {2} }};", enum_name_fq, v.name, v.name.size());
        }
    } else {
        for (auto &entry: e.string_tables.tables) {
            size_t offset = entry.offset_str;
            for (auto &e_name: entry.names) {
                wl("case {0}::{1}: return {{ &::{2}::detail::{3}::enum_strings[{4}], {5} }};",
                    enum_name_fq, e_name, enum_ns, e.enum_name, offset, e_name.size());
                offset += entry.size + 1;
            }
        }
    }
    wlu("}");
    wlu("return { nullptr, 0 };");
    wlu("}");

    // END NAMESPACE
    wlu("}");
    wlf();
}

void cpp_generator::emit_ve_func_from_string_with_size(const value_enum_context &e) {
    if (e.values.size() == 1) {
        const auto &v = e.values.at(0);
        push("entry_name", v.name);
        push("entry_name_len", std::to_string(v.name.length()));
        wlu("template<>");
        wvl("constexpr ::enumbra::optional_value<{enum_name_fq}> enumbra::from_string<{enum_name_fq}>(const char* str, int len) noexcept {{");
        wvl("if ((len == {entry_name_len}) && ::enumbra::detail::streq_fixed_size<{entry_name_len}>(\"{entry_name}\", str)) {{");
        wvl("return ::enumbra::optional_value<{enum_name_fq}>({enum_name_fq}::{entry_name});");
        wlu("}");
        wlu("return {};");
        wlu("}");
        pop("entry_name");
        pop("entry_name_len");
    } else {
        wlu("template<>");
        wvl("constexpr ::enumbra::optional_value<{enum_name_fq}> enumbra::from_string<{enum_name_fq}>(const char* str, int len) noexcept {{");
        if (e.string_tables.tables.size() == 1) {
            auto &first = e.string_tables.tables.front();
            push("entry_name_len", std::to_string(first.size));
            wvl("if(len != {entry_name_len}) {{ return {{}}; }}");
            wl("constexpr int offset_str = {0};", first.offset_str);
            wl("constexpr int offset_enum = {0};", first.offset_enum);
            wl("constexpr int count = {0};", first.count);
            wlu("for (int i = 0; i < count; i++) {");
            wvl("if (::enumbra::detail::streq_fixed_size<{entry_name_len}>({enum_detail_ns}::enum_strings + offset_str + (i * (len + 1)), str)) {{");
            pop("entry_name_len");
        } else {
            wlu("int offset_str = 0;");
            wlu("int offset_enum = 0;");
            wlu("int count = 0;");
            wlu("switch(len) {");
            for (auto &entry: e.string_tables.tables) {
                wl("case {0}: offset_str = {1}; offset_enum = {2}; count = {3}; break;",
                   entry.size, entry.offset_str, entry.offset_enum, entry.count);
            }
            wlu("default: return {};");
            wlu("}");
            wlu("for (int i = 0; i < count; i++) {");
            wvl("if (::enumbra::detail::streq_known_size({enum_detail_ns}::enum_strings + offset_str + (i * (len + 1)), str, len)) {{");
        }

        if (e.is_one_string_table) {
            wvl("return ::enumbra::optional_value<{enum_name_fq}>({enum_detail_ns}::values_arr[offset_enum + i]);");
        } else {
            wvl("return ::enumbra::optional_value<{enum_name_fq}>({enum_detail_ns}::enum_string_values[offset_enum + i]);");
        }
        wlu("}");
        wlu("}");

        wlu("return {};");
        wlu("}");
    }
    wlf();
}

void cpp_generator::emit_ve_func_from_string_cstr(const value_enum_context& /*e*/)
{
    wlu("template<>");
    wvl("constexpr ::enumbra::optional_value<{enum_name_fq}> enumbra::from_string<{enum_name_fq}>(const char* str) noexcept {{");
    wvl("const int len = ::enumbra::detail::strlen(str);");
    wvl("return ::enumbra::from_string<{enum_name_fq}>(str, len);");
    wlu("}");
    wlf();
}

