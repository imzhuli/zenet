@echo off
if not defined VCPKG_PATH goto :env_failed

rd/S/Q .\build
md build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_PATH%
if "%errorlevel%"=="1" goto :cmake_failed

cmake --build . --config Release
if "%errorlevel%"=="1" goto :build_failed

cmake --install .

goto :end

REM Error Cases:
:env_failed
echo environment check failed, make sure VCPKG_PATH points to currect vcpkg path
exit /B

:cmake_failed
echo cmake configuration error !
goto :end

:build_failed
echo Failed to build target(s) !
goto :end

:end
cd ..