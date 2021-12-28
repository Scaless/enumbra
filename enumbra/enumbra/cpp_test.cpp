#include "enumbra_test.hpp"

using namespace enumbra;

struct V
{
    ENUMBRA_PACK(test_nodefault, W);
    ENUMBRA_PACK(test_nodefault, X);
    ENUMBRA_PACK(test_nodefault, Y);
    ENUMBRA_PACK(test_nodefault, Z);
};
static_assert(sizeof(V) == 2);

template<typename T>
void test_templated(const T& value)
{
    static_assert(T::is_enumbra_flags_enum());
}

int main()
{
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
    if (d.is_set(test_nodefault::B | test_nodefault::C)) { }

    // Test packed bitfields
    V v;
    v.X = test_nodefault::B;
    v.Y = test_nodefault::B | test_nodefault::C;
    v.Z = d;

    d = v.X;

    test_nodefault::Value vv{};
    vv = ~vv;
}