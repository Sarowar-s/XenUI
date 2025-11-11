


@echo off
setlocal enabledelayedexpansion
::
::
::  SPDX-License-Identifier: Apache-2.0
::
::  Copyright (C) 2025 MD S M Sarowar Hossain
::


:: ====================================================================
:: Defaults
:: ====================================================================
set "BUILD_TYPE=Release"
set "CLEAN_BUILD=false"
set "ABI=arm64-v8a"
set "NDK_PLATFORM=android-31"
set "BUILD_TOOLS_VERSION=34.0.0"

:: ====================================================================
:: Argument Parser loop
:: ====================================================================
:parse_args
if "%~1"=="" goto end_parse_args

set "arg=%~1"
shift

if /i "%arg%"=="--help"    goto show_help
if /i "%arg%"=="--debug"   set "BUILD_TYPE=Debug"    & goto parse_args
if /i "%arg%"=="--release" set "BUILD_TYPE=Release"  & goto parse_args
if /i "%arg%"=="--clean"   set "CLEAN_BUILD=true"    & goto parse_args

if /i "%arg%"=="--abi" (
    if "%~1"=="" (
        echo [ERROR] Missing value for --abi
        exit /b 1
    )
    set "ABI=%~1"
    shift
    goto parse_args
)

:: Unrecognized flag: skip
goto parse_args

:end_parse_args

:: ====================================================================
:: Welcome banner
:: ====================================================================
set "TODAY=%DATE% %TIME%"
echo.
echo ================================================================================
echo Starting APK build process at %TODAY%
echo    Build Type : %BUILD_TYPE%
echo    Target ABI : %ABI%
echo ================================================================================

:: ====================================================================
:: Step 1: Load SDK/NDK/Java paths
:: ====================================================================
if not exist "..\local.properties" (
    echo [ERROR] local.properties file not found.
    exit /b 1
)
for /f "usebackq eol=# tokens=1,* delims==" %%G in ("..\local.properties") do (
    if /i "%%G"=="sdk.dir"   set "ANDROID_SDK_ROOT=%%H"
    if /i "%%G"=="ndk.dir"   set "ANDROID_NDK_HOME=%%H"
    if /i "%%G"=="java.home" set "JAVA_HOME=%%H"
)

set "PLATFORM_TOOLS_DIR=%ANDROID_SDK_ROOT%\platform-tools"
set "BUILD_TOOLS_DIR=%ANDROID_SDK_ROOT%\build-tools\%BUILD_TOOLS_VERSION%"
set "PATH=%JAVA_HOME%\bin;%BUILD_TOOLS_DIR%;%PLATFORM_TOOLS_DIR%;%PATH%"

set "SEVENZIP=C:\Program Files\7-Zip\7z.exe"
set "LLVM_STRIP=%ANDROID_NDK_HOME%\toolchains\llvm\prebuilt\windows-x86_64\bin\llvm-strip.exe"

if not exist "%LLVM_STRIP%" (
    echo [ERROR] llvm-strip.exe not found at "%LLVM_STRIP%"
    exit /b 1
)

:: ====================================================================
:: Step 2: Clean / Prepare build directories
:: ====================================================================
echo [INFO] Preparing build directories...
if "%CLEAN_BUILD%"=="true" (
    echo [INFO] Cleaning existing build folders...
    if exist "..\build"          rmdir /s /q "..\build"
    if exist "..\app\build-temp" rmdir /s /q "..\app\build-temp"
    exit /b 0
)

if not exist "..\build"          mkdir "..\build"
if not exist "..\app\build-temp" mkdir "..\app\build-temp"

