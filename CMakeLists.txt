# CMakeList.txt : CMake project for enumbra, include source and define
# project specific logic here.
cmake_minimum_required (VERSION 3.15)

project ("enumbra")

find_package(tomlplusplus)
find_package(cxxopts)
find_package(fmt)

# Add source to this project's executable.
add_executable (enumbra "enumbra.cpp" "cpp_generator.cpp")
target_compile_features(enumbra PRIVATE cxx_std_17)

if(MSVC)
	# cmake sets /W3 by default and there's no way to override except regex replacing it
	string(REGEX REPLACE "/W3" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	# Add additional options
	target_compile_options(enumbra PRIVATE /W4 /WX)
else()
	target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

target_link_libraries(enumbra PRIVATE 
	tomlplusplus::tomlplusplus
	cxxopts::cxxopts
	fmt::fmt)

add_subdirectory(examples)

# TODO: Add tests and install targets if needed.
