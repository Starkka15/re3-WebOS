#!/bin/bash
# WebOS re3 Issue Checker
# Run this in WSL to identify potential problems before deployment

set +e  # Don't exit on errors, we want to see all issues

PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build-webos-wsl"
RE3_BINARY="${BUILD_DIR}/src/re3"

echo "========================================="
echo "  WebOS re3 Pre-Deployment Checker"
echo "========================================="
echo ""

ISSUES_FOUND=0

# Check 1: Binary exists
echo "[1] Checking if re3 binary exists..."
if [ -f "${RE3_BINARY}" ]; then
    echo "    ✓ Binary found: ${RE3_BINARY}"
    BINARY_SIZE=$(stat -c%s "${RE3_BINARY}" 2>/dev/null || stat -f%z "${RE3_BINARY}")
    echo "    Size: $((BINARY_SIZE / 1024 / 1024))MB"

    # Check if stripped
    if file "${RE3_BINARY}" | grep -q "not stripped"; then
        echo "    ⚠ WARNING: Binary contains debug symbols (${BINARY_SIZE} bytes)"
        echo "      This will be automatically stripped during packaging"
        ISSUES_FOUND=$((ISSUES_FOUND + 1))
    else
        echo "    ✓ Binary is stripped"
    fi
else
    echo "    ✗ ERROR: Binary not found at ${RE3_BINARY}"
    echo "      Run build first!"
    exit 1
fi

# Check 2: Shared library dependencies
echo ""
echo "[2] Checking shared library dependencies..."
if command -v "$HOME/toolchains/gcc-linaro-4.8-2015.06-x86_64_arm-linux-gnueabi/bin/arm-linux-gnueabi-readelf" >/dev/null 2>&1; then
    READELF="$HOME/toolchains/gcc-linaro-4.8-2015.06-x86_64_arm-linux-gnueabi/bin/arm-linux-gnueabi-readelf"
elif command -v arm-linux-gnueabi-readelf >/dev/null 2>&1; then
    READELF="arm-linux-gnueabi-readelf"
else
    echo "    ⚠ WARNING: arm-linux-gnueabi-readelf not found, trying native readelf..."
    READELF="readelf"
fi

echo "    Shared library dependencies:"
${READELF} -d "${RE3_BINARY}" 2>/dev/null | grep "NEEDED" | while read -r line; do
    LIB=$(echo "$line" | grep -oP '\[.*?\]' | tr -d '[]')
    echo "      - ${LIB}"

    # Check for problematic dependencies
    case "${LIB}" in
        libm.so*|libc.so*|libpthread.so*|libdl.so*|ld-linux*)
            # System libraries - OK
            ;;
        libSDL*)
            echo "        ✓ Should be available in webOS PDK"
            ;;
        libGLESv2*|libEGL*)
            echo "        ✓ Should be available in webOS PDK"
            ;;
        libopenal*)
            echo "        ✓ Should be available in webOS PDK"
            ;;
        libstdc++*|libgcc*)
            echo "        ⚠ Should be statically linked (check toolchain flags)"
            ISSUES_FOUND=$((ISSUES_FOUND + 1))
            ;;
        libmpg123*)
            echo "        ⚠ WARNING: mpg123 might not be available on webOS!"
            echo "          MP3 audio may not work"
            ISSUES_FOUND=$((ISSUES_FOUND + 1))
            ;;
        *)
            echo "        ⚠ UNKNOWN: Verify this library exists on webOS"
            ISSUES_FOUND=$((ISSUES_FOUND + 1))
            ;;
    esac
done || echo "    ⚠ Could not read dependencies"

# Check 3: Icon file
echo ""
echo "[3] Checking for icon file..."
if [ -f "${PROJECT_ROOT}/package/icon.png" ]; then
    echo "    ✓ Icon found: package/icon.png"
