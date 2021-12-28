# enumbra

A code generator for enums.

# Examples

Annotated config files are provided as enumbra_config.toml and enum_config.toml. 

# Building

enumbra uses vcpkg for a couple of dependencies. Modify CMakeSettings.json to fit your setup, namely set cmakeToolchain to your vcpkg toolchain file.

# Limitations

1. TOML integers are represented by INT64, therefore values greater than INT64_MAX cannot be represented currently. The plan is to eventually support an optional string format for values not representable by INT64. Until then, the toml parser should give you one of the following warnings:
	* Error while parsing decimal integer: '9446744073709551615' is not representable in 64 bits
	* Error while parsing decimal integer: exceeds maximum length of 19 characters

# Q&A

Q. Why is the library called enumbra (pronounced e-num-bruh)?

A. The word umbra represents a region behind a celestial body where light is obscured. C++ enums sit in that region of the language. They're integers, kind of? They're flags, kind of? They're an *enumeration*, yet not *enumerable*? They are widely used as bit flags but there are few built-in resources for safely using them as such. They can be incredibly useful but for some reason feel neglected over shiny new features (reflection, etc).

![umbra](https://www.nasa.gov/sites/default/files/umbra-penumbra.jpg)
[Source: NASA.gov](https://www.nasa.gov/audience/forstudents/k-4/stories/umbra-and-penumbra)

Q. Why not use another library like [magic-enum](https://github.com/Neargye/magic_enum)?

Compile-time libraries like magic-enum rely on compiler hacks to function properly. For large enums, constexpr generation is slow and cumbersome on compile times / memory. Since enumbra pre-generates all its data, compiling is fast and can provide some additional functionality. magic_enum has convenience in its simplicity, just pop the header in and you're done. Use what works for you.

Q. Why are you not using <templates/reflection/language feature>?

A. The entire reason I made this project is because existing solutions are too complicated, lack the specific features I need, or are not supported on the compilers that I am restricted to. You are free to fork the project and alter the outputs to your liking, or submit a PR. I suggest making an issue first to discuss if it's an appropriate change.
