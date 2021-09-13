# enumbra

A code generator for enums.

# Examples

# Limitations
#### PLEASE BE AWARE OF THE FOLLOWING CURRENT LIMITATIONS OF enumbra
1. TOML integers are represented by INT64, therefore values between INT64_MAX and UINT64_MAX cannot be represented currently.

# Q&A

Q> Why is the library called enumbra (pronounced e-num-bruh)?

A> The word umbra represents a region where visible light is obscured by another body. Enums are often obscured by the languages they reside in, so I thought it was a fitting name.

Q> Why isnt %thing% done using <templates/reflection/language feature>?

A> Because I didn't know how or it was too cumbersome. The entire reason I made this project is because exising solutions are too complicated, lack the features I want, or are not supported on the compilers that I am restricted to. You are free to fork the project and alter the outputs to your liking. PRs using newer language features are welcome, however they will only be accepted if they follow the baseline requirements of this project and/or are sufficiently configurable to be disabled.

Q> Why code generation instead of <templates/reflection/language feature>?

A> C++ constexpr enums just are not there yet. The template syntax to generate the functions and outputs is ugly or plain impossible to fit many desired use-cases. Large constexpr expressions are slow and cumbersome on compile times / memory. Pre-generating all of the relevant data is just very convenient. C++23 reflection will probably get us a step closer but I am not convinced it is the final solution.
