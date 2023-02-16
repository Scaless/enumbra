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

struct Errata {
#if ENUMBRA_CPP_VERSION >= 17
	void enum_class_initializer_list() {
		// BAD but I don't think it's possible for us to stop this without disallowing bitfields.
		// enum class is allowed to be initialized from initializer list with no way to override.
		test_nodefault::Value qq{ 4 };
	}
#endif
};

struct V
{
	ENUMBRA_PACK_UNINITIALIZED(test_nodefault, W);
	ENUMBRA_PACK_UNINITIALIZED(test_nodefault, X);
	ENUMBRA_PACK_UNINITIALIZED(test_nodefault, Y);
	ENUMBRA_PACK_UNINITIALIZED(test_nodefault, Z);

	V() :
		ENUMBRA_INIT_DEFAULT(W),
		ENUMBRA_INIT(X, test_nodefault::B),
		Y(test_nodefault::default_value()),
		Z(test_nodefault::default_value())
	{ }
};
STATIC_ASSERT(sizeof(V) == 2);

#if ENUMBRA_CPP_VERSION >= 17
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
		STATIC_ASSERT_MSG(std::false_type::operator(), "T is not an enumbra type.");
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
	ENUMBRA_PACK_INIT(test_nodefault, B, test_nodefault().value());
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

	switch (d.value()) {
		// Possible but error prone
		// Better to use if/elseif with explicit checks
	}

	bool b;
	b = (d == d);
	b = (d != d);
	b = (d == test_nodefault::B);
	b = (d != test_nodefault::B);
	b = (test_nodefault::B == d);
	b = (test_nodefault::B != d);
	d = b ? d : d;

	HexDiagonal f;
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
	v.Z = d.value();
	v.X = v.X | d.value();

	struct D {
		ENUMBRA_PACK_UNINITIALIZED(NegativeTest3, dd);
	};

	D bigD{};
	NegativeTest3::to_string(bigD.dd);

	constexpr NegativeTest3 t3 = NegativeTest3::A;
	NegativeTest3::to_string(t3);
	NegativeTest3::to_string(t3.value());

	constexpr auto res1 = NegativeTest3::from_string(NegativeTest3::to_string(t3));
	STATIC_ASSERT(res1.first);

	constexpr auto res2 = NegativeTest3::from_string("");
	bigD.dd = res2.second;

	d = v.X; // uses implicit constructor

	test_nodefault::Value vv{};
	vv = ~vv;

	TestSingleFlag z;
	const bool q = z == z;
	z |= TestSingleFlag::C;
	z = z | z;

#if ENUMBRA_CPP_VERSION >= 17
	test_is_enumbra_type(d);
	test_is_enumbra_type(d.value());
	test_is_enumbra_type(v.X);
	test_is_enumbra_type(vv);
	test_is_enumbra_type(NonEnumbraEnum::A);
#endif

}
