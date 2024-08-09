#include "enumbra_test.hpp"

// Include second header with the same namespace to test that include macros are working
//#include "enumbra_minimal.hpp"

#include <type_traits>

enum class NonEnumbraEnum
{
    A = 4,
    B = 5
};

#define UNUSED(x) (void)x

static void TestMacroWithUsingNamespace()
{
    using namespace enums;
    struct NSV
    {
        ENUMBRA_PACK_UNINITIALIZED(test_nodefault, W)
        ENUMBRA_PACK_UNINITIALIZED(test_nodefault, X)
        ENUMBRA_PACK_UNINITIALIZED(test_nodefault, Y)
        ENUMBRA_PACK_UNINITIALIZED(test_nodefault, Z)

        NSV()
            : ENUMBRA_INIT_DEFAULT(W)
            , ENUMBRA_INIT(X, test_nodefault::B)
            , Y(enums::test_nodefault())
            , Z(enums::test_nodefault())
        { }
    };
    NSV v;
    UNUSED(v);
}

struct PackedStruct
{
	ENUMBRA_PACK_UNINITIALIZED(enums::test_nodefault, W)
	ENUMBRA_PACK_UNINITIALIZED(enums::test_nodefault, X)
	ENUMBRA_PACK_UNINITIALIZED(enums::test_nodefault, Y)
	ENUMBRA_PACK_UNINITIALIZED(enums::test_nodefault, Z)

	PackedStruct() :
		ENUMBRA_INIT_DEFAULT(W),
		ENUMBRA_INIT(X, test_nodefault::B),
		Y(enums::test_nodefault()),
		Z(enums::test_nodefault())
	{ }
};
static_assert(sizeof(PackedStruct) == 2);

static void TestEnumbraTypeChecks()
{
    static_assert(enumbra::is_enumbra_enum<NonEnumbraEnum> == false);
    static_assert(enumbra::is_enumbra_flags_enum<NonEnumbraEnum> == false);
    static_assert(enumbra::is_enumbra_value_enum<NonEnumbraEnum> == false);

    static_assert(enumbra::is_enumbra_enum<enums::Signed8Test> == true);
    static_assert(enumbra::is_enumbra_flags_enum<enums::Signed8Test> == false);
    static_assert(enumbra::is_enumbra_value_enum<enums::Signed8Test> == true);

    static_assert(enumbra::is_enumbra_enum<enums::TestSingleFlag> == true);
    static_assert(enumbra::is_enumbra_flags_enum<enums::TestSingleFlag> == true);
    static_assert(enumbra::is_enumbra_value_enum<enums::TestSingleFlag> == false);

    static_assert(std::is_enum_v<NonEnumbraEnum> == true);
    static_assert(std::is_enum_v<enums::Signed8Test> == true);
    static_assert(std::is_enum_v<enums::TestSingleFlag> == true);
}

using namespace enumbra;


#if ENUMBRA_CPP_VERSION >= 20
struct Struct20
{
	// Correct usage
	ENUMBRA_PACK_INIT(enums::test_nodefault, A, enums::test_nodefault::B | enums::test_nodefault::C)
	ENUMBRA_PACK_INIT(enums::test_nodefault, B, default_value<enums::test_nodefault>())
	ENUMBRA_PACK_INIT_DEFAULT(enums::test_nodefault, C)

	// Not allowed
	// ENUMBRA_PACK_INIT(test_nodefault, Ab, 4)
	// ENUMBRA_PACK_INIT(test_nodefault, B, NonEnumbraEnum::A)
	// ENUMBRA_PACK_INIT(test_nodefault, C, test_flags::B)
};
#endif

