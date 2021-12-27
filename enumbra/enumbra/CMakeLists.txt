# CMakeList.txt : CMake project for enumbra, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

find_package(tomlplusplus)
find_package(cxxopts)

# Add source to this project's executable.
add_executable (enumbra "enumbra.cpp" "cpp_generator.cpp")

target_compile_features(enumbra PRIVATE cxx_std_17)

target_link_libraries(enumbra PRIVATE tomlplusplus::tomlplusplus)
target_link_libraries(enumbra PRIVATE cxxopts::cxxopts)

# TODO: Add tests and install targets if needed.
