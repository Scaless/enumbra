cmake_minimum_required (VERSION 3.20)

# C++ 20 Project
add_executable (enumbra_cpp20_test "cpp_test.cpp")
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	# cmake sets /W3 by default and there's no way to override except regex replacing it
	string(REGEX REPLACE "/W3" "/Wall" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	# As of Visual Studio 16.11, cmake infers cxx_std_20 to be c++latest instead of c++20, so we need to pass it as a flag instead
	target_compile_options(enumbra_cpp20_test PRIVATE /WX /Wall /wd4514 /wd5045 /std:c++20)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
		target_compile_options(enumbra_cpp20_test PRIVATE /WX /Wall /wd4514 /wd5045 /std:c++20)
		target_compile_options(enumbra_cpp20_test PRIVATE -Wno-c++98-compat -Wno-c++98-compat-pedantic)
	elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
		target_compile_options(enumbra_cpp20_test PRIVATE -Wall -Wextra -Wpedantic)
	endif()
else()
	message( FATAL_ERROR "Compiler Not Configured" )
endif()

# C++ 17 Project
add_executable (enumbra_cpp17_test "cpp_test.cpp")
target_compile_features(enumbra_cpp17_test PRIVATE cxx_std_17)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	# cmake sets /W3 by default and there's no way to override except regex replacing it
	string(REGEX REPLACE "/W3" "/Wall" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	# As of Visual Studio 16.11, cmake infers cxx_std_20 to be c++latest instead of c++20, so we need to pass it as a flag instead
	target_compile_options(enumbra_cpp17_test PRIVATE /WX /Wall /wd4514 /wd5045)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
		target_compile_options(enumbra_cpp17_test PRIVATE /WX /Wall /wd4514 /wd5045)
		target_compile_options(enumbra_cpp17_test PRIVATE -Wno-c++98-compat -Wno-c++98-compat-pedantic)
	elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
		target_compile_options(enumbra_cpp17_test PRIVATE -Wall -Wextra -Wpedantic)
	endif()
else()
	message( FATAL_ERROR "Compiler Not Configured" )
endif()

# C++ 14 Project
add_executable (enumbra_cpp14_test "cpp_test.cpp")
target_compile_features(enumbra_cpp14_test PRIVATE cxx_std_14)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	# cmake sets /W3 by default and there's no way to override except regex replacing it
	string(REGEX REPLACE "/W3" "/Wall" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	# As of Visual Studio 16.11, cmake infers cxx_std_20 to be c++latest instead of c++20, so we need to pass it as a flag instead
	target_compile_options(enumbra_cpp14_test PRIVATE /WX /Wall /wd4514 /wd5045)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
		target_compile_options(enumbra_cpp14_test PRIVATE /WX /Wall /wd4514 /wd5045)
		target_compile_options(enumbra_cpp14_test PRIVATE -Wno-c++98-compat -Wno-c++98-compat-pedantic)
	elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
		target_compile_options(enumbra_cpp14_test PRIVATE -Wall -Wextra -Wpedantic)
	endif()
else()
	message( FATAL_ERROR "Compiler Not Configured" )
endif()
