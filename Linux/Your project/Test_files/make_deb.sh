#
#
#
#   SPDX-License-Identifier: Apache-2.0
#
#   Copyright (C) 2025 MD S M Sarowar Hossain
#
#
#
# Description:
# This script is a Debian package builder (.deb) for the 'XenUI' application.
# It handles compilation, creates the necessary package directory structure,
# bundles the XenUI framework and its bundled SDL dependencies (SDL3, SDL3_ttf,
# SDL3_image) from the local 'tools/' directory, sets up a library loading
# wrapper script, and generates all required metadata (control file, desktop entry).
#
set -e

# --- Configuration Variables ---
APP_NAME="XenUI"
APP_VERSION="0.9.0"
ARCH="amd64"
PKG_DIR="package"
BUILD_DIR="build"
OUTPUT_DEB="${APP_NAME}_${APP_VERSION}_${ARCH}.deb"
BUNDLED_ANY=0 # Flag to track if any runtime libraries were bundled

# base dir (script location)
# --- Robust Build Block ---
# Resolve the script's absolute directory path. This is crucial for paths relative to the script.
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

echo "🔧 Building ${APP_NAME} (source: ${SCRIPT_DIR})..."
mkdir -p "$BUILD_DIR"

# Temporarily change the current directory to the build directory.
# This prevents polluting the source directory and ensures 'make' works correctly.
pushd "$BUILD_DIR" >/dev/null

# Run cmake using the explicit source directory to locate the CMakeLists.txt.
cmake "$SCRIPT_DIR"

# Build the project using all available processor cores.
make -j"$(nproc)"

# Restore the previous working directory.
popd >/dev/null
echo "✅ Build finished."
# -----------------------------------------------------------------------


echo "📦 Creating package structure..."
# Create the standard Debian package hierarchy.
mkdir -p "$PKG_DIR/DEBIAN"
mkdir -p "$PKG_DIR/usr/bin"
mkdir -p "$PKG_DIR/usr/share/applications"
mkdir -p "$PKG_DIR/usr/share/icons/hicolor/256x256/apps"
# Create the custom directory for bundled shared libraries.
mkdir -p "$PKG_DIR/usr/share/${APP_NAME}/lib"


# --- Bundle XenUI Shared Libraries ---
# Locate the pre-built XenUI shared library from the parent project's build path.
XENUI_BUILD_DIR="${SCRIPT_DIR}/../../build"
echo "📦 Bundling libXenUI from ${XENUI_BUILD_DIR} ..."

# Enable nullglob to prevent expansion to literal '*'' if no files match.
shopt -s nullglob
# Copy all versioned and unversioned libXenUI shared library files.
# The 'cp -a' command preserves symlinks if they exist, which is handled later.
# Suppress error messages if the file doesn't exist (e.g., during clean build)
cp -a "${XENUI_BUILD_DIR}"/libXenUI.so* "$PKG_DIR/usr/share/${APP_NAME}/lib/" 2>/dev/null || true
shopt -u nullglob

# Check if any libXenUI files were successfully copied.
if ls "${PKG_DIR}/usr/share/${APP_NAME}/lib"/libXenUI.so* >/dev/null 2>&1; then
  BUNDLED_ANY=1
  echo "  -> libXenUI variants bundled."
else
  echo "Warning: no libXenUI files found in ${XENUI_BUILD_DIR}; package will miss libXenUI."
fi
# ------------------------------------------------------------------------


# --- Robust External Library Lookup and Bundle Logic ---

##
# @brief Finds a system library via ldconfig cache.
#
# This function queries the ldconfig cache for the specified library name.
# It is kept for reference but is currently bypassed to enforce bundling from local tools/.
#
# @param libname The name of the library (e.g., libSDL3.so).
# @return The full path to the library or an empty string if not found.
##
find_system_lib() {
  local libname="$1"
  ldconfig -p 2>/dev/null | awk -v l="$libname" '$0 ~ l {print $NF; exit}'
}

