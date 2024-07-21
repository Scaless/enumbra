// THIS FILE WAS GENERATED BY A TOOL: https://github.com/Scaless/enumbra
// It is highly recommended that you not make manual edits to this file,
// as they will be overwritten when the file is re-generated.
// Generated by enumbra v0.2.0

#pragma once

#include <cstdint>

#if !defined(ENUMBRA_REQUIRED_MACROS_VERSION) 
#define ENUMBRA_REQUIRED_MACROS_VERSION 9

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
#elif (ENUMBRA_REQUIRED_MACROS_VERSION + 0) < 9 
#error An included header was generated using a newer version of enumbra. Regenerate your headers using the same version. 
#elif (ENUMBRA_REQUIRED_MACROS_VERSION + 0) > 9 
#error An included header was generated using an older version of enumbra. Regenerate your headers using the same version. 
#endif // end check existing version supported
#endif // ENUMBRA_REQUIRED_MACROS_VERSION

#if !defined(ENUMBRA_OPTIONAL_MACROS_VERSION)
#define ENUMBRA_OPTIONAL_MACROS_VERSION 6

// Bitfield convenience functions
#define ENUMBRA_ZERO(Field) { decltype(Field) _field_ = Field; zero(_field_); Field = _field_; }
#define ENUMBRA_SET(Field, Value) { decltype(Field) _field_ = Field; set(_field_, Value); Field = _field_; }
#define ENUMBRA_UNSET(Field, Value) { decltype(Field) _field_ = Field; unset(_field_, Value); Field = _field_; }
#define ENUMBRA_TOGGLE(Field, Value) { decltype(Field) _field_ = Field; toggle(_field_, Value); Field = _field_; }

// Bit field storage helper
#define ENUMBRA_PACK_UNINITIALIZED(Enum, Name) Enum Name : ::enumbra::bits_required_storage<Enum>();
#define ENUMBRA_INIT(Name, InitValue) Name(::enumbra::default_value<decltype(Name)>())
#define ENUMBRA_INIT_DEFAULT(Name) Name(::enumbra::default_value<decltype(Name)>())

#if ENUMBRA_CPP_VERSION >= 20
// Bit field storage helper with type-checked member initialization
#define ENUMBRA_PACK_INIT(Enum, Name, InitValue) Enum Name : ::enumbra::bits_required_storage<Enum>() { InitValue };
// Bit field storage helper with default value initialization
#define ENUMBRA_PACK_INIT_DEFAULT(Enum, Name) Enum Name : ::enumbra::bits_required_storage<Enum>() { ::enumbra::default_value<Enum>() };
#endif

#else // check existing version supported
#if (ENUMBRA_OPTIONAL_MACROS_VERSION + 0) == 0
#error ENUMBRA_OPTIONAL_MACROS_VERSION has been defined without a proper version number. Check your build system.
#elif (ENUMBRA_OPTIONAL_MACROS_VERSION + 0) < 6
#error An included header was generated using a newer version of enumbra. Regenerate your headers using the same version.
#elif (ENUMBRA_OPTIONAL_MACROS_VERSION + 0) > 6
#error An included header was generated using an older version of enumbra. Regenerate your headers using the same version.
#endif // end check existing version supported
#endif // ENUMBRA_OPTIONAL_MACROS_VERSION

#if !defined(ENUMBRA_BASE_TEMPLATES_VERSION)
#define ENUMBRA_BASE_TEMPLATES_VERSION 19
namespace enumbra {
    namespace detail {
        // Re-Implementation of std:: features to avoid including std headers
        template<bool B, class T = void>
        struct enable_if {};
        template<class T>
        struct enable_if<true, T> { typedef T type; };

        // Type info
        template<bool is_enumbra, bool is_value_enum, bool is_flags_enum>
        struct type_info { 
            static constexpr bool enumbra_type = is_enumbra;
            static constexpr bool enumbra_value_enum = is_value_enum;
            static constexpr bool enumbra_flags_enum = is_flags_enum;
        };

