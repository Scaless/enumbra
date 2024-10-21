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

// Type Checks
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


//using namespace enumbra;

#if ENUMBRA_CPP_VERSION >= 20
struct Struct20
{
	// Correct usage
	ENUMBRA_PACK_INIT(enums::test_nodefault, A, enums::test_nodefault::B | enums::test_nodefault::C)
	ENUMBRA_PACK_INIT(enums::test_nodefault, B, enumbra::default_value<enums::test_nodefault>())
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

	{
		constexpr test_nodefault cz{};
		static_assert(enumbra::has_none(cz));
		static_assert(!enumbra::has_single(cz));
		static_assert(!enumbra::has_any(cz));
		static_assert(!enumbra::has_all(cz));
		static_assert(enumbra::is_valid(cz));
	}
	{
		constexpr test_nodefault cz = test_nodefault::B;
		static_assert(!enumbra::has_none(cz));
		static_assert(enumbra::has_single(cz));
		static_assert(enumbra::has_any(cz));
		static_assert(!enumbra::has_all(cz));
		static_assert(enumbra::is_valid(cz));
	}
	{
		constexpr test_nodefault cz = test_nodefault::B | test_nodefault::C;
		static_assert(!enumbra::has_none(cz));
		static_assert(!enumbra::has_single(cz));
		static_assert(enumbra::has_any(cz));
		static_assert(enumbra::has_all(cz));
		static_assert(enumbra::is_valid(cz));
	}
	{
		// Invalid
		constexpr test_nodefault cz =  enumbra::from_integer_unsafe<test_nodefault>(4);
		static_assert(enumbra::has_none(cz));
		static_assert(!enumbra::has_single(cz));
		static_assert(!enumbra::has_any(cz));
		static_assert(!enumbra::has_all(cz));
		static_assert(!enumbra::is_valid(cz));
	}
	

    TestSingleFlag zz{};
    TestSingleFlag z{};
	const bool q = z == zz;
	UNUSED(q);
	

	z |= TestSingleFlag::C;
	z = z | zz;

	// Large integers
	{
		constexpr Unsigned64Test Max = Unsigned64Test::MAX;
        static_assert(enumbra::to_underlying(Max) == UINT64_MAX);
		UNUSED(Max);

		constexpr auto m = enumbra::min<HexDiagonal>();
		UNUSED(m);

		{
			constexpr auto NANFail = enumbra::from_string<Unsigned64Test>("NAN", 3);
			static_assert(NANFail.has_value() == false);
			UNUSED(NANFail);
		}
		{
			constexpr auto NANFail = enumbra::from_string<Unsigned64Test>("NAN");
			static_assert(NANFail.has_value() == false);
			UNUSED(NANFail);
		}
		{
			constexpr auto Hex = enumbra::from_string<HexDiagonal>("NORTH");
			static_assert(Hex.has_value());
			UNUSED(Hex);
		}

		constexpr auto MAXSuccess = enumbra::from_string<Unsigned64Test>("V_UINT32_MAX", 12);
        static_assert(MAXSuccess.has_value() == true);
        UNUSED(MAXSuccess);

        constexpr auto SP = enumbra::from_string<test_string_parse>("C", 1);
        static_assert(SP.has_value() == true);
        UNUSED(SP);

        constexpr auto SPN = enumbra::from_string<test_string_parse>("EEE", 3);
        static_assert(SPN.has_value() == false);
        UNUSED(SPN);
	}

	auto& arr = enumbra::values<test_string_parse>();

	// Range-for
	for (auto& key : arr)
	{
		const auto str = enumbra::to_string(key);
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

        constexpr auto result = enumbra::from_integer<test_string_parse>(raw);
		static_assert(result);
		static_assert(result.has_value());
        static_assert(result.value() == test_string_parse::B);

        constexpr auto result2 = enumbra::from_integer<test_string_parse>(raw2);
		static_assert(!result2);
		static_assert(result2.has_value() == false);
        UNUSED(result2);
	}
	{
		constexpr uint8_t raw = 0;
		constexpr auto valid = enumbra::from_integer<HexDiagonal>(raw);
        static_assert(valid);
		UNUSED(valid);

		constexpr uint8_t raw2 = 6;
		constexpr auto valid2 = enumbra::from_integer<HexDiagonal>(raw2);
        static_assert(valid2.has_value() == false);
		UNUSED(valid2);

		HexDiagonal hd = enumbra::from_integer_unsafe<HexDiagonal>(raw2);
		UNUSED(hd);
	}

	{
		struct q_t {
			ENUMBRA_PACK_UNINITIALIZED(test_flags, flags)
		};
		q_t qt{};
		ENUMBRA_CLEAR(qt.flags)
		ENUMBRA_SET(qt.flags, test_flags::B)
		ENUMBRA_TOGGLE(qt.flags, test_flags::B)
		ENUMBRA_UNSET(qt.flags, test_flags::B)

		test_flags tf = enumbra::default_value<test_flags>();
		ENUMBRA_CLEAR(tf)
		ENUMBRA_SET(tf, test_flags::C)
		ENUMBRA_TOGGLE(tf, test_flags::B)
		ENUMBRA_UNSET(tf, test_flags::B)

		enumbra::clear(tf);
		enumbra::set(tf, test_flags::C);
		enumbra::toggle(tf, test_flags::B);
		enumbra::unset(tf, test_flags::B);
	}

	{
		test_string_parse zzz = enumbra::from_integer_unsafe<test_string_parse>(-1);
		test_string_parse eee = enumbra::from_integer_unsafe<test_string_parse>(1);
		zzz = eee;
        UNUSED(zzz);

		auto ccc = enumbra::to_underlying(eee);
		UNUSED(ccc);
	}

}
