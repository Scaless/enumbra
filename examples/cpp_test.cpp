#include "enumbra_test.hpp"

#include <type_traits>

#define UNUSED(x) (void)x

enum NonEnumbraEnum
{
	A = 4,
	B = 5
};

enum class NonEnumbraEnumClass
{
	A = 4,
	B = 5
};

// Make sure macros work while without full namespace
static void TestMacroWithUsingNamespace()
{
	using namespace enums;
	struct NSV
	{
		ENUMBRA_PACK_UNINITIALIZED(test_nodefault, W);
		ENUMBRA_PACK_UNINITIALIZED(test_nodefault, X);
		ENUMBRA_PACK_UNINITIALIZED(test_nodefault, Y);
		ENUMBRA_PACK_UNINITIALIZED(test_nodefault, Z);

		NSV()
			: ENUMBRA_INIT_DEFAULT(W)
			, ENUMBRA_INIT(X, test_nodefault::B)
			, Y(test_nodefault())
			, Z(test_nodefault())
		{ }
	};
	NSV v;
	UNUSED(v);
}

// Verify bitfield packing, clang/gcc/msvc *should* pack these, other compilers might not
struct PackedStruct
{
	ENUMBRA_PACK_UNINITIALIZED(enums::test_nodefault, W);
	ENUMBRA_PACK_UNINITIALIZED(enums::test_nodefault, X);
	ENUMBRA_PACK_UNINITIALIZED(enums::test_nodefault, Y);
	ENUMBRA_PACK_UNINITIALIZED(enums::test_nodefault, Z);

	PackedStruct() :
		ENUMBRA_INIT_DEFAULT(W),
		ENUMBRA_INIT(X, test_nodefault::B),
		Y(enums::test_nodefault()),
		Z(enums::test_nodefault())
	{ }
};
static_assert(sizeof(PackedStruct) == 2, "PackedStruct bitfield packing failed");

// Type Checks
static_assert(enumbra::is_enumbra_enum<NonEnumbraEnum> == false, "is_enumbra_enum failed");
static_assert(enumbra::is_enumbra_flags_enum<NonEnumbraEnum> == false, "is_enumbra_flags_enum failed");
static_assert(enumbra::is_enumbra_value_enum<NonEnumbraEnum> == false, "is_enumbra_value_enum failed");

static_assert(enumbra::is_enumbra_enum<NonEnumbraEnumClass> == false, "is_enumbra_enum failed");
static_assert(enumbra::is_enumbra_flags_enum<NonEnumbraEnumClass> == false, "is_enumbra_flags_enum failed");
static_assert(enumbra::is_enumbra_value_enum<NonEnumbraEnumClass> == false, "is_enumbra_value_enum failed");

static_assert(enumbra::is_enumbra_enum<enums::Signed8Test> == true, "is_enumbra_enum failed");
static_assert(enumbra::is_enumbra_flags_enum<enums::Signed8Test> == false, "is_enumbra_flags_enum failed");
static_assert(enumbra::is_enumbra_value_enum<enums::Signed8Test> == true, "is_enumbra_value_enum failed");

static_assert(enumbra::is_enumbra_enum<enums::TestSingleFlag> == true, "is_enumbra_enum failed");
static_assert(enumbra::is_enumbra_flags_enum<enums::TestSingleFlag> == true, "is_enumbra_flags_enum failed");
static_assert(enumbra::is_enumbra_value_enum<enums::TestSingleFlag> == false, "is_enumbra_value_enum failed");

static_assert(std::is_enum_v<NonEnumbraEnum> == true, "std::is_enum_v failed");
static_assert(std::is_enum_v<NonEnumbraEnumClass> == true, "std::is_enum_v failed");
static_assert(std::is_enum_v<enums::Signed8Test> == true, "std::is_enum_v failed");
static_assert(std::is_enum_v<enums::TestSingleFlag> == true, "std::is_enum_v failed");

