
#!/bin/bash


# SPDX-License-Identifier: Apache-2.0
#
#
# Copyright (C) 2025 MD S M Sarowar Hossain
#
#
#
#
## --- Strict Mode & Debugging ---
set -e
set -x
# set -u
set -o pipefail

echo "-------------------------------------"
echo "[INFO] Starting Android APK Packaging Script (Assumes Native Libs are Prebuilt by CMake)"
echo "-------------------------------------"

# --- Configuration & Initial Checks ---

# 1. ABI Check
echo "[CHECK] Verifying ABI argument..."
if [ -z "$1" ]; then
    echo "❌ [ERROR] Usage: $0 <ABI>"
    echo "  Supported ABIs: arm64-v8a, armeabi-v7a, x86, x86_64"
    exit 1
fi
TARGET_ABI="$1"
echo "[INFO] Target ABI: $TARGET_ABI"
case "$TARGET_ABI" in
    arm64-v8a|armeabi-v7a|x86|x86_64) echo "[OK] ABI '$TARGET_ABI' is valid." ;;
    *) echo "❌ [ERROR] Invalid ABI specified: '$TARGET_ABI'. Supported: arm64-v8a, armeabi-v7a, x86, x86_64"; exit 1 ;;
esac
# 2. Environment Variables (SDK needed, NDK less critical for this script directly)
# ... ANDROID_SDK_ROOT is essential ...
echo "[CHECK] Verifying Android SDK environment variable..."
# Use the provided SDK path OR a more standard default OR error if unset/empty

: "${ANDROID_SDK_ROOT:=$HOME/Android/Sdk}"
if [ -z "$ANDROID_SDK_ROOT" ]; then
    echo "❌ [ERROR] ANDROID_SDK_ROOT environment variable is not set."
    echo "   Please set it to your Android SDK installation path (e.g., export ANDROID_SDK_ROOT=/path/to/sdk)."
    exit 1
fi
echo "[INFO] Using ANDROID_SDK_ROOT: $ANDROID_SDK_ROOT"
if [ ! -d "$ANDROID_SDK_ROOT" ]; then
    echo "❌ [ERROR] ANDROID_SDK_ROOT directory not found at specified path: $ANDROID_SDK_ROOT"
    exit 1
fi
if [ ! -d "$ANDROID_SDK_ROOT/build-tools/34.0.0/" ] || [ -z "$(ls -A $ANDROID_SDK_ROOT/build-tools/34.0.0/)" ]; then
    echo "❌ [ERROR] No build-tools found within ANDROID_SDK_ROOT: $ANDROID_SDK_ROOT/build-tools/34.0.0/"
    echo "   Ensure you have installed build-tools via the Android SDK Manager."
    exit 1
fi
if [ ! -d "$ANDROID_SDK_ROOT/platforms" ] || [ -z "$(ls -A $ANDROID_SDK_ROOT/platforms)" ]; then
    echo "❌ [ERROR] No platforms found within ANDROID_SDK_ROOT: $ANDROID_SDK_ROOT/platforms"
    echo "   Ensure you have installed platform SDKs (like android-21) via the Android SDK Manager."
    exit 1
fi
echo "[OK] ANDROID_SDK_ROOT directory exists and seems valid (contains build-tools, platforms)."


# 3. Project Structure & Paths
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PROJECT_ROOT="$SCRIPT_DIR/.." # Assumes script is in a subdir
OUTPUT_DIR="$PROJECT_ROOT/output_apk"
MANIFEST_PATH="$PROJECT_ROOT/AndroidManifest.xml"
RES_DIR="$PROJECT_ROOT/res"
ASSETS_DIR="$PROJECT_ROOT/src/main/assets" 
KEYSTORE_PATH="$PROJECT_ROOT/debug.keystore"


JAVA_SRC_DIR="$PROJECT_ROOT/src/main/java"
# Temporary directory for compiled custom Java .class files
JAVA_CLASSES_OUT_DIR="$OUTPUT_DIR/compiled_app_classes"

APP_CLASSES_JAR="$OUTPUT_DIR/compiled_app_classes.jar"

# Path to CMake build output directory
CMAKE_BUILD_DIR="$PROJECT_ROOT/build" # This is where CMake was configured to build

