enumbra is a code generator for enums.

**enumbra is in a volatile state of development where syntax and features may change at any time.  
If you need stability, wait until the 1.0 release.**

# Getting Started
Please refer to the [wiki](https://github.com/Scaless/enumbra/wiki) for full documentation.  

Annotated starter config files are provided in the [examples](/examples/) directory.  
Running the following command will parse the given config files and generate [enumbra_test.hpp](/examples/enumbra_test.hpp).  
```
./enumbra.exe -c enumbra_config.json -s enum.json --cppout enumbra_test.hpp
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
C: No plans.  
C++98/03: No interest. Without the bare minimum C++11 features you may as well just use C.  

# Types
enumbra provides two core types of enums: Value Enum and Flags Enum.  

| Enum Type | State | Bitwise Ops | Bit Packing (C++) |
| --- | --- | --- | --- |
| Value | Single Value | No | Yes |
| Flags | None / Single / Multiple | Yes | Yes |

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