:: ====================================================================
:: Step 3: Build Native Code
:: ====================================================================
echo [INFO] Building native code for ABI=%ABI%...
set "BUILD_DIR=..\build\native\%ABI%"
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
pushd "%BUILD_DIR%"
cmake ..\..\.. ^
    -DCMAKE_TOOLCHAIN_FILE="%ANDROID_NDK_HOME%\build\cmake\android.toolchain.cmake" ^
    -DANDROID_PLATFORM=%NDK_PLATFORM% ^
    -DANDROID_ABI=%ABI% ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -G Ninja || (
        echo [ERROR] CMake configuration failed
        popd
        exit /b 1
    )
ninja || (
    echo [ERROR] Native build failed
    popd
    exit /b 1
)
popd

:: ====================================================================
:: Step 3.5: Strip Native Libraries for Release
:: ====================================================================
if "%BUILD_TYPE%"=="Release" (
    echo [INFO] Stripping native libs...
    set "LIB_SO=%BUILD_DIR%\libmain.so"
    if exist "%LIB_SO%" (
        "%LLVM_STRIP%" --strip-unneeded "%LIB_SO%" || (
            echo [WARNING] Failed to strip %LIB_SO%
        )
    ) else (
        echo [WARNING] %LIB_SO% not found—skipping strip.
    )
)
:: ============================================================================
:: Steps 4,5,6,7: Java compile, DEX, resources, packaging, signing
:: ============================================================================
echo [INFO] Compiling Java code...
set "SCRIPT_DIR=%~dp0"
set "PROJECT_DIR=%SCRIPT_DIR%.."
set "JAVA_SRC=%PROJECT_DIR%\src\main\java"
set "JAVA_OUT=%PROJECT_DIR%\app\build-temp\java_classes"
set "DEX_OUT=%PROJECT_DIR%\app\build-temp"
set "SDL_JAR=%PROJECT_DIR%\lib\classes.jar"
set "ANDROID_JAR=%ANDROID_SDK_ROOT%\platforms\%NDK_PLATFORM%\android.jar"
set "D8_JAR=%BUILD_TOOLS_DIR%\lib\d8.jar"

if not exist "%JAVA_OUT%" mkdir "%JAVA_OUT%"
"%JAVA_HOME%\bin\javac" --release 8 -d "%JAVA_OUT%" -classpath "%ANDROID_JAR%;%SDL_JAR%" "%JAVA_SRC%\com\example\myapp\MainActivity.java" || (
    echo [ERROR] Java compilation failed
    exit /b 1
)

echo [INFO] Creating classes.jar...
pushd "%JAVA_OUT%"
jar cf "%DEX_OUT%\classes.jar" . || (
    echo [ERROR] Failed to create classes.jar
    popd
    exit /b 1
)
popd

echo [INFO] Converting to DEX
"%JAVA_HOME%\bin\java" -cp "%D8_JAR%" com.android.tools.r8.D8 --output "%DEX_OUT%" --lib "%ANDROID_JAR%" "%DEX_OUT%\classes.jar" "%SDL_JAR%" || (
    echo [ERROR] Dex conversion failed
    exit /b 1
)

echo [INFO] Compiling resources
if not exist "%PROJECT_DIR%\app\build-temp\compiled_resources"    mkdir "%PROJECT_DIR%\app\build-temp\compiled_resources"
if not exist "%PROJECT_DIR%\app\build-temp\linked_resources"      mkdir "%PROJECT_DIR%\app\build-temp\linked_resources"
"%BUILD_TOOLS_DIR%\aapt2.exe" compile --dir "%PROJECT_DIR%\src\main\res" -o "%PROJECT_DIR%\app\build-temp\compiled_resources\res.zip" || (
    echo [ERROR] Resource compilation failed
    exit /b 1
)
"%BUILD_TOOLS_DIR%\aapt2.exe" link -I "%ANDROID_JAR%" -o "%PROJECT_DIR%\app\build-temp\linked_resources\base.apk" --manifest "%PROJECT_DIR%\src\main\AndroidManifest.xml" --java "%PROJECT_DIR%\app\build-temp\linked_resources" --min-sdk-version 21 --target-sdk-version 31 --auto-add-overlay "%PROJECT_DIR%\app\build-temp\compiled_resources\res.zip" || (
    echo [ERROR] Resource linking failed
    exit /b 1
)

