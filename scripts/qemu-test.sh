#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0
#
# qemu-test.sh  —  Boot patched kernel in QEMU and run HBF tests
#
# Uses tests/qemu-test-init.c as the /init binary (statically linked).
#
# Prerequisites:
#   - KERNEL_TREE points to a built kernel with CONFIG_HBF_CONTROL_PLANE=y
#   - Cross/x86_64 gcc (x86_64-linux-gnu-gcc) for static compilation
#   - QEMU installed (qemu-system-x86_64)
#
# Usage:
#   export KERNEL_TREE=/path/to/linux
#   ./scripts/qemu-test.sh

set -euo pipefail

KERNEL_TREE="${KERNEL_TREE:-}"

if [ -z "$KERNEL_TREE" ]; then
	echo "error: KERNEL_TREE must be set" >&2
	exit 1
fi

KERNEL="${KERNEL_TREE}/arch/x86/boot/bzImage"

if [ ! -f "$KERNEL" ]; then
	echo "error: Kernel not built at $KERNEL" >&2
	echo "Run: cd \$KERNEL_TREE && make -j\$(nproc)" >&2
	exit 1
fi

# Locate the test source (relative to this script or explicit)
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
TEST_SRC="${TEST_SRC:-${SCRIPT_DIR}/../tests/qemu-test-init.c}"

if [ ! -f "$TEST_SRC" ]; then
	echo "error: test source not found at $TEST_SRC" >&2
	exit 1
fi

# Build a minimal initrd with the standalone test binary as /init
INITRD_DIR=$(mktemp -d)

# Compile the test binary statically against kernel UAPI headers
x86_64-linux-gnu-gcc -static \
	-I"${KERNEL_TREE}/include/uapi" -I"${KERNEL_TREE}/include" \
	"$TEST_SRC" -o "${INITRD_DIR}/init"

# Create initrd cpio archive
(cd "$INITRD_DIR" && find . | cpio -o -H newc | gzip > /tmp/hbf-test-initrd.gz)

# Boot QEMU
qemu-system-x86_64 \
	-kernel "$KERNEL" \
	-initrd /tmp/hbf-test-initrd.gz \
	-append "console=ttyS0 panic=1" \
	-nographic \
	-m 512M \
	-nodefaults \
	-serial mon:stdio

rm -rf "$INITRD_DIR"