        // Value enum info
        template<typename underlying_type, underlying_type min_v, underlying_type max_v,
            underlying_type default_v, ::std::int32_t count_v,
            bool is_contiguous_v, ::std::int32_t bits_required_storage_v, ::std::int32_t bits_required_transmission_v>
        struct value_enum_info {
            using underlying_t = underlying_type;
            static constexpr underlying_type min = min_v;
            static constexpr underlying_type max = max_v;
            static constexpr underlying_type def = default_v;
            static constexpr ::std::int32_t count = count_v;
            static constexpr bool is_contiguous = is_contiguous_v;
            static constexpr ::std::int32_t bits_required_storage = bits_required_storage_v;
            static constexpr ::std::int32_t bits_required_transmission = bits_required_transmission_v;
        };

        // Flags enum info
        template<typename underlying_type, underlying_type min_v, underlying_type max_v, 
            underlying_type default_v, ::std::int32_t count_v,
            bool is_contiguous_v, ::std::int32_t bits_required_storage_v, ::std::int32_t bits_required_transmission_v>
        struct flags_enum_info {
            using underlying_t = underlying_type;
            static constexpr underlying_type min = min_v;
            static constexpr underlying_type max = max_v;
            static constexpr underlying_type default_value = default_v;
            static constexpr ::std::int32_t count = count_v;
            static constexpr bool is_contiguous = is_contiguous_v;
            static constexpr ::std::int32_t bits_required_storage = bits_required_storage_v;
            static constexpr ::std::int32_t bits_required_transmission = bits_required_transmission_v;
        };
        
        // Default template for non-enumbra types
        template<class T>
        struct base_helper : type_info<false, false, false> { };
        template<class T>
        struct value_enum_helper;
        template<class T>
        struct flags_enum_helper;

        // Constexpr string compare
        constexpr bool streq_s(const char* a, ::std::uint32_t a_len, const char* b, ::std::uint32_t b_len) noexcept {
            if(a_len != b_len) { return false; }
            for(::std::uint32_t i = 0; i < a_len; ++i) { if(a[i] != b[i]) { return false; } }
            return true;
        }
        constexpr bool streq_known_size(const char* a, const char* b, ::std::uint32_t len) noexcept {
            for(::std::uint32_t i = 0; i < len; ++i) { if(a[i] != b[i]) { return false; } }
            return true;
        }
        template<uint32_t length>
        constexpr bool streq_fixed_size(const char* a, const char* b) noexcept {
            static_assert(length > 0);
            for(::std::uint32_t i = 0; i < length; ++i) { if(a[i] != b[i]) { return false; } }
            return true;
        }
    } // end namespace enumbra::detail
    template<class T>
    constexpr bool is_enumbra_enum = detail::base_helper<T>::enumbra_type;
    template<class T>
    constexpr bool is_enumbra_value_enum = is_enumbra_enum<T> && detail::base_helper<T>::enumbra_value_enum;
    template<class T>
    constexpr bool is_enumbra_flags_enum = is_enumbra_enum<T> && detail::base_helper<T>::enumbra_flags_enum;

    template<class T, typename ::enumbra::detail::enable_if<is_enumbra_value_enum<T>, T>::type* = nullptr>
    constexpr T min() noexcept { return static_cast<T>(detail::value_enum_helper<T>::min); }
    template<class T, typename ::enumbra::detail::enable_if<is_enumbra_flags_enum<T>, T>::type* = nullptr>
    constexpr T min() noexcept { return static_cast<T>(detail::flags_enum_helper<T>::min); }
    template<class T, typename ::enumbra::detail::enable_if<!is_enumbra_enum<T>, T>::type* = nullptr>
    constexpr T min() noexcept = delete;

