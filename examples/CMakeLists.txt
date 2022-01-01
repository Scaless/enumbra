cmake_minimum_required (VERSION 3.15)

# C++ 20 Project
add_executable (enumbra_cpp20_test "cpp_test.cpp")
target_compile_features(enumbra_cpp20_test PRIVATE cxx_std_20)
target_compile_definitions(enumbra_cpp20_test PRIVATE CPP_TEST_VER=20)

if(MSVC)
	# cmake sets /W3 by default and there's no way to override except regex replacing it
	string(REGEX REPLACE "/W3" "/Wall" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	# Add additional options
	target_compile_options(enumbra_cpp20_test PRIVATE /WX)
else()
	target_compile_options(enumbra_cpp20_test PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# C++ 17 Project
add_executable (enumbra_cpp17_test "cpp_test.cpp")
target_compile_features(enumbra_cpp17_test PRIVATE cxx_std_17)
target_compile_definitions(enumbra_cpp17_test PRIVATE CPP_TEST_VER=17)

if(MSVC)
	# cmake sets /W3 by default and there's no way to override except regex replacing it
	string(REGEX REPLACE "/W3" "/Wall" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	# Add additional options
	target_compile_options(enumbra_cpp17_test PRIVATE /WX)
else()
	target_compile_options(enumbra_cpp17_test PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# C++ 14 Project
add_executable (enumbra_cpp14_test "cpp_test.cpp")
target_compile_features(enumbra_cpp14_test PRIVATE cxx_std_14)
target_compile_definitions(enumbra_cpp14_test PRIVATE CPP_TEST_VER=14)

if(MSVC)
	# cmake sets /W3 by default and there's no way to override except regex replacing it
	string(REGEX REPLACE "/W3" "/Wall" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	# Add additional options
	target_compile_options(enumbra_cpp14_test PRIVATE /WX)
else()
	target_compile_options(enumbra_cpp14_test PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# C++ 11 Project
add_executable (enumbra_cpp11_test "cpp_test.cpp")
target_compile_features(enumbra_cpp11_test PRIVATE cxx_std_11)
target_compile_definitions(enumbra_cpp11_test PRIVATE CPP_TEST_VER=11)

if(MSVC)
	# cmake sets /W3 by default and there's no way to override except regex replacing it
	string(REGEX REPLACE "/W3" "/Wall" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	# Add additional options
	target_compile_options(enumbra_cpp11_test PRIVATE /WX)
else()
	target_compile_options(enumbra_cpp11_test PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
