
enum class test {
    e = 0
};

template<class e>
class flag_set {
public:

private:
    e flags;
};

namespace enumbra {
    // Returns true if n is a power of 2. Must be greater than 0.
    template<::std::uint64_t n>
    constexpr bool is_pow2() {
        static_assert(n > 0);
        // http://www.graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
        return (n & (n - 1)) == 0;
    }

    // Round up to the closest power of 2 for a number. Must be greater than 0.
    template<::std::uint64_t n>
    constexpr ::std::uint64_t closest_pow2() {
        static_assert(n > 0);
        if (is_pow2<n>()) {
            return n;
        }

        // https://stackoverflow.com/questions/1322510/1322548#1322548
        ::std::uint64_t v = n;
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v |= v >> 32;
        v++;

        return v;
    }

    static constexpr ::std::uint64_t get_mask_bits(::std::uint64_t bits) {
        ::std::uint64_t out = 0;
        for (::std::uint64_t i = 0; i < bits; i++)
        {
            out <<= 1;
            out &= 1;
        }
        return out;
    }

    template<class E, ::std::uint64_t count, class storage_type = ::std::uint64_t>
    class packed_value_array {
        static constexpr ::std::uint64_t bits_per_element = closest_pow2<::enumbra::bits_required_transmission<E>()>();
        static constexpr ::std::uint64_t bits_per_storage_type = sizeof(storage_type) * 8;

        static_assert(::enumbra::is_enumbra_value_enum<E>(), "E must be an enumbra value enum");
        static_assert((sizeof(storage_type) * 8) >= bits_per_element);

    public:
        //packed_value_array(E fill);

        storage_type* data() { return &storage[0]; }
        const storage_type* data() const { return &storage[0]; }

        E get(::std::uint64_t index)
        {
            const auto position = index / sizeof(storage_type);
            const auto offset = index % sizeof(storage_type);
            const auto x = (storage[position] >> (offset / bits_per_element)) & get_mask_bits(bits_per_element);
            return from_integer_unsafe<E>(x);
        }

    private:

        static constexpr ::std::uint64_t calc_packed_array_size() {
            constexpr ::std::uint64_t total_bits = count * bits_per_element;
            static_assert(total_bits > 0, "Invalid number of total_bits required for enum");

            if constexpr ((total_bits % bits_per_storage_type) == 0) {
                return total_bits / bits_per_storage_type;
            }
            else {
                return 1 + (total_bits / bits_per_storage_type);
            }
        }



        static constexpr ::std::uint64_t storage_count = calc_packed_array_size();
        storage_type storage[storage_count] = {};
    };
}