    template<class T, typename ::enumbra::detail::enable_if<is_enumbra_value_enum<T>, T>::type* = nullptr>
    constexpr T max() noexcept { return static_cast<T>(detail::value_enum_helper<T>::max); }
    template<class T, typename ::enumbra::detail::enable_if<is_enumbra_flags_enum<T>, T>::type* = nullptr>
    constexpr T max() noexcept { return static_cast<T>(detail::flags_enum_helper<T>::max); }
    template<class T, typename ::enumbra::detail::enable_if<!is_enumbra_enum<T>, T>::type* = nullptr>
    constexpr T max() noexcept = delete;

    template<class T, typename ::enumbra::detail::enable_if<is_enumbra_value_enum<T>, T>::type* = nullptr>
    constexpr T default_value() noexcept { return static_cast<T>(detail::value_enum_helper<T>::default_value); }
    template<class T, typename ::enumbra::detail::enable_if<is_enumbra_flags_enum<T>, T>::type* = nullptr>
    constexpr T default_value() noexcept { return static_cast<T>(detail::flags_enum_helper<T>::default_value); }
    template<class T, typename ::enumbra::detail::enable_if<!is_enumbra_enum<T>, T>::type* = nullptr>
    constexpr T default_value() noexcept = delete;

    template<class T, typename ::enumbra::detail::enable_if<is_enumbra_value_enum<T>, T>::type* = nullptr>
    constexpr ::std::int32_t count() noexcept { return detail::value_enum_helper<T>::count; }
    template<class T, typename ::enumbra::detail::enable_if<is_enumbra_flags_enum<T>, T>::type* = nullptr>
    constexpr ::std::int32_t count() noexcept { return detail::flags_enum_helper<T>::count; }
    template<class T, typename ::enumbra::detail::enable_if<!is_enumbra_enum<T>, T>::type* = nullptr>
    constexpr ::std::int32_t count() noexcept = delete;

    template<class T, typename ::enumbra::detail::enable_if<is_enumbra_value_enum<T>, T>::type* = nullptr>
    constexpr bool is_contiguous() noexcept { return detail::value_enum_helper<T>::is_contiguous; }
    template<class T, typename ::enumbra::detail::enable_if<is_enumbra_flags_enum<T>, T>::type* = nullptr>
    constexpr bool is_contiguous() noexcept { return detail::flags_enum_helper<T>::is_contiguous; }
    template<class T, typename ::enumbra::detail::enable_if<!is_enumbra_enum<T>, T>::type* = nullptr>
    constexpr bool is_contiguous() noexcept = delete;

    template<class T, typename ::enumbra::detail::enable_if<is_enumbra_value_enum<T>, T>::type* = nullptr>
    constexpr ::std::int32_t bits_required_storage() noexcept { return detail::value_enum_helper<T>::bits_required_storage; }
    template<class T, typename ::enumbra::detail::enable_if<is_enumbra_flags_enum<T>, T>::type* = nullptr>
    constexpr ::std::int32_t bits_required_storage() noexcept { return detail::flags_enum_helper<T>::bits_required_storage; }
    template<class T, typename ::enumbra::detail::enable_if<!is_enumbra_enum<T>, T>::type* = nullptr>
    constexpr ::std::int32_t bits_required_storage() noexcept = delete;

    template<class T, typename ::enumbra::detail::enable_if<is_enumbra_value_enum<T>, T>::type* = nullptr>
    constexpr ::std::int32_t bits_required_transmission() noexcept { return detail::value_enum_helper<T>::bits_required_transmission; }
    template<class T, typename ::enumbra::detail::enable_if<is_enumbra_flags_enum<T>, T>::type* = nullptr>
    constexpr ::std::int32_t bits_required_transmission() noexcept { return detail::flags_enum_helper<T>::bits_required_transmission; }
    template<class T, typename ::enumbra::detail::enable_if<!is_enumbra_enum<T>, T>::type* = nullptr>
    constexpr ::std::int32_t bits_required_transmission() noexcept = delete;

    template<class T, class underlying_type = typename detail::base_helper<T>::base_type, typename ::enumbra::detail::enable_if<is_enumbra_enum<T>, T>::type* = nullptr>
    constexpr T from_integer_unsafe(underlying_type e) noexcept { return static_cast<T>(e); }
    template<class T, class underlying_type = typename detail::base_helper<T>::base_type, typename ::enumbra::detail::enable_if<!is_enumbra_enum<T>, T>::type* = nullptr>
    constexpr T from_integer_unsafe(underlying_type e) noexcept = delete;

