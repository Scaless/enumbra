// THIS FILE WAS GENERATED BY A TOOL: https://github.com/Scaless/enumbra
// It is highly recommended that you not make manual edits to this file,
// as they will be overwritten when the file is re-generated.
// Generated by enumbra v0.2.3

#ifndef ENUMBRA_8DFC7B86EEA68504_H
#define ENUMBRA_8DFC7B86EEA68504_H


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

#ifndef ENUMBRA_NO_OPTIONAL_MACROS
#if !defined(ENUMBRA_OPTIONAL_MACROS_VERSION)
#define ENUMBRA_OPTIONAL_MACROS_VERSION 10

// Bitfield convenience functions
#define ENUMBRA_CLEAR(Field) do { decltype(Field) _field_ = Field; ::enumbra::clear(_field_); (Field) = _field_; } while (0)
#define ENUMBRA_SET(Field, Value) do { decltype(Field) _field_ = Field; ::enumbra::set(_field_, Value); (Field) = _field_; } while (0)
#define ENUMBRA_UNSET(Field, Value) do { decltype(Field) _field_ = Field; ::enumbra::unset(_field_, Value); (Field) = _field_; } while (0)
#define ENUMBRA_TOGGLE(Field, Value) do { decltype(Field) _field_ = Field; ::enumbra::toggle(_field_, Value); (Field) = _field_; } while (0)

// Bit field storage helper
#define ENUMBRA_PACK_UNINITIALIZED(Enum, Name) Enum Name : ::enumbra::bits_required_storage<Enum>()
#define ENUMBRA_INIT(Name, InitValue) Name(::enumbra::default_value<decltype(Name)>())
#define ENUMBRA_INIT_DEFAULT(Name) Name(::enumbra::default_value<decltype(Name)>())

// Iterate flags in a switch
#define ENUMBRA_FLAGS_SWITCH_BEGIN(var) do { for (const auto flag : ::enumbra::flags<decltype(var)>()) { if (::enumbra::has_any(var & flag)) { switch (var & flag)
#define ENUMBRA_FLAGS_SWITCH_END } } } while (0)

#if ENUMBRA_CPP_VERSION >= 20
// Bit field storage helper with type-checked member initialization
#define ENUMBRA_PACK_INIT(Enum, Name, InitValue) Enum Name : ::enumbra::bits_required_storage<Enum>() { InitValue }
// Bit field storage helper with default value initialization
#define ENUMBRA_PACK_INIT_DEFAULT(Enum, Name) Enum Name : ::enumbra::bits_required_storage<Enum>() { ::enumbra::default_value<Enum>() }
#endif

#else // check existing version supported
#if (ENUMBRA_OPTIONAL_MACROS_VERSION + 0) == 0
#error ENUMBRA_OPTIONAL_MACROS_VERSION has been defined without a proper version number. Check your build system.
#elif (ENUMBRA_OPTIONAL_MACROS_VERSION + 0) < 10
#error An included header was generated using a newer version of enumbra. Regenerate your headers using the same version.
#elif (ENUMBRA_OPTIONAL_MACROS_VERSION + 0) > 10
#error An included header was generated using an older version of enumbra. Regenerate your headers using the same version.
#endif // end check existing version supported
#endif // ENUMBRA_OPTIONAL_MACROS_VERSION
#endif

#if !defined(ENUMBRA_BASE_TEMPLATES_VERSION)
#define ENUMBRA_BASE_TEMPLATES_VERSION 31
namespace enumbra {
    namespace detail {
        // Re-Implementation of std:: features to avoid including std headers
        template<bool B, class T, class F>
        struct conditional { using type = T; };
        template<class T, class F>
        struct conditional<false, T, F> { using type = F; };
        template<bool B, class T, class F>
        using conditional_t = typename conditional<B, T, F>::type;

 #if defined(__cpp_lib_is_constant_evaluated)
        // Supported on clang/gcc/MSVC in C++17, even though it's only in the C++20 standard. 
        constexpr bool is_constant_evaluated() noexcept { return __builtin_is_constant_evaluated(); }
#else
        constexpr bool is_constant_evaluated() noexcept { return false; }
#endif

        // Type info
        template<bool is_enumbra, bool is_value_enum, bool is_flags_enum>
        struct type_info {
            static constexpr bool enumbra_type = is_enumbra;
            static constexpr bool enumbra_value_enum = is_value_enum;
            static constexpr bool enumbra_flags_enum = is_flags_enum;
        };

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
        struct enum_info {
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
        };

