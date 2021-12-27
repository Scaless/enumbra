# enumbra

A code generator for enums.

# Examples

# Building

enumbra uses vcpkg for a couple of dependencies. Modify CMakeSettings.json to fit your setup, namely set cmakeToolchain to your vcpkg toolchain file.

# Limitations

1. TOML integers are represented by INT64, therefore values greater than INT64_MAX cannot be represented currently. The plan is to eventually support an optional string format for values not representable by INT64. Until then, the toml parser should give you one of the following warnings:
	* Error while parsing decimal integer: '9446744073709551615' is not representable in 64 bits
	* Error while parsing decimal integer: exceeds maximum length of 19 characters

# Q&A

Q. Why is the library called enumbra (pronounced e-num-bruh)?

A. The word umbra represents a region behind a celestial body where light is obscured.

![umbra](https://www.nasa.gov/sites/default/files/umbra-penumbra.jpg)
[Source: NASA.gov](https://www.nasa.gov/audience/forstudents/k-4/stories/umbra-and-penumbra)

C++ enums sit in that region of the language. They're integers, kind of? They're flags, kind of? They're an *enumeration*, yet not *enumerable*? They are widely used as bit flags but there are few built-in resources for safely using them as such. 

Q. Why not use another library like [magic-enum](https://github.com/Neargye/magic_enum)?

Libraries like magic-enum rely on compiler hacks to function properly. For lage enums, constexpr generation is slow and cumbersome on compile times / memory. Pre-generating all of the relevant data is just very convenient. Enumbra will eventually support exporting to multiple langauges, making it easy to define your data in one central place. 

Q. Why are you not using <templates/reflection/language feature>?

A. Because I didn't know how or it was too cumbersome. The entire reason I made this project is because exising solutions are too complicated, lack the features I want, or are not supported on the compilers that I am restricted to. You are free to fork the project and alter the outputs to your liking or submit a PR.
