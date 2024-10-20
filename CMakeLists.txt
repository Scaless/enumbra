# CMakeList.txt : CMake project for enumbra, include source and define
# project specific logic here.
cmake_minimum_required (VERSION 3.20)

project ("enumbra")

find_package(nlohmann_json)
find_package(cxxopts)
find_package(fmt)

# Add source to this project's executable.
add_executable (enumbra
		"src/enumbra.cpp"
		"src/cpp_generator.cpp"
		"src/cpp_utility.cpp"
 "benchmark/benchmark_shared.h")
target_compile_features(enumbra PRIVATE cxx_std_17)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	# cmake sets /W3 by default and there's no way to override except regex replacing it
	string(REGEX REPLACE "/W3" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	target_compile_options(enumbra PRIVATE /W4 /WX)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC") # aka clang-cl
		target_compile_options(enumbra PRIVATE /W4 /WX)
		target_compile_options(enumbra PRIVATE -Wno-c++98-compat -Wno-c++98-compat-pedantic)
	elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
		target_compile_options(enumbra PRIVATE -Wall -Wextra -Wpedantic)
	endif()
else()
	message( FATAL_ERROR "Compiler Not Configured" )
endif()

target_link_libraries(enumbra PRIVATE 
	cxxopts::cxxopts
	fmt::fmt
)

add_subdirectory(examples)

# CMAKE_TOOLCHAIN_FILE is used the first time cmake is configured, but then creates
# an annoying warning on subsequent cmake project reloads since it's not referenced.
# We just reference it here to make it slightly less annoying.
MESSAGE(STATUS "Using CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")

# TODO: Add tests and install targets if needed.
