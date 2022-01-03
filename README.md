enumbra is a code generator for enums.

# Getting Started
Please refer to the [wiki](https://github.com/Scaless/enumbra/wiki) for full documentation.

Annotated TOML config files are provided in the `examples` directory to get started.
```
./enumbra.exe -c enumbra_config.toml -s enum_config.toml --cppout enumbra_test.hpp
```
An example of C++ generated output is at [examples/enumbra_test.hpp](/examples/enumbra_test.hpp).

# Generators

### CPP
Generated code requires a minimum of C++11.  
Additional features are optionally enabled up to C++20.  

Currently, `<array>` is the only fully required header in the generated output.  
In the default enumbra_config, `<cstdint>` is also included. This can be overridden in your enumbra_config by specifying your own types.  

All generated headers are self-sufficient, just drop them into your project and include them.

### Other Languages
TBD, C# Planned

# Types
enumbra provides two core types of enums: Value Enum and Flags Enum.  

| Enum | State | Bitwise Ops | Bit Packing |
| --- | --- | --- | --- |
| Value | Single Only | No | Yes |
| Flags | None / Single / Multiple | Yes | Yes |

An typical standard C++ enum for each type would look like: 

```
// Value Enum
enum class ENetworkStatus : uint8_t { Disconnected = 0, WaitingForServer = 1, Connected = 2, Dropped = 3 }
// Flags Enum
enum class FDirectionFlags : uint8_t { North = 1, East = 2, South = 4, West = 8 }
```

# Building
This section refers to building enumbra itself, not the generated code. See the Generators section for generated code requirements.

enumbra requires a C++17 compiler and is primarily tested to build on Windows with Visual Studio 2019/2022. 

If you are on another OS/compiler and would like to add native support, open an issue/PR. I suck at cmake so don't expect any help.

enumbra uses vcpkg manifests for a couple of dependencies. Modify CMakeSettings.json and set cmakeToolchain to point to your vcpkg installation.

# Limitations
1. TOML integers are represented by INT64, therefore values greater than INT64_MAX cannot be represented currently. The plan is to eventually support an optional string format for values not representable by INT64. Until then, the toml parser should give you one of the following warnings:
	* Error while parsing decimal integer: '9446744073709551615' is not representable in 64 bits
	* Error while parsing decimal integer: exceeds maximum length of 19 characters
2. Flags Enums are required to have an unsigned underlying type.
3. Values within Flags Enums may not span multiple bits. A single value must control a single bit.
4. All Enums require unique values.

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

* For large enums, constexpr generation is slow and cumbersome on compile times / memory.
* The number of constants within a single enum is usually limited to around 128 due to compiler limits.
* Lack of configuration options.
* Limited to one programming language.
* The provided `bitwise_operators` namespace lets you use bitwise operators on ALL enums regardless of if they are inteded to be flags or not.
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

The one advantage std::bitset has is that it can represent state with more than 64 bits at once.
