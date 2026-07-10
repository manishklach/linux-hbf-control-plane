#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0
#
# build-kernel.sh  —  Build Linux kernel with HBF control plane enabled/disabled
#
# Usage:
#   export KERNEL_TREE=/path/to/linux
#   ./scripts/build-kernel.sh            # builds with CONFIG_HBF_CONTROL_PLANE=y
#   ./scripts/build-kernel.sh --disable  # builds without HBF (no-regression check)

set -euo pipefail

KERNEL_TREE="${KERNEL_TREE:-}"
CONFIG_MODE="${1:-enable}"

if [ -z "$KERNEL_TREE" ]; then
	echo "error: KERNEL_TREE must be set" >&2
	echo "usage: KERNEL_TREE=/path/to/linux $0 [--disable]" >&2
	exit 1
fi

if [ ! -d "$KERNEL_TREE" ]; then
	echo "error: KERNEL_TREE does not point to a directory: $KERNEL_TREE" >&2
	exit 1
fi

cd "$KERNEL_TREE"

# Ensure patches are applied first
if [ ! -f "mm/hbf/Kconfig" ]; then
	echo "HBF source not found — did you apply the patches?"
	echo "  git am /path/to/patches/000*.patch"
	exit 1
fi

make defconfig

if [ "$CONFIG_MODE" = "enable" ] || [ "$CONFIG_MODE" = "enable" ]; then
	./scripts/config -e CONFIG_HBF_CONTROL_PLANE
	echo "Building with CONFIG_HBF_CONTROL_PLANE=y"
else
	./scripts/config -d CONFIG_HBF_CONTROL_PLANE
	echo "Building with CONFIG_HBF_CONTROL_PLANE disabled"
fi

make -j"$(nproc)" 2>&1 | tee build.log

echo "Build complete. Log in build.log"
echo "To verify: grep -E 'hbf|error|warning' build.log"
