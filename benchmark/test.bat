@echo off
setlocal

cd /d %~dp0

echo Calling vcvarsall
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

rmdir /s /q build
mkdir build
rmdir /s /q report
mkdir report

echo TEST: Generate
call "..\builds\Win-MSVC-x64-Release\enumbra.exe" "-c" "C:/Repos/enumbra/examples/enumbra_config.json" "-s"  "C:/Repos/enumbra/benchmark/enum_benchmark.json" "--cppout" "C:/Repos/enumbra/benchmark/build/enumbra_test.hpp"
call "..\builds\Win-MSVC-x64-Release\enumbra.exe" "-c" "C:/Repos/enumbra/examples/enumbra_config_minimal.json" "-s"  "C:/Repos/enumbra/benchmark/enum_benchmark.json" "--cppout" "C:/Repos/enumbra/benchmark/build/enumbra_minimal.hpp"

REM Run the tests and record results
echo TEST: Compile

REM Run it once to do file caching
call cl /nologo /MD /EHsc /std:c++17 /Fo:build/ /Bt+ /Fe:build/benchmark_baseline.exe benchmark_baseline.cpp > report/report17_benchmark_baseline.txt
call cl /nologo /MD /EHsc /std:c++17 /Fo:build/ /Bt+ /Fe:build/benchmark_enumbra.exe benchmark_enumbra.cpp > report/report17_benchmark_enumbra.txt
call cl /nologo /MD /EHsc /std:c++17 /Fo:build/ /Bt+ /Fe:build/benchmark_enumbra_minimal.exe benchmark_enumbra_minimal.cpp > report/report17_benchmark_enumbra_minimal.txt
call cl /nologo /MD /EHsc /std:c++17 /Fo:build/ /Bt+ /Fe:build/benchmark_magic_enum.exe benchmark_magic_enum.cpp > report/report17_benchmark_magic_enum.txt
call cl /nologo /MD /EHsc /std:c++17 /Fo:build/ /Bt+ /Fe:build/benchmark_magic_enum_all.exe benchmark_magic_enum_all.cpp > report/report17_benchmark_magic_enum_all.txt

call cl /nologo /MD /EHsc /std:c++20 /Fo:build/ /Bt+ /Fe:build/benchmark_baseline.exe benchmark_baseline.cpp > report/report20_benchmark_baseline.txt
call cl /nologo /MD /EHsc /std:c++20 /Fo:build/ /Bt+ /Fe:build/benchmark_enumbra.exe benchmark_enumbra.cpp > report/report20_benchmark_enumbra.txt
call cl /nologo /MD /EHsc /std:c++20 /Fo:build/ /Bt+ /Fe:build/benchmark_enumbra_minimal.exe benchmark_enumbra_minimal.cpp > report/report20_benchmark_enumbra_minimal.txt
call cl /nologo /MD /EHsc /std:c++20 /Fo:build/ /Bt+ /Fe:build/benchmark_magic_enum.exe benchmark_magic_enum.cpp > report/report20_benchmark_magic_enum.txt
call cl /nologo /MD /EHsc /std:c++20 /Fo:build/ /Bt+ /Fe:build/benchmark_magic_enum_all.exe benchmark_magic_enum_all.cpp > report/report20_benchmark_magic_enum_all.txt

call cl /nologo /MD /EHsc /std:c++latest /Fo:build/ /Bt+ /Fe:build/benchmark_baseline.exe benchmark_baseline.cpp > report/report23_benchmark_baseline.txt
call cl /nologo /MD /EHsc /std:c++latest /Fo:build/ /Bt+ /Fe:build/benchmark_enumbra.exe benchmark_enumbra.cpp > report/report23_benchmark_enumbra.txt
call cl /nologo /MD /EHsc /std:c++latest /Fo:build/ /Bt+ /Fe:build/benchmark_enumbra_minimal.exe benchmark_enumbra_minimal.cpp > report/report23_benchmark_enumbra_minimal.txt
call cl /nologo /MD /EHsc /std:c++latest /Fo:build/ /Bt+ /Fe:build/benchmark_magic_enum.exe benchmark_magic_enum.cpp > report/report23_benchmark_magic_enum.txt
call cl /nologo /MD /EHsc /std:c++latest /Fo:build/ /Bt+ /Fe:build/benchmark_magic_enum_all.exe benchmark_magic_enum_all.cpp > report/report23_benchmark_magic_enum_all.txt