echo [INFO] Packaging APK
set "TEMP=%PROJECT_DIR%\app\build-temp\apk_temp"
if exist "%TEMP%" rmdir /s /q "%TEMP%"
mkdir "%TEMP%"

"%SEVENZIP%" x "%PROJECT_DIR%\app\build-temp\linked_resources\base.apk" -o"%TEMP%" -y >nul
copy "%DEX_OUT%\classes.dex" "%TEMP%" >nul

mkdir "%TEMP%\lib\%ABI%" >nul 2>&1
copy "%PROJECT_DIR%\build\native\%ABI%\libmain.so"          "%TEMP%\lib\%ABI%\" >nul
copy "%PROJECT_DIR%\lib\android\android.%ABI%\libSDL3.so"   "%TEMP%\lib\%ABI%\" >nul
copy "%PROJECT_DIR%\lib\SDL3_image\libs\android.%ABI%\libSDL3_image.so" "%TEMP%\lib\%ABI%\" >nul
copy "%PROJECT_DIR%\lib\SDL3_ttf\libs\android.%ABI%\libSDL3_ttf.so"     "%TEMP%\lib\%ABI%\" >nul

echo [INFO] Copying assets
if not exist "%TEMP%\assets" mkdir "%TEMP%\assets"
xcopy "%PROJECT_DIR%\src\main\assets" "%TEMP%\assets\" /E /I /Y >nul

pushd "%TEMP%"
"%JAVA_HOME%\bin\jar" cf0 "%PROJECT_DIR%\build\XenonUI-unsigned.apk" * >nul
popd

echo [INFO] Aligning APK
"%BUILD_TOOLS_DIR%\zipalign.exe" -f -p 4 "%PROJECT_DIR%\build\XenonUI-unsigned.apk" "%PROJECT_DIR%\build\XenonUI-unsigned-aligned.apk" || (
    echo [ERROR] APK alignment failed
    exit /b 1
)

echo [INFO] Signing APK
set "KEYSTORE=%PROJECT_DIR%\keystore\release.keystore"
set "FINAL=%PROJECT_DIR%\build\final.apk"
if not exist "%KEYSTORE%" (
    echo [WARNING] Keystore not found. Creating one
    mkdir "%PROJECT_DIR%\keystore"
    "%JAVA_HOME%\bin\keytool" -genkey -v -keystore "%KEYSTORE%" -alias my-key-alias -keyalg RSA -keysize 2048 -validity 10000
)

set /p "KS_PASS=Enter Keystore Password: "
set /p "KEY_PASS=Enter Key Alias Password (press Enter if same as Keystore): "
if not defined KEY_PASS set "KEY_PASS=%KS_PASS%"

"%JAVA_HOME%\bin\java" -jar "%BUILD_TOOLS_DIR%\lib\apksigner.jar" sign --ks "%KEYSTORE%" --ks-pass "pass:%KS_PASS%" --key-pass "pass:%KEY_PASS%" --out "%FINAL%" "%PROJECT_DIR%\build\XenonUI-unsigned-aligned.apk" || (
    echo [ERROR] APK signing failed
    exit /b 1
)

if exist "%TEMP%" rmdir /s /q "%TEMP%"

echo.
echo [SUCCESS] Build finished: ABI=%ABI%, Type=%BUILD_TYPE% at %DATE% %TIME%
goto :eof


:: Show available options
:show_help
echo Usage: build.bat [--debug^|--release] [--clean] [--abi ^<abi-name^>]
echo.
echo   --debug       Build in Debug mode
echo   --release     Build in Release mode (default)
echo   --clean       Delete build output and exit
echo   --abi ^<name^>  Target ABI (e.g., armeabi-v7a, arm64-v8a, x86). Default: arm64-v8a
goto :eof

:eof
exit /b 0














