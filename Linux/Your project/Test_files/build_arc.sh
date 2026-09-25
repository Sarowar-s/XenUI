#!/bin/bash

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
# This script builds an Arch Linux package (.pkg.tar.zst)
# for the XenUI application.
#
# It follows the same build and bundling logic as the Debian
# package builder, but generates an Arch PKGBUILD and uses
# makepkg to create the final package.
#

set -e

# ------------------------------------------------------------
# Configuration
# ------------------------------------------------------------

APP_NAME="xenui"
APP_VERSION="0.9.0"
PKGREL="1"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

BUILD_DIR="${SCRIPT_DIR}/build"
PACKAGE_DIR="${SCRIPT_DIR}/arch-package"

OUTPUT_PACKAGE="${APP_NAME}-${APP_VERSION}-${PKGREL}-x86_64.pkg.tar.zst"

BUNDLED_ANY=0

echo "🔧 Building XenUI..."
echo "Source: ${SCRIPT_DIR}"

# ------------------------------------------------------------
# Build application
# ------------------------------------------------------------

mkdir -p "$BUILD_DIR"

pushd "$BUILD_DIR" >/dev/null

cmake "$SCRIPT_DIR"
make -j"$(nproc)"

popd >/dev/null

echo "✅ Build finished."

# ------------------------------------------------------------
# Clean previous Arch package build
# ------------------------------------------------------------

rm -rf "$PACKAGE_DIR"
mkdir -p "$PACKAGE_DIR"

# ------------------------------------------------------------
# Create PKGBUILD
# ------------------------------------------------------------

cat > "$PACKAGE_DIR/PKGBUILD" <<EOF
pkgname=${APP_NAME}
pkgver=${APP_VERSION}
pkgrel=${PKGREL}
pkgdesc="A lightweight app built with XenUI and SDL3"
arch=('x86_64')
url="https://github.com/"
license=('Apache-2.0')

depends=('glibc' 'gcc-libs' 'libglvnd')