#if ENUMBRA_CPP_VERSION >= 23
static_assert(std::is_scoped_enum_v<NonEnumbraEnum> == false, "std::is_scoped_enum_v failed");
static_assert(std::is_scoped_enum_v<NonEnumbraEnumClass> == true, "std::is_scoped_enum_v failed");
static_assert(std::is_scoped_enum_v<enums::Signed8Test> == true, "std::is_scoped_enum_v failed");
static_assert(std::is_scoped_enum_v<enums::TestSingleFlag> == true, "std::is_scoped_enum_v failed");
#endif

#if ENUMBRA_CPP_VERSION >= 20
struct StructWithPackedBitfields
{
	// Correct usage
	ENUMBRA_PACK_INIT(enums::test_nodefault, A, enums::test_nodefault::B | enums::test_nodefault::C);
	ENUMBRA_PACK_INIT(enums::test_nodefault, B, enumbra::default_value<enums::test_nodefault>());
	ENUMBRA_PACK_INIT_DEFAULT(enums::test_nodefault, C);
	ENUMBRA_PACK_UNINITIALIZED(enums::test_nodefault, D);

	ENUMBRA_PACK_INIT_DEFAULT(enums::test_nodefault, E);
	ENUMBRA_PACK_INIT_DEFAULT(enums::test_nodefault, F);
	ENUMBRA_PACK_INIT_DEFAULT(enums::test_nodefault, G);
	ENUMBRA_PACK_INIT_DEFAULT(enums::test_nodefault, H);

	ENUMBRA_PACK_INIT_DEFAULT(enums::HexDiagonal, Hex1);
	ENUMBRA_PACK_INIT_DEFAULT(enums::HexDiagonal, Hex2);
	ENUMBRA_PACK_INIT_DEFAULT(enums::HexDiagonal, Hex3);
	ENUMBRA_PACK_INIT_DEFAULT(enums::HexDiagonal, Hex4);

	StructWithPackedBitfields() : ENUMBRA_INIT_DEFAULT(D) {}
};
static_assert(sizeof(StructWithPackedBitfields) == 4, "packing issue");
#endif

static void TestBitOps()
{
	using namespace enums;

	test_nodefault b = test_nodefault::B;
	test_nodefault c = test_nodefault::C;

	b = test_nodefault::B | test_nodefault::C;
	b = test_nodefault::B & test_nodefault::C;
	b = test_nodefault::B ^ test_nodefault::C;

	b |= test_nodefault::B;
	b &= test_nodefault::B;
	b ^= test_nodefault::B;

	b |= test_nodefault::B | test_nodefault::C;
	b &= test_nodefault::B & test_nodefault::C;
	b ^= test_nodefault::B ^ test_nodefault::C;

	b = ~b;
	b = ~test_nodefault::B;

	b = b | c;
	b = b & c;
	b = b ^ c;

	b = b | test_nodefault::B;
	b = b & test_nodefault::B;
	b = b ^ test_nodefault::B;

	b = test_nodefault::B | b;
	b = test_nodefault::B & b;
	b = test_nodefault::B ^ b;
}

static constexpr bool TestFlagsSwitch()
{
	using enums::TestSparseFlags;

	int callCount = 0;
	int count = 0;
	TestSparseFlags b = TestSparseFlags::B | TestSparseFlags::D;

	// Lambda is called once per matching flag, in this case 2 times for B and D
	enumbra::flags_switch(b, [&](auto val) {
		switch (val) {
		case TestSparseFlags::B: count += 1; break;
		case TestSparseFlags::C: count += 2; break;
		case TestSparseFlags::D: count += 3; break;
		}
		callCount++;
	});

	// Switch executes once per matching flag, in this case 2 times for B and D
	ENUMBRA_FLAGS_SWITCH_BEGIN(b) {
		case TestSparseFlags::B: count += 1; break;
		case TestSparseFlags::C: count += 2; break;
		case TestSparseFlags::D: count += 3; break;
	} ENUMBRA_FLAGS_SWITCH_END;

	return (callCount == 2) && (count == 8);
}

static void TestPackedBitfields()
{
	using enums::test_nodefault;

	PackedStruct v;
	test_nodefault d = test_nodefault::B;
	v.X = test_nodefault::B;
	v.Y = test_nodefault::B | test_nodefault::C;
	v.Z = d;
	v.X = v.X | d;
	d = v.X;
}