        // Default template for non-enumbra types
        template<class T>
        struct base_helper : type_info<false, false, false> { };
        template<class T>
        struct enum_helper;

        // Compare strings with sizes only known at runtime
        constexpr bool streq_known_size(const char* a, const char* b, int len) noexcept {
            for(int i = 0; i < len; ++i) { if(a[i] != b[i]) { return false; } }
            return true;
        }
        // Compare strings with sizes known at compile time
        template<int length>
        constexpr bool streq_fixed_size(const char* a, const char* b) noexcept {
            static_assert(length > 0);
            for(int i = 0; i < length; ++i) { if(a[i] != b[i]) { return false; } }
            return true;
        }
        // C-style string length
        constexpr int strlen(const char* a) noexcept {
            if (a == nullptr) { return 0; }
            int count = 0;
            while (a[count] != 0) { count++; }
            return count;
        }
    } // end namespace enumbra::detail

    template<class T>
    constexpr bool is_enumbra_enum = detail::base_helper<T>::enumbra_type;
    template<class T>
    constexpr bool is_enumbra_value_enum = detail::base_helper<T>::enumbra_value_enum;
    template<class T>
    constexpr bool is_enumbra_flags_enum = detail::base_helper<T>::enumbra_flags_enum;

    template<class T>
    constexpr T min() {
        static_assert(is_enumbra_enum<T>, "T is not an enumbra enum");
        return static_cast<T>(detail::enum_helper<T>::min);
    }

    template<class T>
    constexpr T max() {
        static_assert(is_enumbra_enum<T>, "T is not an enumbra enum");
        return static_cast<T>(detail::enum_helper<T>::max);
    }

    template<class T>
    constexpr T default_value() {
        static_assert(is_enumbra_enum<T>, "T is not an enumbra enum");
        return static_cast<T>(detail::enum_helper<T>::default_value);
    }

    template<class T>
    constexpr int count() {
        static_assert(is_enumbra_enum<T>, "T is not an enumbra enum");
        return detail::enum_helper<T>::count;
    }

    template<class T>
    constexpr bool is_contiguous() {
        static_assert(is_enumbra_enum<T>, "T is not an enumbra enum");
        return detail::enum_helper<T>::is_contiguous;
    }

    template<class T>
    constexpr int bits_required_storage() {
        static_assert(is_enumbra_enum<T>, "T is not an enumbra enum");
        return detail::enum_helper<T>::bits_required_storage;
    }
 
    template<class T>
    constexpr int bits_required_transmission() {
        static_assert(is_enumbra_enum<T>, "T is not an enumbra enum");
        return detail::enum_helper<T>::bits_required_transmission;
    }

    template<class T, class underlying_type>
    constexpr T from_integer_unsafe(underlying_type e) noexcept {
        static_assert(is_enumbra_enum<T>, "T is not an enumbra enum");
        return static_cast<T>(e);
    }

    template<class T>
    constexpr auto to_underlying(T e) noexcept {
        static_assert(is_enumbra_enum<T>, "T is not an enumbra enum");
        return static_cast<typename detail::enum_helper<T>::underlying_t>(e);
    }

    namespace detail {
        template<class T>
        struct optional_result_base_bool 
        {
        private:
            using bool_type = 
                detail::conditional_t<sizeof(T) == 1, char,
                detail::conditional_t<sizeof(T) == 2, short,
                detail::conditional_t<sizeof(T) == 4, int,
                detail::conditional_t<sizeof(T) == 8, long long,
                void /* invalid size */>>>>;
        protected:
            bool_type success = 0;
        };
        
        struct optional_result_base_inplace { };
    }

    template<class T, bool use_invalid_sentinel = detail::enum_helper<T>::has_invalid_sentinel>
    struct optional_value : detail::conditional_t<use_invalid_sentinel, detail::optional_result_base_inplace, detail::optional_result_base_bool<T>>
    {
    private:
        T v = static_cast<T>(detail::enum_helper<T>::invalid_sentinel);
    public:
        constexpr optional_value() : v(static_cast<T>(detail::enum_helper<T>::invalid_sentinel)) { }

        constexpr explicit optional_value(T value) : v(value) {
            if constexpr(!use_invalid_sentinel) {
                this->success = 1;
            }
        }

        constexpr explicit operator bool() const noexcept {
            if constexpr (use_invalid_sentinel) {
                return v != static_cast<T>(detail::enum_helper<T>::invalid_sentinel);
            } else {
                return this->success > 0;
            }
        }

