# enumbra
A code generator for enums.

### Running
Display help and list all available options:

`./enumbra.exe -h`

Example generating CPP output file and also printing to the console:

`./enumbra.exe -c enumbra_config.toml -s enum_config.toml --cppout enumbra_test.hpp -p`

# Examples
Annotated TOML config files are provided in the `examples` directory.

An example of C++ generated output is at `examples/enumbra_test.hpp`.

# Generators

### CPP
Generated code requires a minimum of C++11. 

Currently, `<array>` is the only fully required header in the generated output.
In the default enumbra_config, `<cstdint>` is also included.
This can be overridden in your enumbra_config by specifying your own types.

There are no extra setup requirements, just drop the generated header(s) into your project.

On MSVC, the generated headers will compile with `/Wall /WX /wd4514`.

On GCC/Clang, the generated headers will compile with `-Wall -Wextra -Wpedantic -Werror`.

A warning to Visual Studio users: Rapidly iterating on enumbra generated output while the file is open in VS will inflate your `.vs` directory up to a maximum of 5GB (by default) because automated PCH files are being constantly generated. 
This cache size can be globally configured in Tools > Options > Text Editor > C/C++ > Advanced > Automatic Precompiled Header Cache Quota.
Otherwise, close VS while making changes or delete your `.vs/v16/ipch/AutoPCH` directory regularly.

### Other Languages
TBD, C# Planned

# Usage
enumbra provides two types of enums: Value Enum and Flags Enum. The names should give you a hint as to how they operate.

### Value Enum
A value enum is just a list of possible single-state values.
It doesn't make sense for multiple of these values to be set at the same time.
Bitwise operations are not provided for these types.

An equivalent standard C++ enum class would look like: 

```
enum class ENetworkStatus : uint8_t { Disconnected = 0, WaitingForServer = 1, Connected = 2 }
enum class ETruthStatus : uint8_t { False, True }
```

### Flags Enum
A flags enum can store multiple possible flag values where each flag is toggleable on its own. 

Say we're making an adventure game and want to store the possible directions available to the player:

```
enum class EDirectionFlags : uint8_t { North = 1, East = 2, South = 4, West = 8 }

EDirectionFlags possible_directions = EDirectionFlags::North | EDirectionFlags::West;

// Make sure user is not cheating and pressing multiple directions at once by using single().
// Then test if the direction is in our possible directions with test().
if (user_input.direction.single() && possible_directions.test(user_input.direction)) {
    // Move the player
}
```

### Packed Bit Field Enums
Both Value Enums and Flag Enums can be packed more tightly within a struct by utilizing bit fields.
Several macros are provided to handle this:

#### ENUMBRA_PACK_UNINITIALIZED(Enum, Name)

Declare an enum. The value is NOT initialized, you must do so through your own constructor.

#### ENUMBRA_PACK_INIT(Enum, Name, InitValue) (REQUIRES C++20)

Declare and initialize an enum with a user-given value.
The type of InitValue is checked at compile-time to make sure it is a valid enumbra type.

#### ENUMBRA_PACK_INIT_DEFAULT(Enum, Name) (REQUIRES C++20)

Declare and initialize an enum with the config-specified default value.

```
struct Packed
{
    // We are using the EDirectionFlags enum from above.
    // ENUMBRA_PACK_UNINITIALIZED does NOT initialize values unlike the struct version.
    // ENUMBRA_PACK_UNINITIALIZED macro will expand to:
    //   EDirectionFlags::Value Player1Directions : EDirectionFlags::bits_required_storage();
    ENUMBRA_PACK_UNINITIALIZED(EDirectionFlags, Player1Directions);
    ENUMBRA_PACK_UNINITIALIZED(EDirectionFlags, Player2Directions);
    ENUMBRA_PACK_UNINITIALIZED(EDirectionFlags, Player3Directions);

    // Constructor to initialize values
    Packed() :
        Player1Directions(HexDiagonal::default_value()), // Initialize with config-defined default value
        Player2Directions(HexDiagonal::SOUTH), // Initialize with fixed value
        Player3Directions() // NOT RECOMMENDED! 
            // This will zero-initialize the value, but zero may not be a valid value for that enum!
    { }
};
static_assert(sizeof(Packed) == 2); // each enum requires 4 bits and the underlying type is uint8_t

// Bit field initializers are also supported on C++20 compilers:
struct PackedInit
{
    ENUMBRA_PACK_INIT(EDirectionFlags, Player1, EDirectionFlags::West);
    ENUMBRA_PACK_INIT(EDirectionFlags, Player2, EDirectionFlags::North | EDirectionFlags::South);
    ENUMBRA_PACK_INIT_DEFAULT(EDirectionFlags, Player3);

    // Constructor not required
};
```

