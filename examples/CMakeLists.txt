cmake_minimum_required (VERSION 3.20)

# cmake sets /W3 by default and there's no way to override except regex replacing it
if(MSVC)
	string(REGEX REPLACE "/W3" "/Wall" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

# C++ 20 Project
add_executable (enumbra_cpp20_test "cpp_test.cpp")
if(MSVC)
	# As of Visual Studio 16.11, cmake infers cxx_std_20 to be c++latest instead of c++20, so we need to pass it as a flag instead
	target_compile_options(enumbra_cpp20_test PRIVATE /WX /std:c++20)
else()
	target_compile_features(enumbra_cpp20_test PRIVATE cxx_std_20)
	target_compile_options(enumbra_cpp20_test PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# C++ 17 Project
add_executable (enumbra_cpp17_test "cpp_test.cpp")
target_compile_features(enumbra_cpp17_test PRIVATE cxx_std_17)
if(MSVC)
	target_compile_options(enumbra_cpp17_test PRIVATE /WX)
else()
	target_compile_options(enumbra_cpp17_test PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# C++ 14 Project
add_executable (enumbra_cpp14_test "cpp_test.cpp")
target_compile_features(enumbra_cpp14_test PRIVATE cxx_std_14)
if(MSVC)
	target_compile_options(enumbra_cpp14_test PRIVATE /WX)
else()
	target_compile_options(enumbra_cpp14_test PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
