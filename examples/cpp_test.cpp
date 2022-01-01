#include "enumbra_test.hpp"

using namespace enumbra;
using namespace enums;

#if CPP_TEST_VER >= 17
#define STATIC_ASSERT(x) static_assert(x)
#define STATIC_ASSERT_MSG(x,y) static_assert(x,y)
#else
#define STATIC_ASSERT(x)
#define STATIC_ASSERT_MSG(x,y)
#endif

struct Errata {
#if CPP_TEST_VER >= 17
    void enum_class_initializer_list() {
        // BAD but I don't think it's possible for us to stop this without disallowing bitfields.
        // enum class is allowed to be initialized from initializer list with no way to override.
        test_nodefault::Value qq{ 4 };
    }
#endif
#if CPP_TEST_VER >= 20
    // C++20 will allow us to also do this with brace/equal initializers on bit-fields.
    //   ENUMBRA_PACK(test_nodefault, X) { 4 };
    // This is legitimately useful though:
    //   ENUMBRA_PACK(test_nodefault, Y) { test_nodefault::C };
    //   ENUMBRA_PACK(test_nodefault, Z) = test_nodefault::C;
    // We could stop this by adding a second macro that does initialization and does type checking?
    // The ENUMBRA_PACK #define would then have a ; at the end to prevent accidental initialization.
    //   ENUMBRA_PACK_INIT(test_nodefault, Z, test_nodefault::C);
#endif
};

struct V
{
    ENUMBRA_PACK(test_nodefault, W)
    ENUMBRA_PACK(test_nodefault, X)
    ENUMBRA_PACK(test_nodefault, Y)
    ENUMBRA_PACK(test_nodefault, Z)
};
STATIC_ASSERT(sizeof(V) == 2);

#if CPP_TEST_VER >= 17
template<typename T>
void test_value_vs_flags()
{
    if constexpr (is_enumbra_flags_enum<T>()) {
        STATIC_ASSERT(enumbra_base_t<T>::is_enumbra_flags_enum());
        enumbra_base_t<T> x;
        x.reset_zero(); // Only flags should have this function
    }
    else if constexpr (is_enumbra_value_enum<T>()) {
        STATIC_ASSERT(enumbra_base_t<T>::is_enumbra_value_enum());
    }
    else {
        STATIC_ASSERT_MSG(false, "T is not an enumbra type.");
    }
}

// Parameter is const& so we can pass in all types here, even bitfields
template<typename T>
void test_is_enumbra_type(const T& value)
{
    if constexpr (is_enumbra_struct<T>()) {
        // T is an enumbra class type.
        constexpr auto min = T::min();

        T x = value;
        x.reset_zero();
    }
    else if constexpr (is_enumbra_scoped_enum<T>())
    {
        // T is an 'enum class' contained within an enumbra struct type.
        // T is convertible from its enum class type to its parent struct type with enumbra_base_t
        using base_type = enumbra_base_t<T>;
        constexpr auto min = base_type::min();

        base_type x = value;
        x.reset_zero();
    }
    else if constexpr (std::is_enum_v<T>)
    {
        // TODO: C++23 check is_scoped_enum<T>
        // T is just a regular c++ enum type.
        // T x = T();
    }
    else
    {
        static_assert(false, "T is not an enum class or enumbra type.");
    }
}
#endif

enum class NonEnumbraEnum
{
    A = 0,
    B = 1
};

int main()
{
    constexpr test_nodefault a = test_nodefault::B | test_nodefault::C;
    STATIC_ASSERT(a.test(test_nodefault::B | test_nodefault::C));

    test_nodefault d;

    d = test_nodefault::B;
    d = test_nodefault::B | test_nodefault::C;
    d = test_nodefault::B & test_nodefault::C;
    d = test_nodefault::B ^ test_nodefault::C;
    d |= test_nodefault::B | test_nodefault::C;
    d &= test_nodefault::B & test_nodefault::C;
    d ^= test_nodefault::B ^ test_nodefault::C;
    d |= d;
    d &= d;
    d ^= d;
    d = ~d;
    d = d | d;
    d = d & d;
    d = d ^ d;
    d = d | test_nodefault::B;
    d = d & test_nodefault::B;
    d = d ^ test_nodefault::B;
    d = test_nodefault::B | d;
    d = test_nodefault::B & d;
    d = test_nodefault::B ^ d;

    bool b;
    b = (d == d);
    b = (d != d);
    b = (d == test_nodefault::B);
    b = (d != test_nodefault::B);
    b = (test_nodefault::B == d);
    b = (test_nodefault::B != d);

    // Test packed bitfields
    V v;
    v.X = test_nodefault::B;
    v.Y = test_nodefault::B | test_nodefault::C;
    v.Z = d;
    d = v.X; // uses implicit constructor

    test_nodefault::Value vv{};
    vv = ~vv;

    
#if CPP_TEST_VER >= 17
    test_is_enumbra_type(d);
    test_is_enumbra_type(d.value());
    test_is_enumbra_type(v.X);
    test_is_enumbra_type(vv);
    test_is_enumbra_type(NonEnumbraEnum::A);
#endif
    
}