int main()
{
    TestMacroWithUsingNamespace();
    TestEnumbraTypeChecks();

    using namespace enums;

	test_nodefault d;
    test_nodefault c = test_nodefault::C;

	d = test_nodefault::B;
	d = test_nodefault::B | test_nodefault::C;
	d = test_nodefault::B & test_nodefault::C;
	d = test_nodefault::B ^ test_nodefault::C;
	d |= test_nodefault::B | test_nodefault::C;
	d &= test_nodefault::B & test_nodefault::C;
	d ^= test_nodefault::B ^ test_nodefault::C;
	d |= test_nodefault::B;
	d &= test_nodefault::B;
	d ^= test_nodefault::B;
	d = ~d;
	d = d | c;
	d = d & c;
	d = d ^ c;
	d = d | test_nodefault::B;
	d = d & test_nodefault::B;
	d = d ^ test_nodefault::B;
	d = test_nodefault::B | d;
	d = test_nodefault::B & d;
	d = test_nodefault::B ^ d;

	switch (d) {
	case test_nodefault::B:
	case test_nodefault::C:
        break;
	}

	bool b;
	b = (d == c);
	b = (d != c);
	b = (d == test_nodefault::B);
	b = (d != test_nodefault::B);
	b = (test_nodefault::B == d);
	b = (test_nodefault::B != d);
	d = b ? d : c;

	HexDiagonal f{};
    HexDiagonal g{};
	b = (f == g);
	b = (f != g);
	b = (f == HexDiagonal::NORTH);
	b = (f != HexDiagonal::NORTH);
	b = (HexDiagonal::NORTH == f);
	b = (HexDiagonal::NORTH != f);
	f = b ? f : g;

	// Test packed bitfields
	PackedStruct v;
	v.X = test_nodefault::B;
	v.Y = test_nodefault::B | test_nodefault::C;
	v.Z = d;
	v.X = v.X | d;

	struct D {
		ENUMBRA_PACK_UNINITIALIZED(NegativeTest3, dd)
	};

#if 0
	D bigD{};
	NegativeTest3::to_string(bigD.dd);

	constexpr NegativeTest3 t3 = NegativeTest3::A;
	NegativeTest3::to_string(t3);
	NegativeTest3::to_string(t3.value());

	constexpr auto res1 = NegativeTest3::from_string(NegativeTest3::to_string(t3));
	static_assert(res1.first);

	constexpr auto res2 = NegativeTest3::from_string("");
	bigD.dd = res2.second;
#endif

	d = v.X; // uses implicit constructor

	test_nodefault vv{};
	vv = ~vv;

    TestSingleFlag zz{};
    TestSingleFlag z{};
	const bool q = z == zz;
	UNUSED(q);

	z |= TestSingleFlag::C;
	z = z | zz;

	// Large integers
	{
		constexpr Unsigned64Test Max = Unsigned64Test::MAX;
        static_assert(to_underlying(Max) == UINT64_MAX);
		UNUSED(Max);

		constexpr auto m = enumbra::min<HexDiagonal>();
		UNUSED(m);

		constexpr auto NANFail = from_string<Unsigned64Test>("NAN", 3);
        static_assert(NANFail.has_value() == false);
        UNUSED(NANFail);

		constexpr auto MAXSuccess = from_string<Unsigned64Test>("V_UINT32_MAX", 12);
        static_assert(MAXSuccess.has_value() == true);
        UNUSED(MAXSuccess);

        constexpr auto SP = from_string<test_string_parse>("C", 1);
        static_assert(SP.has_value() == true);
        UNUSED(SP);

        constexpr auto SPN = from_string<test_string_parse>("EEE", 3);
        static_assert(SPN.has_value() == false);
        UNUSED(SPN);
	}

	auto& arr = values<test_string_parse>();

	// Range-for
	for (auto& key : arr)
	{
		const auto str = to_string(key);
		UNUSED(str);

		constexpr auto x = enumbra::min<test_string_parse>();
		UNUSED(x);
		constexpr auto y = enumbra::max<test_string_parse>();
		UNUSED(y);
	}

	// is_valid
	{
		constexpr int64_t raw = 1;
		constexpr int64_t raw2 = 0;

        constexpr auto result = from_integer<test_string_parse>(raw);
        static_assert(result.success);
        static_assert(result.value == test_string_parse::B);

        constexpr auto result2 = from_integer<test_string_parse>(raw2);
        static_assert(result2.success == false);
        UNUSED(result2);
	}
	{
		constexpr uint8_t raw = 0;
		constexpr auto valid = from_integer<HexDiagonal>(raw);
        static_assert(valid.success);
		UNUSED(valid);

		constexpr uint8_t raw2 = 6;
		constexpr auto valid2 = from_integer<HexDiagonal>(raw2);
        static_assert(valid2.success == false);
		UNUSED(valid2);

		HexDiagonal hd = from_integer_unsafe<HexDiagonal>(raw2);
		UNUSED(hd);
	}

	{
		struct q_t {
			ENUMBRA_PACK_UNINITIALIZED(test_flags, flags)
		};
		q_t qt{};
		ENUMBRA_ZERO(qt.flags)
		ENUMBRA_SET(qt.flags, test_flags::B)
		ENUMBRA_TOGGLE(qt.flags, test_flags::B)
		ENUMBRA_UNSET(qt.flags, test_flags::B)

		test_flags tf = default_value<test_flags>();
		ENUMBRA_ZERO(tf)
		ENUMBRA_SET(tf, test_flags::C)
		ENUMBRA_TOGGLE(tf, test_flags::B)
		ENUMBRA_UNSET(tf, test_flags::B)

		zero(tf);
		set(tf, test_flags::C);
		toggle(tf, test_flags::B);
		unset(tf, test_flags::B);
	}

	{
		test_string_parse zzz = enumbra::from_integer_unsafe<test_string_parse>(-1);
		test_string_parse eee = enumbra::from_integer_unsafe<test_string_parse>(1);
		zzz = eee;
        UNUSED(zzz);

		auto ccc = to_underlying(eee);
		UNUSED(ccc);
	}

}
