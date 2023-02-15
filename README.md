enumbra is a code generator for enums.

# Getting Started
Please refer to the [wiki](https://github.com/Scaless/enumbra/wiki) for full documentation.  

Annotated starter config files are provided in the [examples](/examples/) directory.  
Running the following command will parse the given config files and generate [enumbra_test.hpp](/examples/enumbra_test.hpp).  
```
./enumbra.exe -c enumbra_config.toml -s enum_config.toml --cppout enumbra_test.hpp
```

# Generators

### C++
Generated C++ code requires a minimum of C++11. Additional features are conditionally enabled up to C++20.  

Currently, `<array>` is the only required header in the generated output.  
In the default enumbra_config, `<cstdint>` is also included. This can be overridden in your enumbra_config by specifying your own types.  

Generated headers are self-sufficient, just drop them into your project and include them. There are no master headers or include order issues to worry about.

Including headers generated with different versions of enumbra will throw compiler warnings.  
By default, only major differences in templates, macros, class layouts, and function definitions would generate warnings.

With `ENUMBRA_STRICT_VERSION` defined, all headers must be generated with the exact same enumbra version regardless of if they would be technically compatible. This is useful for making sure your build system is set up to properly regenerate all files.  

### Other Languages
C#: Planned, not started yet.  
C: No plans, but interested.  
C++98/03: No interest. Without the bare minimum C++11 features you may as well just use C.  

# Types
enumbra provides two core types of enums: Value Enum and Flags Enum.  

| Enum Type | State | Bitwise Ops | Bit Packing (C++) |
| --- | --- | --- | --- |
| Value | Single Value | No | Yes |
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

# Known Limitations
1. The configuration file format, TOML, represents integers as an int64, therefore values greater than INT64_MAX cannot be represented currently. The plan is to eventually support an optional string format for values between INT64_MAX and UINT64_MAX. Until then, the toml parser should give you one of the following warnings:
	* Error while parsing decimal integer: '9446744073709551615' is not representable in 64 bits
	* Error while parsing decimal integer: exceeds maximum length of 19 characters
2. Flags Enums are required to have an unsigned underlying type.
3. Values within Flags Enums may not span multiple bits. A single value must control a single bit. (TODO: A separate mechanism for definining multi-bit flags should be added.)
4. Values and names within an enum must be unique.

# Q&A

### Q. How does enumbra compare to [magic_enum](https://github.com/Neargye/magic_enum)/[Better Enums](http://aantron.github.io/better-enums/index.html)?

* For large enums, constexpr generation is slow on compile times / memory.
* The number of constants within an enum is usually limited to around 128 due to compiler limits for macros/templates.
* Lack of configuration options.
* Limited to one programming language.
* In magic_enum, the provided `bitwise_operators` namespace lets you use bitwise operators on ALL enums regardless of if they are intended to be flags or not.
enumbra defines operators for each enum individually, reducing the chance for mistake.
* enumbra generates all of its data before compilation, so it can do some more analysis on the values to provide extra functionality.

Compile-time libraries have greater convenience in their simplicity, just pop the header in and you're done. Use what works best for you.