REM Run it a second time
call cl /nologo /MD /EHsc /std:c++17 /Fo:build/ /Bt+ /Fe:build/benchmark_baseline.exe benchmark_baseline.cpp > report/report17_benchmark_baseline.txt
call cl /nologo /MD /EHsc /std:c++17 /Fo:build/ /Bt+ /Fe:build/benchmark_enumbra.exe benchmark_enumbra.cpp > report/report17_benchmark_enumbra.txt
call cl /nologo /MD /EHsc /std:c++17 /Fo:build/ /Bt+ /Fe:build/benchmark_enumbra_minimal.exe benchmark_enumbra_minimal.cpp > report/report17_benchmark_enumbra_minimal.txt
call cl /nologo /MD /EHsc /std:c++17 /Fo:build/ /Bt+ /Fe:build/benchmark_magic_enum.exe benchmark_magic_enum.cpp > report/report17_benchmark_magic_enum.txt
call cl /nologo /MD /EHsc /std:c++17 /Fo:build/ /Bt+ /Fe:build/benchmark_magic_enum_all.exe benchmark_magic_enum_all.cpp > report/report17_benchmark_magic_enum_all.txt

call cl /nologo /MD /EHsc /std:c++20 /Fo:build/ /Bt+ /Fe:build/benchmark_baseline.exe benchmark_baseline.cpp > report/report20_benchmark_baseline.txt
call cl /nologo /MD /EHsc /std:c++20 /Fo:build/ /Bt+ /Fe:build/benchmark_enumbra.exe benchmark_enumbra.cpp > report/report20_benchmark_enumbra.txt
call cl /nologo /MD /EHsc /std:c++20 /Fo:build/ /Bt+ /Fe:build/benchmark_enumbra_minimal.exe benchmark_enumbra_minimal.cpp > report/report20_benchmark_enumbra_minimal.txt
call cl /nologo /MD /EHsc /std:c++20 /Fo:build/ /Bt+ /Fe:build/benchmark_magic_enum.exe benchmark_magic_enum.cpp > report/report20_benchmark_magic_enum.txt
call cl /nologo /MD /EHsc /std:c++20 /Fo:build/ /Bt+ /Fe:build/benchmark_magic_enum_all.exe benchmark_magic_enum_all.cpp > report/report20_benchmark_magic_enum_all.txt

call cl /nologo /MD /EHsc /std:c++latest /Fo:build/ /Bt+ /Fe:build/benchmark_baseline.exe benchmark_baseline.cpp > report/report23_benchmark_baseline.txt
call cl /nologo /MD /EHsc /std:c++latest /Fo:build/ /Bt+ /Fe:build/benchmark_enumbra.exe benchmark_enumbra.cpp > report/report23_benchmark_enumbra.txt
call cl /nologo /MD /EHsc /std:c++latest /Fo:build/ /Bt+ /Fe:build/benchmark_enumbra_minimal.exe benchmark_enumbra_minimal.cpp > report/report23_benchmark_enumbra_minimal.txt
call cl /nologo /MD /EHsc /std:c++latest /Fo:build/ /Bt+ /Fe:build/benchmark_magic_enum.exe benchmark_magic_enum.cpp > report/report23_benchmark_magic_enum.txt
call cl /nologo /MD /EHsc /std:c++latest /Fo:build/ /Bt+ /Fe:build/benchmark_magic_enum_all.exe benchmark_magic_enum_all.cpp > report/report23_benchmark_magic_enum_all.txt

echo TEST: Done

endlocal
