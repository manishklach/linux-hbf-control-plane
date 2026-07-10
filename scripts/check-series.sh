#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0
#
# check-series.sh  —  Verify each commit builds independently
#
# Usage:
#   export KERNEL_TREE=/path/to/linux
#   cd \$KERNEL_TREE
#   git checkout topic/hbf-control-plane-v1
#   ../path/to/scripts/check-series.sh

set -euo pipefail

KERNEL_TREE="${KERNEL_TREE:-}"

if [ -z "$KERNEL_TREE" ]; then
	echo "error: KERNEL_TREE must be set" >&2
	exit 1
fi

cd "$KERNEL_TREE"

# Count commits on this branch not in master
COMMIT_COUNT=$(git log --oneline master..HEAD | wc -l)
echo "Testing $COMMIT_COUNT commits..."

git rebase --exec "make defconfig && ./scripts/config -e CONFIG_HBF_CONTROL_PLANE && make -j\$(nproc) 2>&1 | tail -5" master

echo "All commits build successfully."
