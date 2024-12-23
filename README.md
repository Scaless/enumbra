
# Overview
enumbra is a code generator for enums. Config file in, generated code file out.  
**enumbra is in a volatile state of development where syntax and features may change at any time.**  
Please refer to the [wiki](https://github.com/Scaless/enumbra/wiki) for full documentation.  

Starter config files are provided in the [examples](/examples) directory.  
Running the following command will parse the given config files and generate [enumbra_test.hpp](/examples/enumbra_test.hpp)  

```
./enumbra.exe -c enumbra_config.json -s enum.json --cppout enumbra_test.hpp
```

enumbra generates two core types of enums: Value Enum and Flags Enum.  

| Enum Type | State                    | Bitwise Ops | Bitfield Packing (C++) |
|-----------|--------------------------|-------------|-------------------|
| Value     | Single Value             | No          | Yes               |
| Flags     | None / Single / Multiple | Yes         | Yes               |


# Generators

## C++
Generated C++ code requires a minimum of C++17. Additional features are conditionally enabled up to C++23. 

By default, generated headers **do not include any standard library headers** and are completely standalone! Features are implemented completely with the C++ core language.  

The example configuration includes ```<cstdint>``` for fixed size portable types, which is the recommended configuration.

There are no master headers or include order issues to worry about, however including headers generated with incompatible versions of enumbra will issue compiler warnings.  

### Examples

#### Value Enum

```c++
//// generated_enumbra_header.h
// A header containing this enum and supporting functions is generated by enumbra
enum class Direction : uint8_t {
    NORTH = 0,
    EAST = 1,
    SOUTH = 2,
    WEST = 3
};
// < generated enumbra functions omitted >

//// my_cool_project.cpp
#include <generated_enumbra_header.h>

// Default value is defined in the config used to generate the header
Direction dir = enumbra::default_value<Direction>();
dir = Direction::WEST;
switch (dir) {
    case Direction::NORTH: break;
    case Direction::EAST: break;
    case Direction::SOUTH: break;
    case Direction::WEST: break;
    default: break;
}

// Compile time programming
constexpr auto min = enumbra::min<Direction>(); // == 0
constexpr auto max = enumbra::max<Direction>(); // == 3 
constexpr auto count = enumbra::count<Direction>(); // == 4
if constexpr (enumbra::is_contiguous<Direction>()){
    // All metadata is known at compile time to allow for good code-gen.
}

// Not allowed - bitwise ops not defined for Value Enums
// dir |= Direction::EAST;
```

#### Flags Enum

```c++
//// generated_enumbra_header.h
// A header containing this enum and supporting functions is generated by enumbra
enum class FilePermissions : uint8_t {
    READ = 1,
    WRITE = 2,
    EXECUTE = 4,
    DELETE = 8
};
// < generated enumbra functions omitted >

//// my_cool_project.cpp
#include <generated_enumbra_header.h>

// Default value is defined in the config used to generate the header
FilePermissions perms = enumbra::default_value<FilePermissions>();

// Bitwise ops are generated and work like normal
perms = FilePermissions::READ | FilePermissions::WRITE;
perms |= FilePermissions::EXECUTE; 

// Test and set with functions as well
enumbra::set(perms, FilePermissions::DELETE);
if(enumbra::test(perms, FilePermissions::DELETE)){
    // All metadata is known at compile time to allow for good code-gen.
}
enumbra::unset(perms, FilePermissions::READ);

// Compile time programming
constexpr auto min = enumbra::min<FilePermissions>(); // == 0 == 0b0000
constexpr auto max = enumbra::max<FilePermissions>(); // == 15 == 0b1111
constexpr auto count = enumbra::count<FilePermissions>(); // == 4 total flags
if constexpr (enumbra::is_contiguous<FilePermissions>()){
    // All metadata is known at compile time to allow for good code-gen.
}

```

### Enumbra Global Functions

Functions live in the enumbra:: namespace, which is omitted below for brevity.

| Function | Description |
|----------|-------------|
| ```bool is_enumbra_enum<T>()``` | Returns true if T was generated by enumbra. `enum` and `enum class` types that were not generated by enumbra are not supported.
| ```bool is_enumbra_value_enum<T>()``` | Returns true if T is a Value Enum. |
| ```bool is_enumbra_flags_enum<T>()``` | Returns true if T is a Flags Enum. |

### Value Enum Functions

Functions live in the enumbra:: namespace, which is omitted below for brevity.

| Function | Description |
|----------|-------------|
| ```T min<T>()``` | Returns the minimum valid value for the enum. |
| ```T max<T>()``` | Returns the maximum valid value for the enum. |
| ```T default_value<T>()``` | Returns the default value for the enum. |
| ```int count<T>()``` | Returns the number of valid values for the enum. |
| ```bool is_contiguous<T>()``` | Returns true if all values in the enum are contiguous (no gaps). |
| ```int count<T>()``` | Returns the number of valid values for the enum. |
| ```int bits_required_storage<T>()``` | Returns the minimum number of bits required to store the maximum valid value in a bitfield. | 
| ```int bits_required_transmission<T>()``` | Returns the number of bits required to transmit any value in the enum where both sides know the range of valid values. This is distinct from the number of storage bits since we can just send the difference between (max - min).<br>TODO: Might remove, benefit is questionable. |
| ```const T[]& values<T>()``` | Returns a C-style array containing all valid values for the enum.
| ```optional_value_<T> from_integer<T>(underlying_type value)``` | Returns an Optional-like type which indicates if the conversion from a raw integer failed or succeeded. |
| ```T from_integer_unsafe<T>(underlying_type s)``` | Cast an integer value to T. |
| ```optional_value_<T> from_string<T>(const char* str, int len)``` | Returns an Optional-like type which indicates if the conversion from string failed or succeeded. |
| ```optional_value_<T> from_string<T>(const char* str)``` | Returns an Optional-like type which indicates if the conversion from string failed or succeeded. |
| ```underlying_type to_underlying<T>(T v)``` | Cast T to its underlying integer type. |
| ```const char* to_string<T>(T v)``` | Get the string value for T. Returns nullptr on failure. |
| ```bool is_valid<T>(T v)``` | Returns true if ```v``` contains a valid value in the enum.  Useful for asserts. |

### Flags Enum Functions

Functions live in the enumbra:: namespace, which is omitted below for brevity.

| Function | Description |
|----------|-------------|
TODO

#### Compact Bit Field Enums

See [the Wiki](https://github.com/Scaless/enumbra/wiki/CPP-Packed-Enums) for more info.

## Other Languages
Maybe eventually. Focus is on C++.

# Building
This section refers to building enumbra itself, not the generated code that enumbra spits out. See the [Generators](#generators) section for generated code requirements.

enumbra requires a C++17 compiler and is primarily tested to build on Windows with Visual Studio 2022, but should also work for clang and gcc. 

If you are on another OS/compiler and would like to add native support, open an issue/PR with non-invasive changes.

enumbra uses vcpkg manifests for a couple of dependencies and should be automatically detected if VCPKG_ROOT is set.

# Limitations
1. Flags Enums are required to have an unsigned underlying type.
2. Values within Flags Enums may not span multiple bits. A single value must control a single bit. (TODO)
3. Values within an enum must be unique (no aliasing).

# Q&A

### Q. Why would I use enumbra instead of [magic_enum](https://github.com/Neargye/magic_enum)?

The #1 reason is if you care about compile times enough to put up with a separate generation step in your build process. 
There's a few features unique to each library, but it cannot be overstated how convenient it is to use magic_enum. 
Use what makes sense for your goals. 

### Q. How does enumbra compare to [magic_enum](https://github.com/Neargye/magic_enum)?

| | enumbra    | magic_enum |
| --------| -------- | ------- |
| Including in your codebase | Code Generator -> Header for each enum group | Header Only |
| Use external enum/enum class | No [1] | Yes |
| Compile speed | Fast | Slow |
| Max enum elements | Unlimited | 256 (default) [2] |
| Aliasing allowed | No | Partial |
| constexpr | All the things | All the things |
| bitwise operations | Yes (always type-safe) | bitwise_operators  (unsafe)<br>customize::enum_range\<T\>::is_flags (type-safe) |
| Additional enum validation | During header generation | None [3] |
| Language required | C++17 | C++17 |
| Enumerate values | Yes | Yes |
| from/to_string | Yes | Yes |
| from/to_integer | Yes | Yes |
| i/ostream support | No | Yes |
| **Metadata**<br>name<br>min<br>max<br>default_value<br>count<br>is_contiguous<br>bits_required_storage<br>bits_required_transmission<br> is_scoped/is_unscoped<br>index | <br>Yes<br>Yes<br>Yes<br>Yes<br>Yes<br>Yes<br>Yes<br>Yes<br>No [4]<br>No | <br>Yes<br>No [3]<br>No [3]<br>No [3]<br>Yes<br>No [3]<br>No [3]<br>No [3]<br>Yes<br>Yes
| Other Features | bitfield support | array/bitset/set containers

[1] Investigating possible workarounds to ingest foreign enums  
[2] Maximum depends on individual compiler template limits, hard limit of 65536  
[3] Could be implemented by user at compile time, but cost paid on every compile  
[4] All enums created by enumbra are scoped (enum class)  

https://github.com/Neargye/magic_enum/

https://aantron.github.io/better-enums/
