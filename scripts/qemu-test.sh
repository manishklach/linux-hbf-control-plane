#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0
#
# qemu-test.sh  —  Boot patched kernel in QEMU and run HBF tests
#
# Prerequisites:
#   - KERNEL_TREE points to a built kernel with CONFIG_HBF_CONTROL_PLANE=y
#   - Busybox rootfs image at \$ROOTFS_IMAGE
#   - QEMU installed (qemu-system-x86_64)
#
# Usage:
#   export KERNEL_TREE=/path/to/linux
#   export ROOTFS_IMAGE=/path/to/rootfs.ext4
#   ./scripts/qemu-test.sh

set -euo pipefail

KERNEL_TREE="${KERNEL_TREE:-}"
ROOTFS="${ROOTFS_IMAGE:-}"

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

# Build a minimal initrd with hbfctl and test binaries
INITRD_DIR=$(mktemp -d)
cp "$KERNEL_TREE/samples/hbf/hbfctl" "$INITRD_DIR/"
cp "$KERNEL_TREE/tools/testing/selftests/mm/hbf_hint_abi" "$INITRD_DIR/"
cp "$KERNEL_TREE/tools/testing/selftests/mm/hbf_promote_demote" "$INITRD_DIR/"

cat > "$INITRD_DIR/init" << 'INITEOF'
#!/bin/sh
mount -t proc none /proc
mount -t sysfs none /sys
mount -t debugfs none /sys/kernel/debug

echo "=== HBF Control Plane QEMU Test ==="

# Check device exists
if [ -c /dev/hbfctl ]; then
	echo "OK: /dev/hbfctl present"
else
	echo "FAIL: /dev/hbfctl not found"
fi

# Run tests
/hbf_hint_abi 2>&1
echo "exit: $?"

# Power off
poweroff -f
INITEOF
chmod +x "$INITRD_DIR/init"

# Create initrd cpio archive
(cd "$INITRD_DIR" && find . | cpio -o -H newc | gzip > /tmp/hbf-test-initrd.gz)

# Boot QEMU
qemu-system-x86_64 \
	-kernel "$KERNEL" \
	-initrd /tmp/hbf-test-initrd.gz \
	-append "console=ttyS0 panic=1" \
	-nographic \
	-m 2G \
	-smp 2 \
	-nodefaults \
	-serial mon:stdio

rm -rf "$INITRD_DIR"
