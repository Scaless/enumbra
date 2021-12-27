# enumbra

A code generator for enums.

# Examples

# Building

enumbra uses vcpkg for a couple of dependencies. Modify CMakeSettings.json to fit your setup, namely set cmakeToolchain to your vcpkg toolchain file.

# Limitations

1. TOML integers are represented by INT64, therefore values greater than INT64_MAX cannot be represented currently. The plan is to eventually support an optional string format for values between INT64_MAX and UINT64_MAX. Until then, the toml parser should give you one of the following warnings:
	* Error while parsing decimal integer: '9446744073709551615' is not representable in 64 bits
	* Error while parsing decimal integer: exceeds maximum length of 19 characters

# Q&A

Q. Why is the library called enumbra (pronounced e-num-bruh)?

A. The word umbra represents a region where visible light is obscured by another body. C++ enums are used for multiple purposes and their potential is obscured by the rest of the language. They're integers, kind of? They're flags, kind of? They're an *enumeration*, yet not *enumerable*? C++ enums are just in a really weird place. Libraries like magic-enum rely on compiler hacks to function properly. The template syntax to generate the functions and metadata is ugly or plain impossible to fit many desired use-cases. The inability to iterate through an enums values without macros or other hacks is a huge detriment. Large constexpr expressions are slow and cumbersome on compile times / memory. Pre-generating all of the relevant data is just very convenient.

Q. Why are you not using <templates/reflection/language feature>?

A. Because I didn't know how or it was too cumbersome. The entire reason I made this project is because exising solutions are too complicated, lack the features I want, or are not supported on the compilers that I am restricted to. You are free to fork the project and alter the outputs to your liking.

