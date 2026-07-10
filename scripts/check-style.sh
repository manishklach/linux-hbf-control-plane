#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0
#
# check-style.sh  —  Run checkpatch.pl on HBF patches
#
# Usage:
#   export KERNEL_TREE=/path/to/linux
#   ./scripts/check-style.sh

set -euo pipefail

KERNEL_TREE="${KERNEL_TREE:-}"
REPO_DIR="$(cd "$(dirname "$0")/.." && pwd)"

if [ -z "$KERNEL_TREE" ]; then
	echo "KERNEL_TREE not set — checking patches without checkpatch.pl"
	echo "Set KERNEL_TREE to run kernel style checks."
	exit 0
fi

if [ ! -x "${KERNEL_TREE}/scripts/checkpatch.pl" ]; then
	echo "error: ${KERNEL_TREE}/scripts/checkpatch.pl not found or not executable" >&2
	exit 1
fi

for patch in "$REPO_DIR"/patches/000*.patch; do
	echo "=== Checking $patch ==="
	"${KERNEL_TREE}/scripts/checkpatch.pl" --strict "$patch" || true
	echo ""
done