else
    echo "    ⚠ WARNING: No icon.png found in package/"
    echo "      App will use default webOS icon"
    echo "      Create a 64x64 PNG icon at: package/icon.png"
    ISSUES_FOUND=$((ISSUES_FOUND + 1))
fi

# Check 4: Memory requirements
echo ""
echo "[4] Checking memory requirements..."
REQUIRED_MEM=$(grep -o '"requiredMemory": [0-9]*' "${PROJECT_ROOT}/package/appinfo.json" | grep -o '[0-9]*')
echo "    Required memory in appinfo.json: ${REQUIRED_MEM}MB"
if [ "${REQUIRED_MEM}" -lt 512 ]; then
    echo "    ⚠ WARNING: GTA III may need more than ${REQUIRED_MEM}MB"
    echo "      Recommended: 512MB or higher"
    echo "      Edit package/appinfo.json and set requiredMemory to 512"
    ISSUES_FOUND=$((ISSUES_FOUND + 1))
else
    echo "    ✓ Memory allocation looks reasonable"
fi

# Check 5: Install scripts
echo ""
echo "[5] Checking install scripts in staging..."
STAGING_DIR="${BUILD_DIR}/src/re3_ipk_staging"
if [ -d "${STAGING_DIR}" ]; then
    if [ -f "${STAGING_DIR}/pmPostInstall.script" ]; then
        echo "    ✓ pmPostInstall.script exists"
    else
        echo "    ⚠ WARNING: pmPostInstall.script not in staging (will be created during packaging)"
    fi

    if [ -f "${STAGING_DIR}/pmPreRemove.script" ]; then
        echo "    ✓ pmPreRemove.script exists"
    else
        echo "    ⚠ WARNING: pmPreRemove.script not in staging (will be created during packaging)"
    fi
else
    echo "    ℹ Staging directory not found (normal if IPK hasn't been built yet)"
fi

# Check 6: IPK package
echo ""
echo "[6] Checking for existing IPK..."
if [ -f "${BUILD_DIR}/src/com.re3.gta3_1.0.0_all.ipk" ]; then
    IPK_SIZE=$(stat -c%s "${BUILD_DIR}/src/com.re3.gta3_1.0.0_all.ipk" 2>/dev/null || stat -f%z "${BUILD_DIR}/src/com.re3.gta3_1.0.0_all.ipk")
    echo "    ✓ IPK found: $(( IPK_SIZE / 1024 / 1024 ))MB"
    echo "      ${BUILD_DIR}/src/com.re3.gta3_1.0.0_all.ipk"

    # Check if IPK is too large
    if [ "${IPK_SIZE}" -gt 3145728 ]; then  # 3MB
        echo "    ⚠ WARNING: IPK is quite large ($(( IPK_SIZE / 1024 / 1024 ))MB)"
        echo "      May cause installation issues"
        ISSUES_FOUND=$((ISSUES_FOUND + 1))
    fi
else
    echo "    ℹ No IPK found yet (will be created during build)"
fi

# Check 7: Game data location
echo ""
echo "[7] Checking game data requirements..."
echo "    ⚠ REMINDER: GTA III game data must be manually copied to device!"
echo "      Copy your PC GTA III 'data' folder to:"
echo "      /media/internal/.gta3/data/"
echo "      This includes: textures, models, audio, scripts, etc."

# Summary
echo ""
echo "========================================="
echo "  Summary"
echo "========================================="
if [ ${ISSUES_FOUND} -eq 0 ]; then
    echo "✓ No critical issues found!"
    echo "  Ready to deploy to webOS TouchPad"
else
    echo "⚠ Found ${ISSUES_FOUND} potential issue(s)"
    echo "  Review warnings above before deploying"
fi

echo ""
echo "Next steps:"
echo "  1. Fix any warnings above"
echo "  2. Rebuild: cd build-webos-wsl && make -j\$(nproc)"
echo "  3. Install: pdk-device-install.bat com.re3.gta3_1.0.0_all.ipk"
echo "  4. Copy GTA III game data to device"
echo ""
