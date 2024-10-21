cmake_minimum_required (VERSION 3.26)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	# cmake sets /W3 by default and there's no way to override except regex replacing it
	string(REGEX REPLACE "/W3" "/Wall" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

set(ENUMBRA_MSVC_WARNING_FLAGS
	/Wall          # ALL Warnings
	/wd4514        # unreferenced inline function has been removed
	/permissive-   # standard conformance
)

set(ENUMBRA_CLANG_WARNING_FLAGS
	-Weverything                # Actually ALL warnings
	-Wno-c++98-compat           # Don't care
	-Wno-c++98-compat-pedantic  # Don't care
	-Wno-unsafe-buffer-usage    # One of the few "real" warnings we trigger, not much we could do about this without dragging in dependencies
	-Wno-switch-default         # Complains if we do, complains if we don't...
	-Wno-unused-macros          # Yep
)

set(ENUMBRA_GCC_WARNING_FLAGS
	-Wall      # "all" Warnings
	-Wextra    # I can't believe it's all Warnings
	-Wpedantic # standard conformance
)

# C++ 23 Project
add_executable (enumbra_cpp23_test "cpp_test.cpp" "cpp_minimal.cpp")
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(enumbra_cpp23_test PRIVATE /std:c++latest ${ENUMBRA_MSVC_WARNING_FLAGS})
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
		target_compile_options(enumbra_cpp23_test PRIVATE /std:c++latest ${ENUMBRA_MSVC_WARNING_FLAGS} ${ENUMBRA_CLANG_WARNING_FLAGS})
	else()
		target_compile_options(enumbra_cpp23_test PRIVATE -std=c++23 ${ENUMBRA_GCC_WARNING_FLAGS} ${ENUMBRA_CLANG_WARNING_FLAGS})
	endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(enumbra_cpp23_test PRIVATE -std=c++23 ${ENUMBRA_GCC_WARNING_FLAGS})
else()
	message( FATAL_ERROR "Compiler Not Configured" )
endif()

# C++ 20 Project
add_executable (enumbra_cpp20_test "cpp_test.cpp" "cpp_minimal.cpp")
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(enumbra_cpp20_test PRIVATE /std:c++20 ${ENUMBRA_MSVC_WARNING_FLAGS})
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
		target_compile_options(enumbra_cpp20_test PRIVATE /std:c++20 ${ENUMBRA_MSVC_WARNING_FLAGS} ${ENUMBRA_CLANG_WARNING_FLAGS})
	else()
		target_compile_options(enumbra_cpp20_test PRIVATE -std=c++20 ${ENUMBRA_GCC_WARNING_FLAGS} ${ENUMBRA_CLANG_WARNING_FLAGS})
	endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(enumbra_cpp20_test PRIVATE -std=c++20 ${ENUMBRA_GCC_WARNING_FLAGS})
else()
	message( FATAL_ERROR "Compiler Not Configured" )
endif()

# C++ 17 Project
add_executable (enumbra_cpp17_test "cpp_test.cpp" "cpp_minimal.cpp")
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(enumbra_cpp17_test PRIVATE /std:c++17 ${ENUMBRA_MSVC_WARNING_FLAGS})
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
		target_compile_options(enumbra_cpp17_test PRIVATE /std:c++17 ${ENUMBRA_MSVC_WARNING_FLAGS} ${ENUMBRA_CLANG_WARNING_FLAGS})
	else()
		target_compile_options(enumbra_cpp17_test PRIVATE -std=c++17 ${ENUMBRA_GCC_WARNING_FLAGS} ${ENUMBRA_CLANG_WARNING_FLAGS})
	endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(enumbra_cpp17_test PRIVATE -std=c++17 ${ENUMBRA_GCC_WARNING_FLAGS})
else()
	message( FATAL_ERROR "Compiler Not Configured" )
endif()
