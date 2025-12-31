#!/bin/bash
# WebOS IPK Packaging Script for GTA III re3

set -e  # Exit on error

BUILD_DIR="build-webos-wsl"
PACKAGE_DIR="package"
STAGING_DIR="${PACKAGE_DIR}_staging"
PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"

echo "=== GTA III re3 WebOS Packaging Script ==="
echo "Project root: ${PROJECT_ROOT}"

# Check if build directory exists
if [ ! -d "${BUILD_DIR}" ]; then
    echo "Error: Build directory '${BUILD_DIR}' not found!"
    echo "Please run CMake build first:"
    echo "  mkdir ${BUILD_DIR} && cd ${BUILD_DIR}"
    echo "  cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/webos/WebOSToolchain.cmake -DWEBOS_TOUCHPAD=ON"
    echo "  cmake --build . --parallel"
    exit 1
fi

# Check if executable exists
if [ ! -f "${BUILD_DIR}/src/re3" ]; then
    echo "Error: Executable '${BUILD_DIR}/src/re3' not found!"
    echo "Build may have failed. Check build output."
    exit 1
fi

echo "Found re3 executable"

# Clean and create staging directory
rm -rf "${STAGING_DIR}"
mkdir -p "${STAGING_DIR}"

echo "Copying files to staging directory..."

# Strip and copy executable
echo "  - Stripping debug symbols from re3..."
# Try to use toolchain strip if available, otherwise use system strip
STRIP_CMD="arm-linux-gnueabi-strip"
if ! command -v ${STRIP_CMD} >/dev/null 2>&1; then
    # Try Linaro toolchain path
    LINARO_STRIP="$HOME/toolchains/gcc-linaro-4.8-2015.06-x86_64_arm-linux-gnueabi/bin/arm-linux-gnueabi-strip"
    if [ -f "${LINARO_STRIP}" ]; then
        STRIP_CMD="${LINARO_STRIP}"
    else
        echo "  Warning: ARM strip not found, using system strip (may not work)"
        STRIP_CMD="strip"
    fi
fi

${STRIP_CMD} --strip-all "${BUILD_DIR}/src/re3" -o "${STAGING_DIR}/re3"
ORIGINAL_SIZE=$(stat -f%z "${BUILD_DIR}/src/re3" 2>/dev/null || stat -c%s "${BUILD_DIR}/src/re3")
STRIPPED_SIZE=$(stat -f%z "${STAGING_DIR}/re3" 2>/dev/null || stat -c%s "${STAGING_DIR}/re3")
echo "  - re3 executable (reduced from ${ORIGINAL_SIZE} to ${STRIPPED_SIZE} bytes)"

# Copy appinfo.json
cp "${PACKAGE_DIR}/appinfo.json" "${STAGING_DIR}/"
echo "  - appinfo.json"

# Copy icon if it exists
if [ -f "${PACKAGE_DIR}/icon.png" ]; then
    cp "${PACKAGE_DIR}/icon.png" "${STAGING_DIR}/"
    echo "  - icon.png"
else
    echo "  Warning: icon.png not found, package will use default icon"
fi

# Copy gamefiles if they exist
if [ -d "gamefiles" ]; then
    cp -r gamefiles "${STAGING_DIR}/"
    echo "  - gamefiles directory"
fi

# Create package.properties for executable permissions
echo "filemode.755=re3" > "${STAGING_DIR}/package.properties"

# Create install scripts (prevents "Bad file descriptor" errors)
echo "Creating install scripts..."
cat > "${STAGING_DIR}/pmPostInstall.script" << 'EOF'
#!/bin/sh
# Post-install script for re3
# Ensure proper permissions on executable
chmod 755 /media/cryptofs/apps/usr/palm/applications/com.re3.gta3/re3 2>/dev/null || true
exit 0
EOF

cat > "${STAGING_DIR}/pmPreRemove.script" << 'EOF'
#!/bin/sh
# Pre-removal script for re3
exit 0
EOF

chmod +x "${STAGING_DIR}/pmPostInstall.script" "${STAGING_DIR}/pmPreRemove.script"
echo "  - Install scripts created"

# Create IPK using palm-package
echo "Creating IPK package..."

# Add SDK bin to PATH if not already there
SDK_BIN="C:/Users/stark/OneDrive/Documents/Test/HP webOS/SDK/bin"
export PATH="${SDK_BIN}:${PATH}"

# Run palm-package
if command -v palm-package >/dev/null 2>&1; then
    palm-package "${STAGING_DIR}"

    # Find the generated IPK
    IPK_FILE=$(ls -t *.ipk 2>/dev/null | head -1)

    if [ -n "${IPK_FILE}" ]; then
        echo "=== SUCCESS ==="
        echo "IPK package created: ${IPK_FILE}"
        echo ""
        echo "To install on TouchPad:"
        echo "  1. Enable Developer Mode on your TouchPad"
        echo "  2. Connect via USB"
        echo "  3. Run: pdk-device-install.bat ${IPK_FILE}"
        echo ""
        echo "IMPORTANT: You must copy GTA III game data to the device:"
        echo "  Copy your PC GTA III 'data' folder to:"
        echo "  /media/internal/.gta3/data/"
    else
        echo "Error: IPK file not found after packaging"
        exit 1
    fi
else
    echo "Error: palm-package not found in PATH"
    echo "Please ensure WebOS SDK bin directory is in your PATH:"
    echo "  ${SDK_BIN}"
    exit 1
fi

# Cleanup staging directory
rm -rf "${STAGING_DIR}"

echo "=== Packaging complete ==="
