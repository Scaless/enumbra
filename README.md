# Getting Started

enumbra is a code generator for enums.

**enumbra is in a volatile state of development where syntax and features may change at any time.  
If you need stability, wait until the 1.0 release.**

Please refer to the [wiki](https://github.com/Scaless/enumbra/wiki) for full documentation.  

Annotated starter config files are provided in the [examples](/examples/) directory.  
Running the following command will parse the given config files and generate [enumbra_test.hpp](/examples/enumbra_test.hpp).  
```
./enumbra.exe -c enumbra_config.json -s enum.json --cppout enumbra_test.hpp
```

# Types
enumbra generates two core types of enums: Value Enum and Flags Enum.  

A Value Enum always represents one singular state.  

A Flags enum may represent no state, one state, or multiple states.

| Enum Type | State | Bitwise Ops | Bit Packing (C++) |
| --- | --- | --- | --- |
| Value | Single Value | No | Yes |
| Flags | None / Single / Multiple | Yes | Yes |


# Generators

## C++
Generated C++ code requires a minimum of C++11. Additional features are conditionally enabled up to C++20.  

Currently, `<array>` is the only required header in the generated output.  
In the default enumbra_config, `<cstdint>` is also included. This can be overridden in your enumbra_config by specifying your own types.  

Generated headers are self-sufficient, just drop them into your project and include them. There are no master headers or include order issues to worry about.

Including headers generated with different versions of enumbra will throw compiler warnings.  
By default, only major differences in templates, macros, class layouts, and function definitions would generate warnings.

With `ENUMBRA_STRICT_VERSION` defined, all headers must be generated with the exact same enumbra version regardless of if they would be technically compatible. This is useful for making sure your build system is set up to properly regenerate all files.  

### Examples

#### Value Enum

```c++
// This struct is generated by enumbra
struct Direction {
    enum class Value : uint8_t {
        NORTH = 0,
        EAST = 1,
        SOUTH = 2,
        WEST = 3
    };
    // Rest of struct omitted
}

// Usage Example
Direction dir; // Constructed with user-defined default value
dir = Direction::WEST; // "feels" like a normal enum
switch (dir) {
    case Direction::NORTH: break;
    case Direction::EAST: break;
    case Direction::SOUTH: break;
    case Direction::WEST: break;
    default: break;
}

// Compile time programming
constexpr auto min = Direction::min(); // 0
constexpr auto max = Direction::max(); // 3 
constexpr auto count = Direction::count(); // 4
constexpr bool contiguous = Direction::is_contiguous(); // true
if constexpr (Direction::is_contiguous()){
    // Information is known at compile time to produce good code-gen. 
    // Templates are available to make this easier - see wiki for details
}

// Not allowed - bitwise ops not defined for Value Enums
// dir |= Direction::EAST;
```

#### Flags Enum

```c++
// This struct is generated by enumbra
struct FilePermissions {
    enum class Value : uint8_t {
        READ = 1,
        WRITE = 2,
        EXECUTE = 4,
        DELETE = 8
    };
    // Rest of struct omitted
}

// Usage Example
FilePermissions perms; // Constructed with user-defined default value
// Bitwise ops feel like normal
perms = FilePermissions::READ | FilePermissions::WRITE;
perms |= FilePermissions::EXECUTE; 

// Test and set with functions as well
perms.set(FilePermissions::DELETE);
if(perms.test(FilePermissions::DELETE)){
    // ...
}
perms.unset(FilePermissions::READ);

// Compile time programming
constexpr auto min = FilePermissions::min(); // 0 = 0b0000
constexpr auto max = FilePermissions::max(); // 15 = 0b1111
constexpr auto count = FilePermissions::count(); // 4
constexpr bool contiguous = FilePermissions::is_contiguous(); // true
if constexpr (FilePermissions::is_contiguous()){
    // Information is known at compile time to produce good code-gen. 
    // Templates are available to make this easier - see wiki for details
}

```

#### Compact Bit Field Enums

See [the Wiki](https://github.com/Scaless/enumbra/wiki/CPP-Packed-Enums) for more info.

## Other Languages
C#: Planned, not started yet.  


# Building
This section refers to building enumbra itself, not the generated code. See the Generators section for generated code requirements.

enumbra requires a C++17 compiler and is primarily tested to build on Windows with Visual Studio 2022. 

If you are on another OS/compiler and would like to add native support, open an issue/PR. I suck at cmake so don't expect any help.

enumbra uses vcpkg manifests for a couple of dependencies. Modify CMakeSettings.json and set cmakeToolchain to point to your vcpkg installation.

# Known Limitations
1. Flags Enums are required to have an unsigned underlying type.
2. Values within Flags Enums may not span multiple bits. A single value must control a single bit. (TODO: A separate mechanism for definining multi-bit flags should be added.)
3. Values and names within an enum must be unique.

# Q&A

### Q. How does enumbra compare to [magic_enum](https://github.com/Neargye/magic_enum)/[Better Enums](http://aantron.github.io/better-enums/index.html)?

* magic_enum and Better Enums are compile-time generators that work directly on C++ source. All they require is placing a header in your sources.
  * enumbra is a standalone code generator with its own configuration syntax that requires an additional build step.
* Constexpr generation slows down compile times / bloats memory. This cost is paid on every compile.
  * enumbra generates all of the code before compilation, which can be compiled once. Functions still benefit from constexpr where possible. enumbra can analyze enums to provide extra functionality that is not possible with compile-time libraries.
* The number of constants within an enum is usually limited to around 128 due to compiler limits for macros/templates.
  * enumbra has no limits on the number of entries within an enum.
* Lack of configuration options.
  * enumbra has a wide variety of configuration options and optional validation steps.
* Limited to one programming language (C++).
  * enumbra can potentially generate to any language ... but currently only C++.
* In magic_enum, the provided `bitwise_operators` namespace lets you use bitwise operators on ALL enums regardless of if they are intended to be flags or not.
  * enumbra defines operators for each enum individually, reducing the chance for mistake.
* Better Enums are limited to enums that you control and define.
  * enumbra can be used to manually re-define third-party enums, though this is a bit dangerous as you get out of sync. Some more investigation is needed here. magic_enum is the safer option here as it works directly on the original enum.