static void TestBoolConversions()
{
	using namespace enums;

	{
		test_nodefault d = test_nodefault::B;
		test_nodefault c = test_nodefault::C;
		bool b = false;
		b = (d == c);
		b = (d != c);
		b = (d == test_nodefault::B);
		b = (d != test_nodefault::B);
		b = (test_nodefault::B == d);
		b = (test_nodefault::B != d);
		d = b ? d : c;
	}

	{
		HexDiagonal f{};
		HexDiagonal g{};
		bool b = false;
		b = (f == g);
		b = (f != g);
		b = (f == HexDiagonal::NORTH);
		b = (f != HexDiagonal::NORTH);
		b = (HexDiagonal::NORTH == f);
		b = (HexDiagonal::NORTH != f);
		f = b ? f : g;
	}
}

static void TestFlagsFunctions()
{
	using namespace enums;

	{
		constexpr test_nodefault cz{};
		static_assert(enumbra::has_none(cz), "failed");
		static_assert(!enumbra::has_single(cz), "failed");
		static_assert(!enumbra::has_any(cz), "failed");
		static_assert(!enumbra::has_all(cz), "failed");
		static_assert(enumbra::is_valid(cz), "failed");
	}
	{
		constexpr test_nodefault cz = test_nodefault::B;
		static_assert(!enumbra::has_none(cz), "failed");
		static_assert(enumbra::has_single(cz), "failed");
		static_assert(enumbra::has_any(cz), "failed");
		static_assert(!enumbra::has_all(cz), "failed");
		static_assert(enumbra::is_valid(cz), "failed");
	}
	{
		constexpr test_nodefault cz = test_nodefault::B | test_nodefault::C;
		static_assert(!enumbra::has_none(cz), "failed");
		static_assert(!enumbra::has_single(cz), "failed");
		static_assert(enumbra::has_any(cz), "failed");
		static_assert(enumbra::has_all(cz), "failed");
		static_assert(enumbra::is_valid(cz), "failed");
	}
	{
		// Invalid
		constexpr test_nodefault cz = enumbra::from_integer_unsafe<test_nodefault>(4);
		static_assert(enumbra::has_none(cz), "failed");
		static_assert(!enumbra::has_single(cz), "failed");
		static_assert(!enumbra::has_any(cz), "failed");
		static_assert(!enumbra::has_all(cz), "failed");
		static_assert(!enumbra::is_valid(cz), "failed");
	}
}

static void TestStringConversions()
{
	using namespace enums;

	constexpr Unsigned64Test Max = Unsigned64Test::MAX;
	static_assert(enumbra::to_underlying(Max) == UINT64_MAX, "failed");
	UNUSED(Max);

	constexpr auto m = enumbra::min<HexDiagonal>();
	UNUSED(m);

	{
		constexpr auto NANFail = enumbra::from_string<Unsigned64Test>("NAN", 3);
		static_assert(NANFail.has_value() == false, "failed");
		UNUSED(NANFail);
	}
	{
		constexpr auto NANFail = enumbra::from_string<Unsigned64Test>("NAN");
		static_assert(NANFail.has_value() == false, "failed");
		UNUSED(NANFail);
	}
	{
		constexpr auto Hex = enumbra::from_string<HexDiagonal>("NORTH");
		static_assert(Hex.has_value(), "failed");
		static_assert(sizeof(Hex) == sizeof(HexDiagonal), "failed");
		UNUSED(Hex);
	}

	constexpr auto MAXSuccess = enumbra::from_string<Unsigned64Test>("V_UINT32_MAX", 12);
	static_assert(MAXSuccess.has_value() == true, "failed");
	UNUSED(MAXSuccess);

	constexpr auto SP = enumbra::from_string<test_string_parse>("C", 1);
	static_assert(SP.has_value() == true, "failed");
	UNUSED(SP);

	constexpr auto SPN = enumbra::from_string<test_string_parse>("EEE", 3);
	static_assert(SPN.has_value() == false, "failed");
	UNUSED(SPN);
}