echo "[INFO] Project Root: $PROJECT_ROOT"
echo "[INFO] CMake Build Output Directory (for libmain.so): $CMAKE_BUILD_DIR"
echo "[INFO] Output directory (APK staging): $OUTPUT_DIR"


# 4. SDL3 Paths (for PREBUILT .so and classes.jar to be PACKAGED)
echo "[CHECK] Verifying SDL3 paths for packaging..."
SDL3_PREBUILT_ROOT="$PROJECT_ROOT/jni/SDL3" 
SDL3_NATIVE_LIB_DIR="$SDL3_PREBUILT_ROOT/lib/android.${TARGET_ABI}"
SDL3_NATIVE_LIB_NAME="libSDL3.so"
SDL3_SO_SRC="$SDL3_NATIVE_LIB_DIR/$SDL3_NATIVE_LIB_NAME"

# SDL3_image paths for PREBUILT .so to be PACKAGED
SDL3_IMAGE_PREBUILT_DIR_NAME="SDL3_image-3.2.4"
SDL3_IMAGE_NATIVE_LIB_DIR="$SDL3_PREBUILT_ROOT/$SDL3_IMAGE_PREBUILT_DIR_NAME/libs/android.${TARGET_ABI}" 
SDL3_IMAGE_NATIVE_LIB_NAME="libSDL3_image.so"
SDL3_IMAGE_SO_SRC="$SDL3_IMAGE_NATIVE_LIB_DIR/$SDL3_IMAGE_NATIVE_LIB_NAME"


# CMake path: ${SDL3_PREBUILT_ROOT}/SDL3_ttf/libs/android.${CMAKE_ANDROID_ARCH_ABI}/libSDL3_ttf.so
SDL3_TTF_SUBDIR="SDL3_ttf" 
SDL3_TTF_NATIVE_LIB_INTERMEDIATE_DIR="$SDL3_PREBUILT_ROOT/$SDL3_TTF_SUBDIR/libs/android.${TARGET_ABI}" # Path to ABI specific libs for SDL_ttf
SDL3_TTF_NATIVE_LIB_NAME="libSDL3_ttf.so"
SDL3_TTF_SO_SRC="$SDL3_TTF_NATIVE_LIB_INTERMEDIATE_DIR/$SDL3_TTF_NATIVE_LIB_NAME"



# Check for SDL3 .so
if [ ! -f "$SDL3_SO_SRC" ]; then echo "❌ [ERROR] Prebuilt SDL3 native library file '$SDL3_NATIVE_LIB_NAME' not found: $SDL3_SO_SRC"; exit 1; fi
echo "[OK] Found prebuilt SDL3 native library file: $SDL3_SO_SRC"

# Check for SDL3_image .so
if [ ! -f "$SDL3_IMAGE_SO_SRC" ]; then echo "❌ [ERROR] Prebuilt SDL3_image native library file '$SDL3_IMAGE_NATIVE_LIB_NAME' not found: $SDL3_IMAGE_SO_SRC"; exit 1; fi
echo "[OK] Found prebuilt SDL3_image native library file: $SDL3_IMAGE_SO_SRC"

# Check for SDL3_ttf .so
if [ ! -f "$SDL3_TTF_SO_SRC" ]; then
    echo "❌ [ERROR] Prebuilt SDL3_ttf native library file '$SDL3_TTF_NATIVE_LIB_NAME' not found: $SDL3_TTF_SO_SRC"
    echo "   Ensure it exists at this path, prebuilt for ABI '$TARGET_ABI'."
    exit 1
fi
echo "[OK] Found prebuilt SDL3_ttf native library file: $SDL3_TTF_SO_SRC"

# SDL Java classes (classes.jar) - this path is for packaging
SDL_ANDROID_CLASSES_SRC="$SDL3_PREBUILT_ROOT/classes.jar" # Path to SDL's classes.jar
if [ ! -f "$SDL_ANDROID_CLASSES_SRC" ]; then echo "❌ [ERROR] SDL Android Classes JAR not found: '$SDL_ANDROID_CLASSES_SRC'."; exit 1; fi
echo "[OK] Found SDL Android Classes JAR: $SDL_ANDROID_CLASSES_SRC"


