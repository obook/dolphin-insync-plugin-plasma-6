#!/usr/bin/env bash
#
# Name : build.sh
# Description : Build the plugin and produce a Debian (.deb) package.
# Author : Olivier Booklage
# Date : May 2026
# Licence : CC BY-SA 4.0
#
# Usage:
#   ./build.sh        Build the plugin and the .deb package.
#   ./build.sh clean  Remove all build artefacts.
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}"

# -------- clean mode --------
if [[ "${1:-}" == "clean" ]]
then
    rm -rf build/ dist/
    rm -f ../dolphin-insync-plugin_*.deb \
          ../dolphin-insync-plugin_*.ddeb \
          ../dolphin-insync-plugin_*.changes \
          ../dolphin-insync-plugin_*.buildinfo \
          ../dolphin-insync-plugin-dbgsym_*.deb \
          ../dolphin-insync-plugin-dbgsym_*.ddeb
    echo "Build artefacts removed."
    exit 0
fi

# -------- required tools --------
for tool in cmake dpkg-buildpackage dh
do
    if ! command -v "${tool}" >/dev/null
    then
        echo "Error: '${tool}' is not installed." >&2
        echo "On Debian/Ubuntu, install the build prerequisites with:" >&2
        echo "  sudo apt install build-essential cmake debhelper devscripts \\" >&2
        echo "                   extra-cmake-modules qt6-base-dev \\" >&2
        echo "                   kf6-kcoreaddons-dev kf6-kconfig-dev \\" >&2
        echo "                   kf6-kio-dev kf6-kwidgetsaddons-dev" >&2
        exit 1
    fi
done

# -------- 1. quick CMake sanity build --------
# Catches compilation errors early, before invoking the full
# Debian packaging pipeline.
cmake -DCMAKE_INSTALL_PREFIX=/usr -B build/
cmake --build build/ --parallel

# -------- 2. Debian package build --------
# dpkg-buildpackage runs debian/rules, which calls cmake again in
# an isolated build tree under debian/<package>/.
#   -us  do not sign the source package
#   -uc  do not sign the .changes file
#   -b   binary-only build (no source tarball)
dpkg-buildpackage -us -uc -b

# -------- 3. collect produced files into dist/ --------
# dpkg-buildpackage drops its outputs in the parent directory;
# moving them into dist/ keeps the workspace tidy.
mkdir -p dist
shopt -s nullglob
for f in ../dolphin-insync-plugin_*.deb \
         ../dolphin-insync-plugin_*.ddeb \
         ../dolphin-insync-plugin_*.changes \
         ../dolphin-insync-plugin_*.buildinfo \
         ../dolphin-insync-plugin-dbgsym_*.deb \
         ../dolphin-insync-plugin-dbgsym_*.ddeb
do
    mv -v "${f}" dist/
done

echo
echo "Build complete. Files in dist/:"
ls -la dist/
