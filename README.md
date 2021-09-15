# enumbra

A code generator for enums.

# Examples

# Limitations
#### PLEASE BE AWARE OF THE FOLLOWING CURRENT LIMITATIONS OF enumbra
1. TOML integers are represented by INT64, therefore values greater than INT64_MAX cannot be represented currently. The plan is to eventually support an optional string format. Until then, the toml parser should give you with one of the following warnings:
	* Error while parsing decimal integer: '9446744073709551615' is not representable in 64 bits
	* Error while parsing decimal integer: exceeds maximum length of 19 characters

# Q&A

Q. Why is the library called enumbra (pronounced e-num-bruh)?

A. The word umbra represents a region where visible light is obscured by another body. The reason I made the library was to handle C++, where using enums for multiple purposes is obscured by the language. They're integers, kind of? They're flags, kind of? They're an *enumeration*, yet not *enumerable*? 

Q. Why isnt <thing> done using <templates/reflection/language feature>?

A. Because I didn't know how or it was too cumbersome. The entire reason I made this project is because exising solutions are too complicated, lack the features I want, or are not supported on the compilers that I am restricted to. You are free to fork the project and alter the outputs to your liking. PRs using newer language features are welcome, however they will only be accepted if they follow the baseline requirements of this project and/or are sufficiently configurable.

Q. Why code generation instead of <templates/reflection/language feature>?

A. C++ enums just are not there yet. The template syntax to generate the functions and metadata is ugly or plain impossible to fit many desired use-cases. The inability to iterate through an enums values without macros or other hacks is a huge detriment. Large constexpr expressions are slow and cumbersome on compile times / memory. Pre-generating all of the relevant data is just very convenient. C++23 reflection will probably get us a step closer but I am not convinced it is the final solution.