# 5. APK Details
APK_NAME="${TARGET_ABI}.apk"
PACKAGE_NAME="org.sarowar.sdlnativeapp" # Must match AndroidManifest.xml
MIN_SDK=21
TARGET_SDK=31 # AAPT2 target SDK
BUILD_TOOLS_VERSION="34.0.0"

echo "[INFO] APK Name: $APK_NAME"
echo "[INFO] Package Name: $PACKAGE_NAME"
echo "[INFO] Minimum SDK Version: $MIN_SDK"
echo "[INFO] Target SDK Version (for AAPT2): $TARGET_SDK"
echo "[INFO] Build Tools Version: $BUILD_TOOLS_VERSION"

# 6. Tool Paths
echo "[CHECK] Locating Android SDK build tools..."
find_build_tool() {
    local tool_name=$1; local sdk_root=$2; local build_tools_ver=$3; local tool_path
    local specific_path="$sdk_root/build-tools/$build_tools_ver/$tool_name"
    if [ -f "$specific_path" ] && [ -x "$specific_path" ]; then echo "$specific_path"; return 0; fi
    echo "⚠️ [WARNING] Tool '$tool_name' not in build-tools '$build_tools_ver'. Searching all..." >&2
    tool_path=$(find "$sdk_root/build-tools/" -maxdepth 2 -name "$tool_name" -executable -print -quit)
    if [ -z "$tool_path" ]; then echo "❌ [ERROR] Cannot find '$tool_name' in $sdk_root/build-tools/" >&2; return 1; fi
    echo "ℹ️ [INFO] Using '$tool_name' found at: $tool_path" >&2; echo "$tool_path"
}

APKSIGNER=$(find_build_tool apksigner "$ANDROID_SDK_ROOT" "$BUILD_TOOLS_VERSION") || exit 1
D8=$(find_build_tool d8 "$ANDROID_SDK_ROOT" "$BUILD_TOOLS_VERSION") || exit 1
AAPT2=$(find_build_tool aapt2 "$ANDROID_SDK_ROOT" "$BUILD_TOOLS_VERSION") || exit 1
ZIPALIGN=$(find_build_tool zipalign "$ANDROID_SDK_ROOT" "$BUILD_TOOLS_VERSION") || exit 1
JAVAC=$(command -v javac) # Find javac in PATH
if [ -z "$JAVAC" ]; then echo "❌ [ERROR] javac (Java Development Kit) not found in PATH."; exit 1; fi


echo "[OK] Found required build tools: apksigner, d8, aapt2, zipalign, javac."
echo "   APKSIGNER: $APKSIGNER"; echo "   D8: $D8"; echo "   AAPT2: $AAPT2"; echo "   ZIPALIGN: $ZIPALIGN"; echo "   JAVAC: $JAVAC"

echo "[CHECK] Locating Android Platform JAR..."
PLATFORM_API_LEVEL_FOR_JAR=$TARGET_SDK
ANDROID_JAR="$ANDROID_SDK_ROOT/platforms/android-$PLATFORM_API_LEVEL_FOR_JAR/android.jar"
# Fallback logic for ANDROID_JAR
if [ ! -f "$ANDROID_JAR" ]; then echo "⚠️ [WARNING] Platform JAR for API $PLATFORM_API_LEVEL_FOR_JAR not found. Trying API 34..."; ANDROID_JAR="$ANDROID_SDK_ROOT/platforms/android-34/android.jar"; fi
if [ ! -f "$ANDROID_JAR" ]; then echo "⚠️ [WARNING] Platform JAR for API 34 not found. Trying API 33..."; ANDROID_JAR="$ANDROID_SDK_ROOT/platforms/android-33/android.jar"; fi
if [ ! -f "$ANDROID_JAR" ]; then echo "⚠️ [WARNING] Platform JAR for API 33 not found. Trying API 31..."; ANDROID_JAR="$ANDROID_SDK_ROOT/platforms/android-31/android.jar"; fi

echo "[INFO] Using Android Platform JAR: $ANDROID_JAR"
if [ ! -f "$ANDROID_JAR" ]; then echo "❌ [ERROR] Android Platform JAR ('android.jar') not found. Checked for API $PLATFORM_API_LEVEL_FOR_JAR and fallbacks."; exit 1; fi
if [ ! -r "$ANDROID_JAR" ]; then echo "❌ [ERROR] Android Platform JAR '$ANDROID_JAR' is not readable."; exit 1; fi
echo "[OK] Found Android Platform JAR and it is readable."

