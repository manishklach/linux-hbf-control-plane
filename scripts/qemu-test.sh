#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0
#
# qemu-test.sh  —  Boot patched kernel in QEMU and run HBF tests
#
# Boots with a 2-node NUMA topology (256MB each).
# Uses tests/qemu-test-init.c as the /init binary (statically linked).
#
# Prerequisites:
#   - KERNEL_TREE points to a built kernel with CONFIG_HBF_CONTROL_PLANE=y
#   - x86_64-linux-gnu-gcc for static compilation
#   - QEMU installed (qemu-system-x86_64)
#
# Usage:
#   export KERNEL_TREE=/path/to/linux
#   export HBF_QEMU_NODES=2          (optional, default 2)
#   export HBF_QEMU_MEM_PER_NODE=256M (optional, default 256M)
#   ./scripts/qemu-test.sh

set -euo pipefail

KERNEL_TREE="${KERNEL_TREE:-}"
HBF_QEMU_NODES="${HBF_QEMU_NODES:-2}"
HBF_QEMU_MEM_PER_NODE="${HBF_QEMU_MEM_PER_NODE:-256M}"

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

echo "=== HBF QEMU Test ==="
echo "Nodes: $HBF_QEMU_NODES, Memory per node: $HBF_QEMU_MEM_PER_NODE"

# Compile the test binary statically against kernel UAPI headers
x86_64-linux-gnu-gcc -static \
	-I"${KERNEL_TREE}/include/uapi" -I"${KERNEL_TREE}/include" \
	"$TEST_SRC" -o "${INITRD_DIR}/init"

# Create initrd cpio archive
(cd "$INITRD_DIR" && find . | cpio -o -H newc | gzip > /tmp/hbf-test-initrd.gz)

# Build NUMA topology QEMU arguments
NUMACTL=""
MEMOBJ=""
for i in $(seq 0 $((HBF_QEMU_NODES - 1))); do
	MEMOBJ="$MEMOBJ -object memory-backend-ram,size=$HBF_QEMU_MEM_PER_NODE,id=m$i"
	NUMACTL="$NUMACTL -numa node,memdev=m$i,cpus=$i"
done
# Pin CPU 0 to node 0, CPU 1 to node 1, etc.
SMTOPTS="-smp sockets=$HBF_QEMU_NODES,cores=1,threads=1"

# Boot QEMU
qemu-system-x86_64 \
	$SMTOPTS \
	$MEMOBJ \
	$NUMACTL \
	-kernel "$KERNEL" \
	-initrd /tmp/hbf-test-initrd.gz \
	-append "console=ttyS0 panic=1" \
	-nographic \
	-nodefaults \
	-serial mon:stdio

rm -rf "$INITRD_DIR"
