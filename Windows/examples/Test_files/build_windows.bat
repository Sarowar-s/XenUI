
@echo off
setlocal EnableExtensions
title Build XenUI Example

REM ==================================================
REM Configuration
REM ==================================================

set "EXAMPLE_DIR=%~dp0"
set "ROOT_DIR=%EXAMPLE_DIR%..\.."
set "XENUI_BUILD_DIR=%ROOT_DIR%\build-cmd"
set "APP_BUILD_DIR=%EXAMPLE_DIR%build-app"

set "CMAKE=cmake"
set "JOBS=4"

REM ==================================================
REM Main
REM ==================================================

call :CheckRequirements || goto :Error
call :Configure || goto :Error
call :Build || goto :Error

echo.
echo ==========================================
echo BUILD SUCCESSFUL
echo ==========================================
echo Executable:
echo "%APP_BUILD_DIR%\xenui_test.exe"
echo.
pause
exit /b 0

REM ==================================================
REM Validate required files
REM ==================================================

:CheckRequirements
echo.
echo ==========================================
echo       Build XenUI Example Application
echo ==========================================
echo.

where cmake >nul 2>&1
if errorlevel 1 (
    echo ERROR: CMake not found in PATH.
    echo Install CMake or add it to your PATH.
    exit /b 1
)

if not exist "%XENUI_BUILD_DIR%\libXenUI.dll.a" (
    echo ERROR: XenUI import library not found.
    echo Expected: "%XENUI_BUILD_DIR%\libXenUI.dll.a"
    echo Build XenUI separately first.
    exit /b 1
)

if not exist "%XENUI_BUILD_DIR%\libXenUI.dll" (
    echo ERROR: XenUI runtime DLL not found.
    echo Expected: "%XENUI_BUILD_DIR%\libXenUI.dll"
    exit /b 1
)

if not exist "%EXAMPLE_DIR%CMakeLists.txt" (
    echo ERROR: Example CMakeLists.txt not found.
    exit /b 1
)

if not exist "%EXAMPLE_DIR%windows.cpp" (
    echo ERROR: windows.cpp not found.
    exit /b 1
)

if not exist "%EXAMPLE_DIR%app_icon.rc" (
    echo ERROR: app_icon.rc not found.
    exit /b 1
)

if not exist "%EXAMPLE_DIR%logo.ico" (
    echo ERROR: logo.ico not found.
    exit /b 1
)

exit /b 0

REM ==================================================
REM Configure the example only
REM ==================================================

:Configure
echo [1/2] Configuring example application...
echo.

cd /d "%EXAMPLE_DIR%" || exit /b 1

REM Keep this tested command: it works with our paths.
"%CMAKE%" -G "MinGW Makefiles" -B "build-app"

if errorlevel 1 (
    echo ERROR: CMake configuration failed.
    exit /b 1
)

exit /b 0

REM ==================================================
REM Build the executable
REM ==================================================

:Build
echo.
echo [2/2] Building xenui_test.exe...
echo.

"%CMAKE%" --build "%APP_BUILD_DIR%" --parallel %JOBS%

if errorlevel 1 (
    echo ERROR: Application build failed.
    exit /b 1
)

if not exist "%APP_BUILD_DIR%\xenui_test.exe" (
    echo ERROR: Expected executable was not created.
    exit /b 1
)

exit /b 0

REM ==================================================
REM Error handler
REM ==================================================

:Error
echo.
echo ==========================================
echo BUILD FAILED
echo ==========================================
echo.
pause
exit /b 1