static void TestRange()
{
	using namespace enums;

	auto& arr = enumbra::values<test_string_parse>();

	// Range-for
	for (auto& key : arr)
	{
		auto str = enumbra::to_string(key);
		UNUSED(str);

		constexpr auto res = enumbra::to_string(::enums::errc::bad_address);
		static_assert(res.size == 11, "failed");
		static_assert(::enumbra::detail::streq_fixed_size<res.size>(res.str, "bad_address"), "failed");

		constexpr auto x = enumbra::min<test_string_parse>();
		UNUSED(x);
		constexpr auto y = enumbra::max<test_string_parse>();
		UNUSED(y);
	}
}

static void TestIsValid()
{
	using namespace enums;
	
	// is_valid
	{
		constexpr int64_t raw = 1;
		constexpr int64_t raw2 = 0;

		constexpr auto result = enumbra::from_integer<test_string_parse>(raw);
		static_assert(result, "failed");
		static_assert(result.has_value(), "failed");
		static_assert(result.value() == test_string_parse::B, "failed");

		constexpr auto result2 = enumbra::from_integer<test_string_parse>(raw2);
		static_assert(!result2, "failed");
		static_assert(result2.has_value() == false, "failed");
		UNUSED(result2);
	}
	{
		constexpr uint8_t raw = 0;
		constexpr auto valid = enumbra::from_integer<HexDiagonal>(raw);
		static_assert(valid, "failed");
		UNUSED(valid);

		constexpr uint8_t raw2 = 6;
		constexpr auto valid2 = enumbra::from_integer<HexDiagonal>(raw2);
		static_assert(valid2.has_value() == false, "failed");
		UNUSED(valid2);

		HexDiagonal hd = enumbra::from_integer_unsafe<HexDiagonal>(raw2);
		UNUSED(hd);
	}
}

static void TestBitMacros()
{
	using namespace enums;

	// Bit Macros
	{
		struct q_t {
			ENUMBRA_PACK_UNINITIALIZED(test_flags, flags);
		};
		q_t qt{};
		ENUMBRA_CLEAR(qt.flags);
		ENUMBRA_SET(qt.flags, test_flags::B);
		ENUMBRA_TOGGLE(qt.flags, test_flags::B);
		ENUMBRA_UNSET(qt.flags, test_flags::B);

		test_flags tf = enumbra::default_value<test_flags>();
		ENUMBRA_CLEAR(tf);
		ENUMBRA_SET(tf, test_flags::C);
		ENUMBRA_TOGGLE(tf, test_flags::B);
		ENUMBRA_UNSET(tf, test_flags::B);

		if (enumbra::test(tf, test_flags::C))
			ENUMBRA_CLEAR(tf);
		else
			ENUMBRA_TOGGLE(tf, test_flags::B);

		enumbra::clear(tf);
		enumbra::set(tf, test_flags::C);
		enumbra::toggle(tf, test_flags::B);
		enumbra::unset(tf, test_flags::B);
	}
}

static void TestFlagsToString()
{
	using namespace enums;
	
	static_assert(enumbra::to_string(Blorp()).sv().empty(), "");
	static_assert(enumbra::detail::streq_fixed_size<3>(enumbra::to_string(Blorp::big).sv().str, "big"), "");
	static_assert(enumbra::detail::streq_fixed_size<10>(enumbra::to_string(Blorp::big | Blorp::bigger).sv().str, "big|bigger"), "");
	static_assert(enumbra::detail::streq_fixed_size<11>(enumbra::to_string(Blorp::big | Blorp::biggest).sv().str, "big|biggest"), "");
	static_assert(enumbra::detail::streq_fixed_size<14>(enumbra::to_string(Blorp::bigger | Blorp::biggest).sv().str, "bigger|biggest"), "");
	static_assert(enumbra::detail::streq_fixed_size<18>(enumbra::to_string(enumbra::max<Blorp>()).sv().str, "big|bigger|biggest"), "");
}

int main()
{
	TestMacroWithUsingNamespace();
	TestBitOps();
	TestFlagsSwitch();
	TestPackedBitfields();
	TestBoolConversions();
	TestFlagsFunctions();
	TestStringConversions();
	TestRange();
	TestIsValid();
	TestBitMacros();
	TestFlagsToString();
}
