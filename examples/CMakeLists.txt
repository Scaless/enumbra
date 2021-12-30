cmake_minimum_required (VERSION 3.8)

add_executable (enumbra_cpp_test "cpp_test.cpp")
target_compile_features(enumbra_cpp_test PRIVATE cxx_std_17)