        constexpr bool has_value() const { return operator bool(); }
        constexpr T value() const { return v; }
        constexpr T value_or(T default_value) const { return operator bool() ? v : default_value; }
    };

    struct string_view {
        using size_type = detail::conditional_t<sizeof(void*) == 4, int, long long>;

        const char* str = nullptr;
        size_type size = 0;

        constexpr bool empty() const { return size == 0; }
    };

	template<int buf_size>
	struct stack_string {
		static_assert(buf_size > 0, "invalid buf_size");
		static_assert(((buf_size + sizeof(int)) % 16) == 0, "invalid buf_size");

		template<int length>
		constexpr void append(const char* from) {
			for (int i = 0; i < length; ++i) { 
				buffer[data_size+i] = from[i]; 
			}
			data_size += length;
		}

		constexpr void append(char c) {
			buffer[data_size] = c;
			data_size += 1;
		}

		constexpr int size() { return data_size; }
		constexpr bool empty() { return data_size == 0; }
		constexpr string_view sv() { return { &buffer[0], data_size }; }
	private:
		int data_size = 0;
		char buffer[buf_size] = {};
	};

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
    constexpr void flags_switch(Value v, Func&& func) {
        static_assert(::enumbra::is_enumbra_flags_enum<Value>, "Value is not an enumbra flags enum");
        for (const Value flag : ::enumbra::flags<Value>()) {
            if (::enumbra::has_any(v & flag)) {
                func(v & flag);
            }
        }
    }

    // End Default Templates
} // end namespace enumbra
#else // check existing version supported
#if (ENUMBRA_BASE_TEMPLATES_VERSION + 0) == 0
#error ENUMBRA_BASE_TEMPLATES_VERSION has been defined without a proper version number. Check your build system.
#elif (ENUMBRA_BASE_TEMPLATES_VERSION + 0) < 31
#error An included header was generated using a newer version of enumbra. Regenerate your headers using same version of enumbra.
#elif (ENUMBRA_BASE_TEMPLATES_VERSION + 0) > 31
#error An included header was generated using an older version of enumbra. Regenerate your headers using same version of enumbra.
#endif // check existing version supported
#endif // ENUMBRA_BASE_TEMPLATES_VERSION

namespace enums {
enum class minimal_val : unsigned int {
B = 1,
C = 2,
};
}

template<> struct enumbra::detail::base_helper<::enums::minimal_val> : enumbra::detail::type_info<true, true, false> { };
template<> struct enumbra::detail::enum_helper<::enums::minimal_val> : enumbra::detail::enum_info<unsigned int, 1, 2, 1, 2, true, 2, 1, true, 0> { };