    template<class T, class underlying_type = typename detail::value_enum_helper<T>::underlying_t, typename ::enumbra::detail::enable_if<is_enumbra_value_enum<T>, T>::type* = nullptr>
    constexpr underlying_type to_underlying(T e) noexcept { return static_cast<underlying_type>(e); }
    template<class T, class underlying_type = typename detail::flags_enum_helper<T>::underlying_t, typename ::enumbra::detail::enable_if<is_enumbra_flags_enum<T>, T>::type* = nullptr>
    constexpr underlying_type to_underlying(T e) noexcept { return static_cast<underlying_type>(e); }
    template<class T, class underlying_type = T, typename ::enumbra::detail::enable_if<!is_enumbra_enum<T>, T>::type* = nullptr>
    constexpr underlying_type to_underlying(T e) noexcept = delete;

    template<class T>
    struct from_string_result
    {
        bool success;
        T value;
    };

    template <class T>
    struct from_integer_result
    {
        bool success;
        T value;
    };

    // Begin Default Templates
    template<class T>
    constexpr from_string_result<T> from_string(const char* str, ::std::uint16_t len) noexcept = delete;

    template<class T>
    constexpr auto& values() noexcept = delete;

    template<class T>
    constexpr auto& flags() noexcept = delete;

    template<class T, class underlying_type = typename detail::base_helper<T>::base_type>
    constexpr from_integer_result<T> from_integer(underlying_type value) noexcept = delete;
    // End Default Templates
} // end namespace enumbra
#else // check existing version supported
#if (ENUMBRA_BASE_TEMPLATES_VERSION + 0) == 0
#error ENUMBRA_BASE_TEMPLATES_VERSION has been defined without a proper version number. Check your build system.
#elif (ENUMBRA_BASE_TEMPLATES_VERSION + 0) < 19
#error An included header was generated using a newer version of enumbra. Regenerate your headers using same version of enumbra.
#elif (ENUMBRA_BASE_TEMPLATES_VERSION + 0) > 19
#error An included header was generated using an older version of enumbra. Regenerate your headers using same version of enumbra.
#endif // check existing version supported
#endif // ENUMBRA_BASE_TEMPLATES_VERSION

namespace enums {
// minimal_val Definition
enum class minimal_val : uint32_t {
B = 1,
C = 2,
};

namespace detail::minimal_val {
constexpr ::enums::minimal_val values_arr[2] =
{
::enums::minimal_val::B,
::enums::minimal_val::C,
};
constexpr const char enum_strings[5] = {
"B\0"
"C\0"
};
}
} // namespace enums

namespace enumbra {
template<>
constexpr auto& values<::enums::minimal_val>() noexcept
{
return ::enums::detail::minimal_val::values_arr;
}

template<>
constexpr ::enumbra::from_integer_result<::enums::minimal_val> from_integer<::enums::minimal_val>(uint32_t v) noexcept { 
if((1 <= v) && (v <= 2)) { return { true, static_cast<::enums::minimal_val>(v) }; }
return { false, ::enums::minimal_val() };
}

constexpr const char* to_string(const ::enums::minimal_val v) noexcept {
switch (v) {
case ::enums::minimal_val::B: return &::enums::detail::minimal_val::enum_strings[0];
case ::enums::minimal_val::C: return &::enums::detail::minimal_val::enum_strings[2];
}
return nullptr;
}

template<>
constexpr ::enumbra::from_string_result<::enums::minimal_val> from_string<::enums::minimal_val>(const char* str, ::std::uint16_t len) noexcept {
if(len != 1) { return {false, ::enums::minimal_val()}; }
constexpr ::std::uint32_t offset_str = 0;
constexpr ::std::uint32_t offset_enum = 0;
constexpr ::std::uint32_t count = 2;
for (::std::uint32_t i = 0; i < count; i++) {
if (enumbra::detail::streq_fixed_size<1>(::enums::detail::minimal_val::enum_strings + offset_str + (i * (len + 1)), str)) {
return {true, ::enums::detail::minimal_val::values_arr[offset_enum + i]};
}
}
return {false, ::enums::minimal_val()};
}

} // enumbra

