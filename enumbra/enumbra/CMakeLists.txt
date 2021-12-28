# CMakeList.txt : CMake project for enumbra, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

find_package(tomlplusplus)
find_package(cxxopts)
find_package(fmt)

# Add source to this project's executable.
add_executable (enumbra "enumbra.cpp" "cpp_generator.cpp")
target_compile_features(enumbra PRIVATE cxx_std_17)

target_link_libraries(enumbra PRIVATE 
	tomlplusplus::tomlplusplus
	cxxopts::cxxopts
	fmt::fmt)

add_executable (enumbra_cpp_test "cpp_test.cpp")
target_compile_features(enumbra_cpp_test PRIVATE cxx_std_17)

# TODO: Add tests and install targets if needed.