All of the general rules of [C++ bit fields](https://en.cppreference.com/w/cpp/language/bit_field) still apply:
* Their layout is implementation defined and non-portable, so do not transfer them over the network or serialize without a conversion method.
It is implementation defined if bit fields may straddle type boundaries or introduce padding.
* The underlying type of an enum determines the minimum storage, padding, and alignment.
* Adjacent bit fields with differing underlying types may or may not share storage.
* Bit fields are more compact in memory but require extra instructions to pack/unpack. Benchmark your use-case to determine if they are the right choice.
* enumbra specific: The provided `|=`,`&=`, and `^=` operator overloads for flags DO NOT return a reference, since doing so is not possible with bit fields.

# Building
This section refers to building enumbra itself, not the generated code. See the Generators section for generated code requirements.

enumbra requires a C++17 compiler and is primarily tested to build on Windows with Visual Studio 2019/2022. 

If you are on another OS/compiler and would like to add native support, open an issue/PR. I suck at cmake so don't expect any help.

enumbra uses vcpkg manifests for a couple of dependencies. Modify CMakeSettings.json and set cmakeToolchain to point to your vcpkg installation.

# Limitations
1. TOML integers are represented by INT64, therefore values greater than INT64_MAX cannot be represented currently. The plan is to eventually support an optional string format for values not representable by INT64. Until then, the toml parser should give you one of the following warnings:
	* Error while parsing decimal integer: '9446744073709551615' is not representable in 64 bits
	* Error while parsing decimal integer: exceeds maximum length of 19 characters
2. Flags Enums are required to have an unsigned underlying type. I'm open to adding a config override to allow signed types to support other use cases but it's not on my radar. The generator code makes assumptions in a few places that flags will be unsigned.
3. All Enums require unique values.

# Q&A
### Q. Why is the library called enumbra (pronounced e-num-bruh)?

The word umbra represents a region behind a celestial body where light is obscured.
C++ enums sit in that region of the language. They're integers, kind of? 
They're flags, kind of? They're an *enumeration*, yet not *enumerable*? 
They are widely used as bit flags but there are few built-in resources for safely using them as such.

The name also just sounds cool.

![umbra](https://www.nasa.gov/sites/default/files/umbra-penumbra.jpg)
[Source: NASA.gov](https://www.nasa.gov/audience/forstudents/k-4/stories/umbra-and-penumbra)

### Q. Why not use another library like [magic_enum](https://github.com/Neargye/magic_enum)/[Better Enums](http://aantron.github.io/better-enums/index.html)?

* Compile-time libraries like these rely on compiler hacks to function properly. 
* For large enums, constexpr generation is slow and cumbersome on compile times / memory.
* The number of constants within a single enum is usually limited to around 128 due to compiler limits.
* Lack of configuration options.
* magic_enum `to_string()` on flags enum where multiple bits are set returns the bare integer value instead of a meaningful string.
* They provide a `bitwise_operators` namespace lets you use bitwise operators on ALL enums regardless of if they are inteded to be flags or not.
enumbra defines operators each enum type individually, reducing the chance for mistake.
* Since enumbra pre-generates all its data, it can do some more analysis on the values to provide some extra functionality.

Compile-time libraries have greater convenience in their simplicity, just pop the header in and you're done. Use what works best for you.

### Q. Why not use std::bitset?

Several reasons:
* It's hip to hate on the STL.
* std::bitset is more suited for modifying an abstract number of bits at runtime. Enums are static and don't ever grow or shrink.
* Worse Debug performance due to function calls, bounds checking, and other standard library slowness during runtime. Release-optimized performance is mostly just as good as bit twiddling though.
* Can't be packed into bitfields.
* Size is implementation dependent. A std::bitset containing 16 bits will consume a minimum of ([godbolt](https://godbolt.org/z/v3vxe9oYf)):
    * GCC & Clang x64: 8 bytes
    * GCC & Clang x86: 4 bytes
    * MSVC x64 and x86: 4 bytes