# 7. Final Pre-Build Sanity Check
echo "-------------------------------------"; echo "[OK] All prerequisite checks passed."; echo "-------------------------------------"

# --- START BUILD ---
echo "[STEP 1/11] Clean and Create Output Structure"
echo "🧹 Cleaning previous build output directory: $OUTPUT_DIR"; rm -rf "$OUTPUT_DIR"
echo "[INFO] Creating output directories..."
mkdir -p "$OUTPUT_DIR/lib/$TARGET_ABI"; if [ ! -d "$OUTPUT_DIR/lib/$TARGET_ABI" ]; then echo "❌ ERROR creating dir"; exit 1; fi
mkdir -p "$OUTPUT_DIR/dex_files"; if [ ! -d "$OUTPUT_DIR/dex_files" ]; then echo "❌ ERROR creating dir"; exit 1; fi
STAGED_ASSETS_DIR="$OUTPUT_DIR/apk_staging/assets"
mkdir -p "$STAGED_ASSETS_DIR"; if [ ! -d "$STAGED_ASSETS_DIR" ]; then echo "❌ ERROR creating dir"; exit 1; fi
R_JAVA_DIR="$OUTPUT_DIR/gen_r_java" # For R.java generated by AAPT2
R_CLASSES_DIR="$OUTPUT_DIR/compiled_r_classes" # For compiled R.class files
mkdir -p "$R_JAVA_DIR"; if [ ! -d "$R_JAVA_DIR" ]; then echo "❌ ERROR creating dir"; exit 1; fi
mkdir -p "$R_CLASSES_DIR"; if [ ! -d "$R_CLASSES_DIR" ]; then echo "❌ ERROR creating dir"; exit 1; fi

mkdir -p "$JAVA_CLASSES_OUT_DIR"; if [ ! -d "$JAVA_CLASSES_OUT_DIR" ]; then echo "❌ ERROR creating dir"; exit 1; fi
mkdir -p "$RES_DIR"; if [ ! -d "$RES_DIR" ]; then echo "❌ ERROR creating dir"; exit 1; fi
mkdir -p "$RES_DIR/mipmap-hdpi"
echo "[OK] Output directories created."

# --- START BUILD ---


echo "[STEP 2/11] Copy Native Libraries (.so)"
# libmain.so comes from CMake build output
MAIN_SO_SRC_CMAKE="$CMAKE_BUILD_DIR/lib/$TARGET_ABI/libmain.so"
echo "[INFO] Looking for app's native library (from CMake): $MAIN_SO_SRC_CMAKE"
if [ ! -f "$MAIN_SO_SRC_CMAKE" ]; then
    echo "❌ [ERROR] App's native library ('libmain.so') not found at '$MAIN_SO_SRC_CMAKE'."
    echo "   This file MUST be generated by the CMake build process first for ABI '$TARGET_ABI'."
    echo "   Example CMake command:"
    echo "   cmake -S \"$PROJECT_ROOT\" -B \"$CMAKE_BUILD_DIR\" -DANDROID_ABI=$TARGET_ABI -DANDROID_PLATFORM=$MIN_SDK ..."
    echo "   cmake --build \"$CMAKE_BUILD_DIR\""
    exit 1
fi
if [ ! -r "$MAIN_SO_SRC_CMAKE" ]; then echo "❌ [ERROR] Main library file '$MAIN_SO_SRC_CMAKE' is not readable."; exit 1; fi
echo "[OK] Found app's native library: $MAIN_SO_SRC_CMAKE"

echo "[INFO] Copying main library (libmain.so)..."; cp "$MAIN_SO_SRC_CMAKE" "$OUTPUT_DIR/lib/$TARGET_ABI/libmain.so" || { echo "❌ ERROR copying libmain.so"; exit 1; }
echo "[OK] Copied libmain.so."

echo "[INFO] Copying prebuilt SDL3 library ($SDL3_NATIVE_LIB_NAME)..."; cp "$SDL3_SO_SRC" "$OUTPUT_DIR/lib/$TARGET_ABI/$SDL3_NATIVE_LIB_NAME" || { echo "❌ ERROR copying $SDL3_NATIVE_LIB_NAME"; exit 1; }
echo "[OK] Copied $SDL3_NATIVE_LIB_NAME."

