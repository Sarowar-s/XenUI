
@echo off
setlocal
title XenonUI - Windows Build

cd /d "%~dp0"

echo.
echo ========================================
echo       XenonUI Application Builder
echo ========================================
echo.

REM Check required build tools.
where cmake >nul 2>&1
if errorlevel 1 (
    echo ERROR: CMake was not found in PATH.
    echo Install CMake or add it to PATH.
    pause
    exit /b 1
)

where ninja >nul 2>&1
if errorlevel 1 (
    echo ERROR: Ninja was not found in PATH.
    echo Install Ninja or add it to PATH.
    pause
    exit /b 1
)

where g++ >nul 2>&1
if errorlevel 1 (
    echo ERROR: MinGW GCC was not found in PATH.
    echo Add your MinGW bin directory to PATH.
    pause
    exit /b 1
)

echo [1/2] Configuring project...

cmake -S . -B build -G Ninja

if errorlevel 1 (
    echo.
    echo ERROR: CMake configuration failed.
    pause
    exit /b 1
)

echo.
echo [2/2] Compiling application...

cmake --build build

if errorlevel 1 (
    echo.
    echo ERROR: Compilation failed.
    pause
    exit /b 1
)

echo.
echo ========================================
echo         BUILD SUCCESSFUL!
echo ========================================
echo.
echo Executable: build\xenui_test.exe
echo.

pause