##
# @brief Searches for a required shared library within the local project's tools/ directory.
#
# Libraries are first checked in their explicitly mapped SDL build folders,
# then a broader search within the tools/ directory is performed.
#
# @param libname The base library file name (e.g., libSDL3.so).
# @return The absolute, resolved path to the actual shared object file.
##
find_local_lib() {
  local libname="$1"
  local script_tools_dir="$SCRIPT_DIR/../../tools"

  # Associative array mapping required library file names to their expected folder names in tools/.
  declare -A TOOL_DIR_MAP=(
    ["libSDL3.so"]="SDL3-3.2.16"
    ["libSDL3_ttf.so"]="SDL3_ttf-3.2.2"
    ["libSDL3_image.so"]="SDL3_image-3.2.4"
  )

  # 1) Check the expected build/ path(s) based on the TOOL_DIR_MAP.
  local expected_subdir="${TOOL_DIR_MAP[$libname]}"
  if [ -n "$expected_subdir" ]; then
    local base="$SCRIPT_DIR/../../tools/${expected_subdir}/build"
    # Search for both the exact name and any versioned variants (e.g., libSDL3.so.0.2.16).
    for candidate in "$base/$libname" "$base/$libname"*; do
      if [ -e "$candidate" ]; then
        # Resolve symlink to the real file path before returning.
        readlink -f "$candidate"
        return 0
      fi
    done
  fi

  # 2) If not found in the mapped location, perform a broader search under tools/.
  local found
  found=$(find "$script_tools_dir" -type f -name "${libname}*" -print -quit 2>/dev/null || true)
  if [ -n "$found" ]; then
    readlink -f "$found"
    return 0
  fi

  # 3) Library file was not found in any local tools location.
  return 1
}

# List of shared libraries required for bundling from the local tools/ directory.
DECLARED_LIBS=( "libSDL3.so" "libSDL3_ttf.so" "libSDL3_image.so" )

for libfile in "${DECLARED_LIBS[@]}"; do
  echo -n "🔎 Looking for local ${libfile} in project tools to bundle... "
  localpath="$(find_local_lib "$libfile" || true)"

  if [ -n "$localpath" ]; then
    echo "found local copy at ${localpath##$SCRIPT_DIR/} . Bundling."
    # Copy the *real file* (not the symlink) into the package structure.
    cp "$localpath" "$PKG_DIR/usr/share/${APP_NAME}/lib/"
    BUNDLED_ANY=1
  else
    echo "ERROR: ${libfile} not found in project tools/ directory."
    echo "This script is set to ALWAYS bundle, so the .so files must be built or placed in the expected /tools location."
    exit 1
  fi
done
# ------------------------------------------------------------------------

# --- Copy runtime assets (Images, Fonts, etc.) ---
echo "🖼️ Copying Images and Fonts..."
# --- Copy runtime assets (Images, Fonts) if they exist ---
if [ -d "$SCRIPT_DIR/Images" ] && compgen -G "$SCRIPT_DIR/Images/*" > /dev/null; then
    echo "🖼️ Copying Images..."
    mkdir -p "$PKG_DIR/usr/share/${APP_NAME}/Images"
    cp -r "$SCRIPT_DIR/Images/"* "$PKG_DIR/usr/share/${APP_NAME}/Images/"
    chmod -R 644 "$PKG_DIR/usr/share/${APP_NAME}/Images/"*
fi

if [ -d "$SCRIPT_DIR/Fonts" ] && compgen -G "$SCRIPT_DIR/Fonts/*" > /dev/null; then
    echo "🔤 Copying Fonts..."
    mkdir -p "$PKG_DIR/usr/share/${APP_NAME}/Fonts"
    cp -r "$SCRIPT_DIR/Fonts/"* "$PKG_DIR/usr/share/${APP_NAME}/Fonts/"
    chmod -R 644 "$PKG_DIR/usr/share/${APP_NAME}/Fonts/"*
fi



# --- Binary Installation and Wrapper Logic ---

# Create a runtime wrapper script if any external libraries were bundled.
if [ "$BUNDLED_ANY" -eq 1 ]; then
  echo "🧩 Some runtime libraries are bundled. Creating wrapper launcher that uses bundled libs."

  # Install the actual built executable with a .bin suffix.
  cp "$BUILD_DIR/test" "$PKG_DIR/usr/bin/${APP_NAME}.bin"
  chmod 755 "$PKG_DIR/usr/bin/${APP_NAME}.bin"

  # Create the main executable wrapper script.
  cat > "$PKG_DIR/usr/bin/${APP_NAME}" <<'EOF'
#!/bin/sh
# wrapper to prefer bundled libs in /usr/share/<app>/lib, falling back to system libs
HERE="$(dirname "$(readlink -f "$0")")"
LIBDIR="$HERE/../share/XenUI/lib"
# Prepend the bundled library path to LD_LIBRARY_PATH to ensure they are found first.
export LD_LIBRARY_PATH="${LIBDIR}:${LD_LIBRARY_PATH}"
# Execute the real binary, passing all arguments ($@).
exec "$HERE/XenUI.bin" "$@"
EOF

  chmod 755 "$PKG_DIR/usr/bin/${APP_NAME}"