namespace enums {
// minimal Definition
enum class minimal : uint32_t {
B = 1,
C = 2,
};

namespace detail::minimal {
constexpr ::enums::minimal flags_arr[2] =
{
::enums::minimal::B,
::enums::minimal::C,
};
}

} // namespace enums

namespace enumbra {
template<>
constexpr auto& flags<::enums::minimal>() noexcept
{
return ::enums::detail::minimal::flags_arr;
}

constexpr void zero(::enums::minimal& value) noexcept { value = static_cast<::enums::minimal>(0); }
constexpr bool test(::enums::minimal value, ::enums::minimal flags) noexcept { return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(value)) == static_cast<uint32_t>(flags); }
constexpr void set(::enums::minimal& value, ::enums::minimal flags) noexcept { value = static_cast<::enums::minimal>(static_cast<uint32_t>(value) | static_cast<uint32_t>(flags)); }
constexpr void unset(::enums::minimal& value, ::enums::minimal flags) noexcept { value = static_cast<::enums::minimal>(static_cast<uint32_t>(value) & (~static_cast<uint32_t>(flags))); }
constexpr void toggle(::enums::minimal& value, ::enums::minimal flags) noexcept { value = static_cast<::enums::minimal>(static_cast<uint32_t>(value) ^ static_cast<uint32_t>(flags)); }
constexpr bool is_all(::enums::minimal value) noexcept { return static_cast<uint32_t>(value) >= 0x3; }
constexpr bool is_any(::enums::minimal value) noexcept { return static_cast<uint32_t>(value) > 0; }
constexpr bool is_none(::enums::minimal value) noexcept { return static_cast<uint32_t>(value) == 0; }
constexpr bool is_single(::enums::minimal value) noexcept { uint32_t n = static_cast<uint32_t>(value); return n && !(n & (n - 1)); }

// ::enums::minimal Operator Overloads
constexpr ::enums::minimal operator~(const ::enums::minimal a) noexcept { return static_cast<::enums::minimal>(~static_cast<uint32_t>(a)); }
constexpr ::enums::minimal operator|(const ::enums::minimal a, const ::enums::minimal b) noexcept { return static_cast<::enums::minimal>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b)); }
constexpr ::enums::minimal operator&(const ::enums::minimal a, const ::enums::minimal b) noexcept { return static_cast<::enums::minimal>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b)); }
constexpr ::enums::minimal operator^(const ::enums::minimal a, const ::enums::minimal b) noexcept { return static_cast<::enums::minimal>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b)); }
constexpr ::enums::minimal& operator|=(::enums::minimal& a, const ::enums::minimal b) noexcept { return a = a | b; }
constexpr ::enums::minimal& operator&=(::enums::minimal& a, const ::enums::minimal b) noexcept { return a = a & b; }
constexpr ::enums::minimal& operator^=(::enums::minimal& a, const ::enums::minimal b) noexcept { return a = a ^ b; }
} // enumbra

// Template Specializations Begin
template<> struct enumbra::detail::base_helper<enums::minimal_val> : enumbra::detail::type_info<true, true, false> { };
template<> struct enumbra::detail::value_enum_helper<enums::minimal_val> : enumbra::detail::value_enum_info<uint32_t, 1, 2, 1, 2, true, 2, 1> { };
template<> struct enumbra::detail::base_helper<enums::minimal> : enumbra::detail::type_info<true, false, true> { };
template<> struct enumbra::detail::flags_enum_helper<enums::minimal> : enumbra::detail::flags_enum_info<uint32_t, 0, 3, 0, 2, true, 2, 2> { };
// Template Specializations End