namespace enums::detail::minimal_val {
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

template<>
constexpr auto& enumbra::values<::enums::minimal_val>() noexcept
{
return ::enums::detail::minimal_val::values_arr;
}

template<>
constexpr ::enumbra::optional_value<::enums::minimal_val> enumbra::from_integer<::enums::minimal_val>(unsigned int v) noexcept { 
if((1 <= v) && (v <= 2)) { return ::enumbra::optional_value<::enums::minimal_val>(static_cast<::enums::minimal_val>(v)); }
return {};
}

template<>
constexpr bool ::enumbra::is_valid<::enums::minimal_val>(::enums::minimal_val e) noexcept { 
return (1 <= static_cast<unsigned int>(e)) && (static_cast<unsigned int>(e) <= 2);
}

template<>
constexpr ::enumbra::string_view enumbra::enum_name<::enums::minimal_val>() noexcept { 
return { "minimal_val", 11 };
}

template<>
constexpr ::enumbra::string_view enumbra::enum_name_with_namespace<::enums::minimal_val>() noexcept { 
return { "enums::minimal_val", 18 };
}

template<>
constexpr ::enumbra::string_view enumbra::enum_namespace<::enums::minimal_val>() noexcept { 
return { "enums", 5 };
}

namespace enumbra {
constexpr ::enumbra::string_view to_string(const ::enums::minimal_val v) noexcept {
switch (v) {
case ::enums::minimal_val::B: return { &::enums::detail::minimal_val::enum_strings[0], 1 };
case ::enums::minimal_val::C: return { &::enums::detail::minimal_val::enum_strings[2], 1 };
}
return { nullptr, 0 };
}
}

template<>
constexpr ::enumbra::optional_value<::enums::minimal_val> enumbra::from_string<::enums::minimal_val>(const char* str, int len) noexcept {
if(len != 1) { return {}; }
constexpr int offset_str = 0;
constexpr int offset_enum = 0;
constexpr int count = 2;
for (int i = 0; i < count; i++) {
if (::enumbra::detail::streq_fixed_size<1>(::enums::detail::minimal_val::enum_strings + offset_str + (i * (len + 1)), str)) {
return ::enumbra::optional_value<::enums::minimal_val>(::enums::detail::minimal_val::values_arr[offset_enum + i]);
}
}
return {};
}

template<>
constexpr ::enumbra::optional_value<::enums::minimal_val> enumbra::from_string<::enums::minimal_val>(const char* str) noexcept {
const int len = ::enumbra::detail::strlen(str);
return ::enumbra::from_string<::enums::minimal_val>(str, len);
}


namespace enums {
enum class big : unsigned long long {
B = 0x12D687,
C = 0x54F9338,
};
}

template<> struct enumbra::detail::base_helper<::enums::big> : enumbra::detail::type_info<true, true, false> { };
template<> struct enumbra::detail::enum_helper<::enums::big> : enumbra::detail::enum_info<unsigned long long, 0x12D687, 0x54F9338, 0x12D687, 2, false, 27, 27, true, 0> { };

namespace enums::detail::big {
constexpr ::enums::big values_arr[2] =
{
::enums::big::B,
::enums::big::C,
};
constexpr const char enum_strings[5] = {
"B\0"
"C\0"
};
}

template<>
constexpr auto& enumbra::values<::enums::big>() noexcept
{
return ::enums::detail::big::values_arr;
}

template<>
constexpr ::enumbra::optional_value<::enums::big> enumbra::from_integer<::enums::big>(unsigned long long v) noexcept { 
for(auto value : values<::enums::big>()) {
if(value == static_cast<::enums::big>(v)) { return ::enumbra::optional_value<::enums::big>(static_cast<::enums::big>(v)); }
}
return {};
}

template<>
constexpr bool ::enumbra::is_valid<::enums::big>(::enums::big e) noexcept { 
for(auto value : values<::enums::big>()) {
if(value == e) { return true; }
}
return false;
}

template<>
constexpr ::enumbra::string_view enumbra::enum_name<::enums::big>() noexcept { 
return { "big", 3 };
}

template<>
constexpr ::enumbra::string_view enumbra::enum_name_with_namespace<::enums::big>() noexcept { 
return { "enums::big", 10 };
}

template<>
constexpr ::enumbra::string_view enumbra::enum_namespace<::enums::big>() noexcept { 
return { "enums", 5 };
}

namespace enumbra {
constexpr ::enumbra::string_view to_string(const ::enums::big v) noexcept {
switch (v) {
case ::enums::big::B: return { &::enums::detail::big::enum_strings[0], 1 };
case ::enums::big::C: return { &::enums::detail::big::enum_strings[2], 1 };
}
return { nullptr, 0 };
}
}

template<>
constexpr ::enumbra::optional_value<::enums::big> enumbra::from_string<::enums::big>(const char* str, int len) noexcept {
if(len != 1) { return {}; }
constexpr int offset_str = 0;
constexpr int offset_enum = 0;
constexpr int count = 2;
for (int i = 0; i < count; i++) {
if (::enumbra::detail::streq_fixed_size<1>(::enums::detail::big::enum_strings + offset_str + (i * (len + 1)), str)) {
return ::enumbra::optional_value<::enums::big>(::enums::detail::big::values_arr[offset_enum + i]);
}
}
return {};
}

template<>
constexpr ::enumbra::optional_value<::enums::big> enumbra::from_string<::enums::big>(const char* str) noexcept {
const int len = ::enumbra::detail::strlen(str);
return ::enumbra::from_string<::enums::big>(str, len);
}



namespace enums {
enum class minimal : unsigned int {
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

template<>
constexpr bool is_valid<::enums::minimal>(::enums::minimal e) noexcept { 
return (static_cast<unsigned int>(e) | static_cast<unsigned int>(0x3)) == static_cast<unsigned int>(0x3);
}

template<> constexpr void clear(::enums::minimal& value) noexcept { value = static_cast<::enums::minimal>(0); }
template<> constexpr bool test(::enums::minimal value, ::enums::minimal flags) noexcept { return (static_cast<unsigned int>(flags) & static_cast<unsigned int>(value)) == static_cast<unsigned int>(flags); }
template<> constexpr void set(::enums::minimal& value, ::enums::minimal flags) noexcept { value = static_cast<::enums::minimal>(static_cast<unsigned int>(value) | static_cast<unsigned int>(flags)); }
template<> constexpr void unset(::enums::minimal& value, ::enums::minimal flags) noexcept { value = static_cast<::enums::minimal>(static_cast<unsigned int>(value) & (~static_cast<unsigned int>(flags))); }
template<> constexpr void toggle(::enums::minimal& value, ::enums::minimal flags) noexcept { value = static_cast<::enums::minimal>(static_cast<unsigned int>(value) ^ static_cast<unsigned int>(flags)); }
template<> constexpr bool has_all(::enums::minimal value) noexcept { return (static_cast<unsigned int>(value) & static_cast<unsigned int>(0x3)) == static_cast<unsigned int>(0x3); }
template<> constexpr bool has_any(::enums::minimal value) noexcept { return (static_cast<unsigned int>(value) & static_cast<unsigned int>(0x3)) > 0; }
template<> constexpr bool has_none(::enums::minimal value) noexcept { return (static_cast<unsigned int>(value) & static_cast<unsigned int>(0x3)) == 0; }
template<> constexpr bool has_single(::enums::minimal value) noexcept { unsigned int n = static_cast<unsigned int>(static_cast<unsigned int>(value) & 0x3); return n && !(n & (n - 1)); }

} // namespace enumbra

template<> struct enumbra::detail::base_helper<::enums::minimal> : enumbra::detail::type_info<true, false, true> { };
template<> struct enumbra::detail::enum_helper<::enums::minimal> : enumbra::detail::enum_info<unsigned int, 0, 3, 0, 2, true, 2, 2, false, 0> { };

namespace enums {
constexpr ::enums::minimal operator~(const ::enums::minimal a) noexcept { return static_cast<::enums::minimal>(~static_cast<unsigned int>(a)); }
constexpr ::enums::minimal operator|(const ::enums::minimal a, const ::enums::minimal b) noexcept { return static_cast<::enums::minimal>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b)); }
constexpr ::enums::minimal operator&(const ::enums::minimal a, const ::enums::minimal b) noexcept { return static_cast<::enums::minimal>(static_cast<unsigned int>(a) & static_cast<unsigned int>(b)); }
constexpr ::enums::minimal operator^(const ::enums::minimal a, const ::enums::minimal b) noexcept { return static_cast<::enums::minimal>(static_cast<unsigned int>(a) ^ static_cast<unsigned int>(b)); }
constexpr ::enums::minimal& operator|=(::enums::minimal& a, const ::enums::minimal b) noexcept { return a = a | b; }
constexpr ::enums::minimal& operator&=(::enums::minimal& a, const ::enums::minimal b) noexcept { return a = a & b; }
constexpr ::enums::minimal& operator^=(::enums::minimal& a, const ::enums::minimal b) noexcept { return a = a ^ b; }
} // namespace enums