else
  # Install the executable directly without a wrapper if no libraries were bundled.
  echo "🔗 No runtime libs were bundled (system libs will be used). Installing binary directly."
  cp "$BUILD_DIR/test" "$PKG_DIR/usr/bin/${APP_NAME}"
  chmod 755 "$PKG_DIR/usr/bin/${APP_NAME}"
fi

# Install application icon and create metadata files.
cp logo.png "$PKG_DIR/usr/share/icons/hicolor/256x256/apps/${APP_NAME}.png"

echo "🧾 Creating control file..."
# Generate the mandatory Debian control file with required package metadata.
cat > "$PKG_DIR/DEBIAN/control" <<EOF
Package: ${APP_NAME}
Version: ${APP_VERSION}
Section: utils
Priority: optional
Architecture: ${ARCH}
Maintainer: MD S M Sarowar Hosen <contact.xenonui@gmail.com>
Depends: libc6 (>= 2.31), libstdc++6 (>= 10), libgl1
Description: Xenon
 A lightweight SDL3 + XenUI test app.
EOF

echo "🖥️ Creating desktop entry..."
# Generate the .desktop file for graphical application launchers.
cat > "$PKG_DIR/usr/share/applications/${APP_NAME}.desktop" <<EOF
[Desktop Entry]
Name=Xenon
Comment=A lightweight app built with XenUI and SDL3
Exec=/usr/bin/${APP_NAME}
Icon=${APP_NAME}
Terminal=false
Type=Application
Categories=Utility;Development;
StartupNotify=true
EOF

echo "🔐 Setting permissions..."
# Set standard file permissions.
chmod 644 "$PKG_DIR/usr/share/applications/${APP_NAME}.desktop"
chmod 644 "$PKG_DIR/usr/share/icons/hicolor/256x256/apps/${APP_NAME}.png"
# Set execute permissions for the bundled shared libraries.
if [ -d "$PKG_DIR/usr/share/${APP_NAME}/lib" ]; then
  chmod 755 "$PKG_DIR/usr/share/${APP_NAME}/lib"/*.so* || true
fi


# --- Create SONAME and Base Symlinks for Versioned Libraries ---
# This step ensures that the runtime linker (ld-linux) can correctly resolve
# the library names based on the bundled, versioned files.
LIBDIR="$PKG_DIR/usr/share/${APP_NAME}/lib"
if [ -d "$LIBDIR" ]; then
  echo "🔗 Ensuring soname and base symlinks exist in $LIBDIR ..."
  # Temporarily move to the library directory to simplify symlink creation.
  pushd "$LIBDIR" >/dev/null || exit 1

  shopt -s nullglob
  # Iterate over all files with version suffixes (e.g., libfoo.so.X.Y.Z).
  for verfile in *.so.*; do
    # Skip if the file is already a symlink (i.e., we only process the real, copied files).
    [ -f "$verfile" ] || continue

    # Extract components: prefix (libfoo), rest (X.Y.Z), major (X).
    prefix="${verfile%%.so.*}"     # e.g. libSDL3
    rest="${verfile#*.so.}"        # e.g. 0.2.16
    major="${rest%%.*}"            # e.g. 0
    soname="${prefix}.so.${major}" # e.g. libSDL3.so.0 (the required SONAME)
    base="${prefix}.so"            # e.g. libSDL3.so (the base linker name)

    # 1. Create the SONAME symlink (e.g., libSDL3.so.0 -> libSDL3.so.0.2.16)
    if [ ! -e "$soname" ]; then
      ln -s "$verfile" "$soname"
      echo "  -> created $soname -> $verfile"
    fi

    # 2. Create the base linker symlink (e.g., libSDL3.so -> libSDL3.so.0)
    if [ ! -e "$base" ]; then
      ln -s "$soname" "$base"
      echo "  -> created $base -> $soname"
    fi
  done
  shopt -u nullglob

  # Return to the previous working directory.
  popd >/dev/null
else
  echo "Warning: $LIBDIR missing, skipping symlink creation."
fi
# ---------------------------------------------------------------------------


echo "🧰 Building .deb package..."
# Execute the final build command for the Debian package.
dpkg-deb --build "$PKG_DIR" "$OUTPUT_DEB"

echo "✅ Done! Created $OUTPUT_DEB"
if [ "$BUNDLED_ANY" -eq 1 ]; then
  echo "Note: package bundles all runtime libs. The wrapper will force the app to use bundled libs first."
else
  echo "Note: No libs were bundled. This package will rely on system-installed libraries."
fi