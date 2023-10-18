#include "enumbra_test.hpp"

using namespace enumbra;
using namespace enums;

#if ENUMBRA_CPP_VERSION >= 17
#define STATIC_ASSERT(x) static_assert(x)
#define STATIC_ASSERT_MSG(x,y) static_assert(x,y)
#else
#define STATIC_ASSERT(x)
#define STATIC_ASSERT_MSG(x,y)
#endif

struct V
{
	ENUMBRA_PACK_UNINITIALIZED(test_nodefault, W);
	ENUMBRA_PACK_UNINITIALIZED(test_nodefault, X);
	ENUMBRA_PACK_UNINITIALIZED(test_nodefault, Y);
	ENUMBRA_PACK_UNINITIALIZED(test_nodefault, Z);

	V() :
		ENUMBRA_INIT_DEFAULT(W),
		ENUMBRA_INIT(X, test_nodefault::B),
		Y(test_nodefault()),
		Z(test_nodefault())
	{ }
};
STATIC_ASSERT(sizeof(V) == 2);

#if ENUMBRA_CPP_VERSION >= 17
template<typename T>
void test_value_vs_flags()
{
	if constexpr (is_enumbra_flags_enum<T>()) {
		T x{};
		//x.reset_zero(); // Only flags should have this function
	}
	else if constexpr (is_enumbra_value_enum<T>()) {
		
	}
	else {
		STATIC_ASSERT_MSG(std::false_type::operator(), "T is not an enumbra type.");
	}
}

// Parameter is const& so we can pass in all types here, even bitfields
template<typename T>
void test_is_enumbra_type(const T& value)
{
	if constexpr (is_enumbra_enum<T>())
	{
		// T is an 'enum class' contained within an enumbra struct type.
		// T is convertible from its enum class type to its parent struct type with enumbra_base_t
		constexpr auto min = enumbra::min<T>();
	}
	else if constexpr (std::is_enum_v<T>)
	{
		// TODO: C++23 check is_scoped_enum<T>
		// T is just a regular c++ enum type.
		// T x = T();
	}
	else
	{
		STATIC_ASSERT_MSG(std::false_type::operator(), "T is not an enum class or enumbra type.");
	}
}
#endif

enum class NonEnumbraEnum
{
	A = 4,
	B = 5
};

#if ENUMBRA_CPP_VERSION >= 20
struct Struct20
{
	// Correct usage
	ENUMBRA_PACK_INIT(test_nodefault, A, test_nodefault::B | test_nodefault::C);
	ENUMBRA_PACK_INIT(test_nodefault, B, default_value<test_nodefault>());
	ENUMBRA_PACK_INIT_DEFAULT(test_nodefault, C);

	// Not allowed
	// ENUMBRA_PACK_INIT(test_nodefault, Ab, 4);
	// ENUMBRA_PACK_INIT(test_nodefault, B, NonEnumbraEnum::A);
	// ENUMBRA_PACK_INIT(test_nodefault, C, test_flags::B);
};
#endif

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
	d |= test_nodefault::B;
	d &= test_nodefault::B;
	d ^= test_nodefault::B;
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

	switch (d) {
	default:break;
	}

	bool b;
	b = (d == d);
	b = (d != d);
	b = (d == test_nodefault::B);
	b = (d != test_nodefault::B);
	b = (test_nodefault::B == d);
	b = (test_nodefault::B != d);
	d = b ? d : d;

	HexDiagonal f{};
	b = (f == f);
	b = (f != f);
	b = (f == HexDiagonal::NORTH);
	b = (f != HexDiagonal::NORTH);
	b = (HexDiagonal::NORTH == f);
	b = (HexDiagonal::NORTH != f);
	f = b ? f : f;

	switch (f) {
	case HexDiagonal::NORTH: break;
	}

	// Test packed bitfields
	V v;
	v.X = test_nodefault::B;
	v.Y = test_nodefault::B | test_nodefault::C;
	v.Z = d;
	v.X = v.X | d;

	struct D {
		ENUMBRA_PACK_UNINITIALIZED(NegativeTest3, dd);
	};

#if 0
	D bigD{};
	NegativeTest3::to_string(bigD.dd);

	constexpr NegativeTest3 t3 = NegativeTest3::A;
	NegativeTest3::to_string(t3);
	NegativeTest3::to_string(t3.value());

	constexpr auto res1 = NegativeTest3::from_string(NegativeTest3::to_string(t3));
	STATIC_ASSERT(res1.first);

	constexpr auto res2 = NegativeTest3::from_string("");
	bigD.dd = res2.second;
#endif

	d = v.X; // uses implicit constructor

	test_nodefault vv{};
	vv = ~vv;

	TestSingleFlag z{};
	const bool q = z == z;
	z |= TestSingleFlag::C;
	z = z | z;

#if ENUMBRA_CPP_VERSION >= 17
	test_is_enumbra_type(d);
	test_is_enumbra_type(v.X);
	test_is_enumbra_type(vv);
	test_is_enumbra_type(NonEnumbraEnum::A);
#endif

	// Large integers
	{
		constexpr Unsigned64Test Max = Unsigned64Test::MAX;
		STATIC_ASSERT(to_underlying(Max) == UINT64_MAX);

		constexpr auto z = enumbra::min<HexDiagonal>();

		constexpr auto MaxFromStringResultFail = from_string<Unsigned64Test>("NAN");
		STATIC_ASSERT(MaxFromStringResultFail.first == false);
	}

	auto arr = values<test_string_parse>();
	
	// Range-for
	for (auto& key : values<test_string_parse>())
	{
		constexpr auto x = enumbra::min<test_string_parse>();
		constexpr auto y = enumbra::max<test_string_parse>();

		const test_nodefault e = test_nodefault::B;

		const test_string_parse tsp = test_string_parse::D;
		auto str = to_string(tsp);
	}

	// is_valid
	{
		constexpr int64_t raw = 1;
		constexpr bool valid = is_valid<test_string_parse>(raw);
		STATIC_ASSERT(valid);

		constexpr int64_t raw2 = 0;
		constexpr bool valid2 = is_valid<test_string_parse>(raw2);
		STATIC_ASSERT(valid2 == false);
	}
	{
		constexpr uint8_t raw = 0;
		constexpr bool valid = is_valid<HexDiagonal>(raw);
		STATIC_ASSERT(valid);

		constexpr uint8_t raw2 = 6;
		constexpr bool valid2 = is_valid<HexDiagonal>(raw2);
		STATIC_ASSERT(valid2 == false);
		HexDiagonal d = from_underlying_unsafe<HexDiagonal>(raw2);
	}

	{
		struct q_t {
			ENUMBRA_PACK_UNINITIALIZED(test_flags, flags);
		};
		q_t q{};
		ENUMBRA_ZERO(q.flags);
		ENUMBRA_SET(q.flags, test_flags::B);
		ENUMBRA_TOGGLE(q.flags, test_flags::B);
		ENUMBRA_UNSET(q.flags, test_flags::B);

		test_flags f = default_value<test_flags>();
		ENUMBRA_ZERO(f);
		ENUMBRA_SET(f, test_flags::C);
		ENUMBRA_TOGGLE(f, test_flags::B);
		ENUMBRA_UNSET(f, test_flags::B);
	}

	{
		test_string_parse zzz = enumbra::from_underlying_unsafe<test_string_parse>(-1);
		test_string_parse eee = enumbra::from_underlying_unsafe<test_string_parse>(1);
		zzz = eee;

		auto ccc = to_underlying(eee);
	}

}