package() {

    # --------------------------------------------------------
    # Directories
    # --------------------------------------------------------

    install -dm755 "\$pkgdir/usr/bin"
    install -dm755 "\$pkgdir/usr/share/XenUI/lib"
    install -dm755 "\$pkgdir/usr/share/XenUI/Images"
    install -dm755 "\$pkgdir/usr/share/XenUI/Fonts"

    install -dm755 "\$pkgdir/usr/share/applications"
    install -dm755 "\$pkgdir/usr/share/icons/hicolor/256x256/apps"

    # --------------------------------------------------------
    # Application binary
    # --------------------------------------------------------

    install -Dm755 \
        "${BUILD_DIR}/test" \
        "\$pkgdir/usr/bin/XenUI.bin"

    # --------------------------------------------------------
    # XenUI shared library
    # --------------------------------------------------------

EOF

# ------------------------------------------------------------
# Bundle XenUI
# ------------------------------------------------------------

XENUI_BUILD_DIR="${SCRIPT_DIR}/../../build"

echo "📦 Bundling libXenUI..."

shopt -s nullglob

XENUI_LIBS=(
    "${XENUI_BUILD_DIR}"/libXenUI.so*
)

if [ ${#XENUI_LIBS[@]} -gt 0 ]; then

    for lib in "${XENUI_LIBS[@]}"; do
        echo "  -> $lib"

        cp -a "$lib" \
            "$PACKAGE_DIR/libXenUI_temp"
    done

    BUNDLED_ANY=1

fi

shopt -u nullglob

# ------------------------------------------------------------
# Generate remaining PKGBUILD
# ------------------------------------------------------------

cat >> "$PACKAGE_DIR/PKGBUILD" <<EOF

    # --------------------------------------------------------
    # XenUI libraries
    # --------------------------------------------------------

    if [ -d "${PACKAGE_DIR}/libXenUI_temp" ]; then
        cp -a "${PACKAGE_DIR}/libXenUI_temp/"* \
            "\$pkgdir/usr/share/XenUI/lib/"
    fi

EOF

# ------------------------------------------------------------
# SDL libraries
# ------------------------------------------------------------

declare -A TOOL_DIR_MAP=(
    ["libSDL3.so"]="SDL3-3.2.16"
    ["libSDL3_ttf.so"]="SDL3_ttf-3.2.2"
    ["libSDL3_image.so"]="SDL3_image-3.2.4"
)

DECLARED_LIBS=(
    "libSDL3.so"
    "libSDL3_ttf.so"
    "libSDL3_image.so"
)

for libfile in "${DECLARED_LIBS[@]}"; do

    echo "🔎 Looking for ${libfile}..."

    expected_subdir="${TOOL_DIR_MAP[$libfile]}"

    base="${SCRIPT_DIR}/../../tools/${expected_subdir}/build"

    found=""

    for candidate in "$base/$libfile" "$base/$libfile"*; do
        if [ -e "$candidate" ]; then
            found="$(readlink -f "$candidate")"
            break
        fi
    done

    if [ -z "$found" ]; then
        echo "❌ ${libfile} not found."
        exit 1
    fi

    echo "  -> Bundling ${found}"

    cp "$found" \
        "$PACKAGE_DIR/${libfile}.temp"

done

# ------------------------------------------------------------
# Add SDL installation commands to PKGBUILD
# ------------------------------------------------------------

cat >> "$PACKAGE_DIR/PKGBUILD" <<EOF

    # --------------------------------------------------------
    # SDL libraries
    # --------------------------------------------------------

EOF

for libfile in "${DECLARED_LIBS[@]}"; do

cat >> "$PACKAGE_DIR/PKGBUILD" <<EOF
    install -Dm755 \
        "${PACKAGE_DIR}/${libfile}.temp" \
        "\$pkgdir/usr/share/XenUI/lib/${libfile}"
EOF

done

# ------------------------------------------------------------
# Runtime assets
# ------------------------------------------------------------

cat >> "$PACKAGE_DIR/PKGBUILD" <<EOF

    # --------------------------------------------------------
    # Images
    # --------------------------------------------------------

EOF

if [ -d "$SCRIPT_DIR/Images" ]; then

cat >> "$PACKAGE_DIR/PKGBUILD" <<EOF
    cp -r "${SCRIPT_DIR}/Images/"* \
        "\$pkgdir/usr/share/XenUI/Images/" 2>/dev/null || true
EOF

fi

cat >> "$PACKAGE_DIR/PKGBUILD" <<EOF

    # --------------------------------------------------------
    # Fonts
    # --------------------------------------------------------

EOF

if [ -d "$SCRIPT_DIR/Fonts" ]; then

cat >> "$PACKAGE_DIR/PKGBUILD" <<EOF
    cp -r "${SCRIPT_DIR}/Fonts/"* \
        "\$pkgdir/usr/share/XenUI/Fonts/" 2>/dev/null || true
EOF

fi

# ------------------------------------------------------------
# Wrapper
# ------------------------------------------------------------

cat >> "$PACKAGE_DIR/PKGBUILD" <<'EOF'

    # --------------------------------------------------------
    # Runtime wrapper
    # --------------------------------------------------------

    cat > "$pkgdir/usr/bin/XenUI" <<'WRAPPER'
#!/bin/sh

HERE="$(dirname "$(readlink -f "$0")")"
LIBDIR="$HERE/../share/XenUI/lib"

export LD_LIBRARY_PATH="${LIBDIR}:${LD_LIBRARY_PATH}"

exec "$HERE/XenUI.bin" "$@"
WRAPPER

    chmod 755 "$pkgdir/usr/bin/XenUI"

    # --------------------------------------------------------
    # Desktop entry
    # --------------------------------------------------------

    cat > "$pkgdir/usr/share/applications/XenUI.desktop" <<'DESKTOP'
[Desktop Entry]
Name=Xenon
Comment=A lightweight app built with XenUI and SDL3
Exec=/usr/bin/XenUI
Icon=XenUI
Terminal=false
Type=Application
Categories=Utility;Development;
StartupNotify=true
DESKTOP

    chmod 644 "$pkgdir/usr/share/applications/XenUI.desktop"

}
EOF

# ------------------------------------------------------------
# Icon
# ------------------------------------------------------------

cp "$SCRIPT_DIR/logo.png" "$PACKAGE_DIR/XenUI.png"

# Add icon installation before package() closes
python3 - "$PACKAGE_DIR/PKGBUILD" <<EOF
from pathlib import Path

p = Path("$PACKAGE_DIR/PKGBUILD")

text = p.read_text()

text = text.replace(
    '    # --------------------------------------------------------\n    # Runtime wrapper',
    '''    install -Dm644 \\
        "${PACKAGE_DIR}/XenUI.png" \\
        "\\$pkgdir/usr/share/icons/hicolor/256x256/apps/XenUI.png"

    # --------------------------------------------------------
    # Runtime wrapper'''
)

p.write_text(text)
EOF

# ------------------------------------------------------------
# Build Arch package
# ------------------------------------------------------------

echo "📦 Building Arch package..."

cd "$PACKAGE_DIR"

if ! command -v makepkg >/dev/null 2>&1; then
    echo "❌ makepkg is not installed."
    echo
    echo "You need an Arch-compatible makepkg environment."
    exit 1
fi

makepkg --clean

# ------------------------------------------------------------
# Move resulting package
# ------------------------------------------------------------

PACKAGE_FILE=$(find . -maxdepth 1 -name "*.pkg.tar.*" -type f | head -n 1)

if [ -z "$PACKAGE_FILE" ]; then
    echo "❌ Arch package was not created."
    exit 1
fi

cp "$PACKAGE_FILE" "${SCRIPT_DIR}/${OUTPUT_PACKAGE}"

echo
echo "✅ Done!"
echo "Created:"
echo "${SCRIPT_DIR}/${OUTPUT_PACKAGE}"