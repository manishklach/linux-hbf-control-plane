#!/usr/bin/env bash
set -euo pipefail

REPO_DIR="$(cd "$(dirname "$0")/.." && pwd)"
KERNEL_TREE="${KERNEL_TREE:-}"

if [ -z "$KERNEL_TREE" ]; then
	echo "KERNEL_TREE not set — checking patches without checkpatch.pl"
	echo "Set KERNEL_TREE to run kernel style checks."
fi

for patch in "$REPO_DIR"/patches/000*.patch; do
	echo "=== $patch ==="
	if [ -n "$KERNEL_TREE" ]; then
		"${KERNEL_TREE}/scripts/checkpatch.pl" --strict "$patch" || true
	fi
done

# Build the userspace hbfctl tool
if command -v gcc &>/dev/null; then
	gcc -Wall -Wextra -O2 -o "$REPO_DIR/samples/hbf/hbfctl" \
		"$REPO_DIR/samples/hbf/hbfctl.c" 2>/dev/null && \
		echo "Built hbfctl" || \
		echo "Note: hbfctl requires kernel headers and libnuma"
fi
