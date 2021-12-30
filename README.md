# enumbra
A code generator for enums.

#### Running
Display help and list all available options:

`./enumbra.exe -h`

Example generating CPP output file and also printing to the console:

`./enumbra.exe -c enumbra_config.toml -s enum_config.toml --cppout enumbra_test.hpp -p`

# Generators

#### CPP
Generated code requires a minimum of C++11. 
Currently, `<array>` is the only fully required header in the generated output. 
There are no extra setup requirements, just drop the generated header(s) into your project.

In the default enumbra_config, `<cstdint>` is included. These can be overridden in your enumbra config by specifying your own types.

A warning to Visual Studio users: Iterating on enumbra generated output while the file is open in VS will massively inflate your `.vs` directory. 
Close VS while making changes or delete your `.vs` directory regularly.

TODO: Investigating replacing std::array with plain C-style arrays to be fully free of the STL.

#### Other Languages
TBD

# Examples
Example config files are provided as enumbra_config.toml and enum_config.toml.

#### Value Enum and Flags Enum
A problem with using standard enums as flags is that there is no protection against using bitwise operations against an enum that should be a set of values. 
To solve this, enumbra provides two enum types: Value Enum and Flags Enum. The names should give you a hint as to how they operate.

##### Value Enum
A value enum is just a list of possible single-state values.
A standard C++ value enum would look like: 

`enum class ENetworkStatus : uint8_t { Disconnected = 0, WaitingForServer = 1, Connected = 2 }`

It doesn't make sense for multiple of these values to be set at the same time. Attempting to use bitwise ops on a value enum will result in a compile error.

##### Flags Enum
A flags enum is storage for multiple possible flag values where each flag is toggleable on its own. 
A standard C++ flags enum would look like: 

`enum class EDirectionFlags : uint8_t { North = 1, East = 2, South = 4, West = 8 }`

Say we're making an adventure game and want to store the possible directions available to the player:
```
EDirectionFlags possible_directions = EDirectionFlags::North | EDirectionFlags::West;
possible_directions.unset(EDirectionFlags::North)
```

##### Packed Bit Field Enums
Both Value Enums and Flag Enums can be packed more tightly within a struct:
```
struct Packed
{
    // We are using the EDirectionFlags enum from above.
    // ENUMBRA_PACK macro will expand to:
    // EDirectionFlags::Value Player1 : EDirectionFlags::bits_required_storage();
    ENUMBRA_PACK(EDirectionFlags, Player1);
    ENUMBRA_PACK(EDirectionFlags, Player2);
    ENUMBRA_PACK(EDirectionFlags, Player3);
    ENUMBRA_PACK(EDirectionFlags, Player4);
};
static_assert(sizeof(Packed) == 2); // passes, each enum requires 4 bits and the underlying type is uint8_t
```

All of the general rules of [C++ bit fields](https://en.cppreference.com/w/cpp/language/bit_field) still apply:
* Their layout is implementation defined and non-portable, so do not transfer them over the network or serialize without a conversion method.
* The underlying type of an enum determines the minimum storage, padding, and alignment. Adjacent bit fields with mixed underlying types cannot share storage.
* The `|=`,`&=`, and `^=` operators require returning a non-const reference which cannot be done with bit fields. You can still assign using a temporary like so: `V.x = V.x | EDirectionFlags::North`.

# Building
This section refers to building enumbra itself, not the generated code. See the Generators section for generated code requirements.

enumbra requires a C++17 compiler and builds on Windows with Visual Studio 2019/2022. 
If you are on another OS/compiler and would like to add native support, open an issue/PR. I suck at cmake so don't expect any help.
enumbra uses vcpkg for a couple of dependencies. Modify CMakeSettings.json to fit your setup, namely set cmakeToolchain to your vcpkg toolchain file.

# Limitations
1. TOML integers are represented by INT64, therefore values greater than INT64_MAX cannot be represented currently. The plan is to eventually support an optional string format for values not representable by INT64. Until then, the toml parser should give you one of the following warnings:
	* Error while parsing decimal integer: '9446744073709551615' is not representable in 64 bits
	* Error while parsing decimal integer: exceeds maximum length of 19 characters
2. Flags Enums are required to have an unsigned underlying type. I'm open to adding a config override to allow signed types to support other use cases but it's not on my radar. The generator code makes assumptions in a few places that flags will be unsigned.
3. All Enums require unique values.

# Q&A
Q. Why is the library called enumbra (pronounced e-num-bruh)?

The word umbra represents a region behind a celestial body where light is obscured. C++ enums sit in that region of the language. They're integers, kind of? They're flags, kind of? They're an *enumeration*, yet not *enumerable*? They are widely used as bit flags but there are few built-in resources for safely using them as such.

The name also just sounds cool.

![umbra](https://www.nasa.gov/sites/default/files/umbra-penumbra.jpg)
[Source: NASA.gov](https://www.nasa.gov/audience/forstudents/k-4/stories/umbra-and-penumbra)

Q. Why not use another library like [magic_enum](https://github.com/Neargye/magic_enum)?

Compile-time libraries like magic_enum rely on compiler hacks to function properly. 
For large enums, constexpr generation is slow and cumbersome on compile times / memory. 
Including the `magic_enum::bitwise_operators` namespace lets you use bitwise operators on ALL enums regardless of if they are flags or not.
Since enumbra pre-generates all its data, compiling is fast and can provide some additional functionality. 
magic_enum has greater convenience in its simplicity, just pop the header in and you're done. 
Use what works best for you.

enumbra provides additional features over standard enums such as default constructor values and introspection of various properties of the enum.

Q. Why not use std::bitset?

Several reasons:
* It's hip to hate on the STL.
* std::bitset is more suited for modifying an abstract number of bits at runtime. Enums are static and don't ever grow or shrink.
* Worse Debug performance due to function calls, bounds checking, and other standard library slowness during runtime. Release-optimized performance is mostly just as good as bit twiddling though.
* Can't be packed into bitfields.
* Size is implementation dependent. A std::bitset containing 16 bits ([godbolt](https://godbolt.org/z/v3vxe9oYf)):
    * GCC & Clang x64: 8 bytes
    * GCC & Clang x86: 4 bytes
    * MSVC x64 and x86: 4 bytes
    * uint16_t: 2 bytes

Conclusion: Wrong tool for the job.

Q. Why are you not using <templates/reflection/language feature>?

The entire reason I made this project is because existing solutions are too complicated, lack the specific features I need, or are not supported on the compilers that I am restricted to. You are free to fork the project and alter the outputs to your liking, or submit a PR. I suggest making an issue on Github first to discuss if it's an appropriate change.