echo "[INFO] Copying prebuilt SDL3_image library ($SDL3_IMAGE_NATIVE_LIB_NAME)..."; cp "$SDL3_IMAGE_SO_SRC" "$OUTPUT_DIR/lib/$TARGET_ABI/$SDL3_IMAGE_NATIVE_LIB_NAME" || { echo "❌ ERROR copying $SDL3_IMAGE_NATIVE_LIB_NAME"; exit 1; }
echo "[OK] Copied $SDL3_IMAGE_NATIVE_LIB_NAME."



echo "[INFO] Copying prebuilt SDL3_ttf library ($SDL3_TTF_NATIVE_LIB_NAME)..."
cp "$SDL3_TTF_SO_SRC" "$OUTPUT_DIR/lib/$TARGET_ABI/$SDL3_TTF_NATIVE_LIB_NAME" || { echo "❌ ERROR copying $SDL3_TTF_NATIVE_LIB_NAME"; exit 1; }
echo "[OK] Copied $SDL3_TTF_NATIVE_LIB_NAME."


echo "[STEP 3/11] Copy Assets (Optional)"
mkdir -p "$STAGED_ASSETS_DIR" 
if [ -d "$ASSETS_DIR" ] && [ "$(ls -A "$ASSETS_DIR")" ]; then
    echo "[INFO] Copying assets from '$ASSETS_DIR' to staging: $STAGED_ASSETS_DIR"
    cp -r "$ASSETS_DIR"/* "$STAGED_ASSETS_DIR/" || { echo "❌ ERROR copying assets"; exit 1; }
    if [ -z "$(ls -A "$STAGED_ASSETS_DIR")" ]; then echo "⚠️ WARNING: Asset staging dir empty after copy."; fi
    echo "[OK] Assets copied to staging area."
else
    echo "ℹ️ [INFO] No assets found in '$ASSETS_DIR' or directory empty. '$STAGED_ASSETS_DIR' will be empty for AAPT2."

fi

echo "[STEP 4/11] Compile Resources (AAPT2 compile)"
COMPILED_RES_ZIP="$OUTPUT_DIR/compiled_resources.zip"
if [ -d "$RES_DIR" ] && [ "$(ls -A "$RES_DIR")" ]; then
    "$AAPT2" compile --dir "$RES_DIR" -o "$COMPILED_RES_ZIP" --no-crunch || { echo "❌ ERROR: AAPT2 compile failed."; exit 1; }
    if [ ! -f "$COMPILED_RES_ZIP" ]; then echo "❌ ERROR: AAPT2 compiled_resources.zip not found."; exit 1; fi
    echo "[OK] AAPT2 resource compilation successful: $COMPILED_RES_ZIP"
else
    echo "ℹ️ [INFO] Resources directory '$RES_DIR' empty/missing. No app-specific resources compiled."
    COMPILED_RES_ZIP=""
fi

 echo "[STEP 5/11] Link Resources & Generate R.java (AAPT2 link)"
 INTERMEDIATE_UNSIGNED_APK="$OUTPUT_DIR/intermediate_unsigned_${APK_NAME}"
 declare -a AAPT2_LINK_ARGS
 AAPT2_LINK_ARGS=(
     "$AAPT2" link
     -o "$INTERMEDIATE_UNSIGNED_APK"
     -I "$ANDROID_JAR"
     -A "$STAGED_ASSETS_DIR"
     --manifest "$MANIFEST_PATH"
     --min-sdk-version "21"
     --target-sdk-version "$TARGET_SDK"
     --java "$R_JAVA_DIR"
     --auto-add-overlay
     --no-compress xml                   # ensure XML files (including manifest) are stored uncompressed
     -v
 )
if [ -n "$COMPILED_RES_ZIP" ] && [ -f "$COMPILED_RES_ZIP" ]; then AAPT2_LINK_ARGS+=(-R "$COMPILED_RES_ZIP"); else echo "ℹ️ INFO: No compiled resources ZIP for AAPT2 link."; fi
echo "Executing AAPT2 Link: ${AAPT2_LINK_ARGS[*]}"
"${AAPT2_LINK_ARGS[@]}" || { echo "❌ ERROR: AAPT2 link failed."; exit 1; }
if [ ! -f "$INTERMEDIATE_UNSIGNED_APK" ] || [ ! -s "$INTERMEDIATE_UNSIGNED_APK" ]; then echo "❌ ERROR: AAPT2 link did not produce valid APK: $INTERMEDIATE_UNSIGNED_APK"; exit 1; fi
echo "[OK] AAPT2 linking successful. Intermediate APK: $INTERMEDIATE_UNSIGNED_APK. R.java generated in '$R_JAVA_DIR'."



echo "[DEBUG] Inspecting APK after AAPT2 link..."
cp "$INTERMEDIATE_UNSIGNED_APK" "$OUTPUT_DIR/apk_after_aapt2.zip"
# Use Android SDK's apkanalyzer
"$ANDROID_SDK_ROOT/cmdline-tools/latest/bin/apkanalyzer" manifest print "$INTERMEDIATE_UNSIGNED_APK" || echo "❌ [DEBUG] Failed to print manifest after aapt2 link"


echo "[STEP 6/11] Compile Generated R.java files"
R_JAVA_FILES=$(find "$R_JAVA_DIR" -name "*.java")
if [ -n "$R_JAVA_FILES" ]; then
    echo "[INFO] Compiling R.java files: $R_JAVA_FILES"
    
    "$JAVAC" -d "$R_CLASSES_DIR" \
          -classpath "$ANDROID_JAR:$SDL_ANDROID_CLASSES_SRC" \
          -source 1.8 -target 1.8 \
          $R_JAVA_FILES || { echo "❌ ERROR: javac failed to compile R.java files."; exit 1; }
    if [ -z "$(ls -A "$R_CLASSES_DIR")" ]; then echo "⚠️ WARNING: R_CLASSES_DIR is empty after javac. Check R.java generation and compilation."; fi
    echo "[OK] Compiled R.java files to $R_CLASSES_DIR"
else
    echo "ℹ️ [INFO] No R.java files found to compile in $R_JAVA_DIR. This is okay if no resources are defined."
fi



echo "[STEP 7/12] Compile App's Java Source Code"
APP_JAVA_FILES=$(find "$JAVA_SRC_DIR" -name "*.java")
if [ -n "$APP_JAVA_FILES" ]; then
    echo "[INFO] Found app Java source files to compile."
    "$JAVAC" -d "$JAVA_CLASSES_OUT_DIR" \
             -classpath "$ANDROID_JAR:$SDL_ANDROID_CLASSES_SRC:$R_CLASSES_DIR" \
             -source 1.8 -target 1.8 \
             $APP_JAVA_FILES || { echo "❌ ERROR: javac failed to compile app source files."; exit 1; }
    echo "[OK] App Java source compiled to $JAVA_CLASSES_OUT_DIR"
else
    echo "ℹ️  [INFO] No app Java source files found in '$JAVA_SRC_DIR', skipping compilation."
fi



echo "[STEP 7/11] Compile All Java classes (SDL + R.java) to DEX format (D8)"
D8_INPUTS=()
if [ -e "$SDL_ANDROID_CLASSES_SRC" ]; then D8_INPUTS+=("$SDL_ANDROID_CLASSES_SRC"); else echo "❌ ERROR: SDL_ANDROID_CLASSES_SRC invalid for D8."; exit 1; fi
if [ -d "$R_CLASSES_DIR" ] && [ "$(ls -A "$R_CLASSES_DIR")" ]; then
    D8_INPUTS+=("$R_CLASSES_DIR")
    echo "[INFO] Adding compiled R classes from '$R_CLASSES_DIR' to D8 input."
else
    echo "ℹ️ [INFO] No compiled R classes in '$R_CLASSES_DIR' to add to D8 input."
fi

dex_files="$SCRIPT_DIR/output/dex_output"
R_CLASSES_BASE_DIR="$OUTPUT_DIR/compiled_r_classes"
R_CLASSES_JAR="$OUTPUT_DIR/compiled_r_classes.jar"
DEX_OUTPUT_ACTUAL_DIR="$OUTPUT_DIR/dex_files" # Use the one created in STEP 1

if [ -d "$R_CLASSES_BASE_DIR" ]; then
    echo "[INFO] Creating JAR from R classes..."
    (cd "$R_CLASSES_BASE_DIR" && jar cf "$R_CLASSES_JAR" org) || { echo "❌ Failed to create R JAR."; exit 1; }
else
    echo "❌ ERROR: R class folder not found: $R_CLASSES_BASE_DIR"
    exit 1
fi


echo "[STEP 8/12] Package App Classes and Compile All to DEX (D8)"


echo "[INFO] Creating JAR from app classes..."
if [ -d "$JAVA_CLASSES_OUT_DIR" ] && [ "$(ls -A "$JAVA_CLASSES_OUT_DIR")" ]; then
    (cd "$JAVA_CLASSES_OUT_DIR" && jar cf "$APP_CLASSES_JAR" .) || { echo "❌ Failed to create App Classes JAR."; exit 1; }
else
    echo "ℹ️  [INFO] No compiled app classes found to create a JAR, skipping."
    # Create an empty file so the D8 command doesn't fail on a missing path
    touch "$APP_CLASSES_JAR"
fi

# run D8 using the newly created JAR file
echo "[INFO] Running D8 with SDL, R, and App JARs..."
"$D8" --output "$DEX_OUTPUT_ACTUAL_DIR" \
     --lib "$ANDROID_JAR" \
     "$SDL_ANDROID_CLASSES_SRC" \
     "$R_CLASSES_JAR" \
     "$APP_CLASSES_JAR" || { echo "❌ ERROR: D8 command failed."; exit 1; }

echo "[OK] D8 processing successful."



# ...
echo "[STEP 8/11] Add DEX files and Native Libraries to the APK archive"
echo "[INFO] Adding DEX files to '$INTERMEDIATE_UNSIGNED_APK'..."
(cd "$DEX_OUTPUT_ACTUAL_DIR" && zip -ur "$INTERMEDIATE_UNSIGNED_APK" classes*.dex) || { echo "❌ ERROR: Failed to add DEX files to APK."; exit 1; } # classes*.dex for potential multidex
if command -v unzip > /dev/null; then if ! unzip -l "$INTERMEDIATE_UNSIGNED_APK" | grep -q 'classes.dex'; then echo "❌ ERROR: classes.dex not found in APK after zipping."; exit 1; fi; fi
echo "[OK] Added DEX files."

echo "[INFO] Adding native libraries (lib/) to '$INTERMEDIATE_UNSIGNED_APK'..."
(cd "$OUTPUT_DIR" && zip -ur "$INTERMEDIATE_UNSIGNED_APK" lib/) || { echo "❌ ERROR: Failed to add native libraries to APK."; exit 1; }
if command -v unzip > /dev/null; then
    if ! unzip -l "$INTERMEDIATE_UNSIGNED_APK" | grep -q "lib/$TARGET_ABI/libmain.so"; then echo "❌ ERROR: libmain.so not found in APK."; exit 1; fi
    if ! unzip -l "$INTERMEDIATE_UNSIGNED_APK" | grep -q "lib/$TARGET_ABI/$SDL3_NATIVE_LIB_NAME"; then echo "❌ ERROR: $SDL3_NATIVE_LIB_NAME not found in APK."; exit 1; fi
    if ! unzip -l "$INTERMEDIATE_UNSIGNED_APK" | grep -q "lib/$TARGET_ABI/$SDL3_TTF_NATIVE_LIB_NAME"; then echo "❌ ERROR: $SDL3_TTF_NATIVE_LIB_NAME not found in APK."; exit 1; fi
fi
echo "[OK] Added native libraries."

echo "[STEP 9/11] Align the APK (Zipalign)"
UNSIGNED_ALIGNED_APK="$OUTPUT_DIR/unsigned_aligned_${APK_NAME}"
echo "[INFO] Running Zipalign on '$INTERMEDIATE_UNSIGNED_APK'..."
"$ZIPALIGN" -f -p 4 "$INTERMEDIATE_UNSIGNED_APK" "$UNSIGNED_ALIGNED_APK" || { echo "❌ ERROR: Zipalign failed."; exit 1; }
if [ ! -f "$UNSIGNED_ALIGNED_APK" ]; then echo "❌ ERROR: Zipalign did not produce aligned APK."; exit 1; fi
echo "[OK] APK alignment successful: $UNSIGNED_ALIGNED_APK"

echo "[STEP 10/11] Generate Debug Keystore (if needed) & Sign APK"
KEY_ALIAS="androiddebugkey"; KEY_PASS="android"; STORE_PASS="android"
if [ ! -f "$KEYSTORE_PATH" ]; then
    echo "[INFO] Debug keystore not found. Generating new one..."
    if ! command -v keytool > /dev/null; then echo "❌ ERROR: keytool not found."; exit 1; fi
    keytool -genkey -v -keystore "$KEYSTORE_PATH" -alias "$KEY_ALIAS" -keyalg RSA -keysize 2048 -validity 10000 -storepass "$STORE_PASS" -keypass "$KEY_PASS" -dname "CN=Android Debug,O=Android,C=US" || { echo "❌ ERROR: keytool failed."; exit 1; }
    if [ ! -f "$KEYSTORE_PATH" ]; then echo "❌ ERROR: Keystore not generated."; exit 1; fi
    echo "[OK] Debug keystore generated."
else
    echo "[INFO] Using existing debug keystore: $KEYSTORE_PATH"
    if [ ! -r "$KEYSTORE_PATH" ]; then echo "❌ ERROR: Existing keystore not readable."; exit 1; fi
fi
SIGNED_APK="$OUTPUT_DIR/$APK_NAME"
echo "[INFO] Running APKSIGNER to sign '$UNSIGNED_ALIGNED_APK'..."
"$APKSIGNER" sign --ks "$KEYSTORE_PATH" --ks-key-alias "$KEY_ALIAS" --ks-pass "pass:$STORE_PASS" --key-pass "pass:$KEY_PASS" --min-sdk-version "$MIN_SDK" --out "$SIGNED_APK" "$UNSIGNED_ALIGNED_APK" || { echo "❌ ERROR: apksigner failed."; exit 1; }
if [ ! -f "$SIGNED_APK" ] || [ ! -s "$SIGNED_APK" ]; then echo "❌ ERROR: apksigner did not produce valid signed APK."; exit 1; fi
echo "[OK] Signed APK created: $SIGNED_APK"
"$APKSIGNER" verify --min-sdk-version 21 "$SIGNED_APK" || { echo "❌ ERROR: apksigner verification failed."; exit 1; }
echo "[OK] APK signature verified."

echo "[STEP 11/11] Cleanup Intermediate Files"
echo "[INFO] Removing intermediate unsigned APK: $INTERMEDIATE_UNSIGNED_APK"; rm -f "$INTERMEDIATE_UNSIGNED_APK"
echo "[INFO] Removing unsigned aligned APK: $UNSIGNED_ALIGNED_APK"; rm -f "$UNSIGNED_ALIGNED_APK"
if [ -n "$COMPILED_RES_ZIP" ] && [ -f "$COMPILED_RES_ZIP" ]; then echo "[INFO] Removing compiled resources ZIP: $COMPILED_RES_ZIP"; rm -f "$COMPILED_RES_ZIP"; fi
echo "[INFO] Removing generated R.java directory: $R_JAVA_DIR"; rm -rf "$R_JAVA_DIR"
echo "[INFO] Removing compiled R.class directory: $R_CLASSES_DIR"; rm -rf "$R_CLASSES_DIR" # Cleanup for R.class

echo "[INFO] Removing compiled app classes directory: $JAVA_CLASSES_OUT_DIR"; rm -rf "$JAVA_CLASSES_OUT_DIR"

echo "[INFO] Removing compiled app classes JAR: $APP_CLASSES_JAR"; rm -f "$APP_CLASSES_JAR"
echo "[INFO] Removing temporary DEX directory: $OUTPUT_DIR/dex_files"; rm -rf "$OUTPUT_DIR/dex_files"
echo "[INFO] Removing staging assets directory: $STAGED_ASSETS_DIR"; rm -rf "$STAGED_ASSETS_DIR"
echo "[OK] Cleanup complete."
# ...

echo "-------------------------------------"; echo "✅✅✅ SUCCESS ✅✅✅"; echo "-------------------------------------"
echo "Signed APK created at: $SIGNED_APK"
echo "Install with: adb install -r \"$SIGNED_APK\""
echo "If crashes occur, check 'adb logcat' for errors from package '$PACKAGE_NAME'."
echo "-------------------------------------"