namespace enumbra {

constexpr ::enumbra::stack_string<12> to_string(const ::enums::minimal v) noexcept {
::enumbra::stack_string<12> output;
if (static_cast<unsigned int>(v & ::enums::minimal::B) > 0) {
output.append<1>("B");
}
if (static_cast<unsigned int>(v & ::enums::minimal::C) > 0) {
if (!output.empty()) { output.append('|'); }
output.append<1>("C");
}
return output;
}

template<>
constexpr ::enumbra::optional_value<::enums::minimal> from_string<::enums::minimal>(const char* str, int len) noexcept {
if (len < 0) { return {}; } // Invalid size
const char* start = str;
const char* end = start;
::enums::minimal output = {};
for (int i = 0; i < len; ++i) {
if (str[i] == '\0') { return {}; } // Invalid: null in string
end++;
if ((i == (len - 1)) || (*end == '|')) {
const auto check_len = end - start;
if (check_len == 1) {
if (::enumbra::detail::streq_fixed_size<1>(start, "B")) { output |= ::enums::minimal::B; }
else if (::enumbra::detail::streq_fixed_size<1>(start, "C")) { output |= ::enums::minimal::C; }
else { return {}; }
}
else { return {}; }
start = end + 1;
}
}
return ::enumbra::optional_value<::enums::minimal>(output);
}

template<>
constexpr ::enumbra::optional_value<::enums::minimal> from_string<::enums::minimal>(const char* str) noexcept {
    const int len = ::enumbra::detail::strlen(str);
    return ::enumbra::from_string<::enums::minimal>(str, len);
}
} // namespace enumbra

#endif // ENUMBRA_8DFC7B86EEA68504_H
