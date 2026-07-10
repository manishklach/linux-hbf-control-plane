#!/usr/bin/env bash
set -euo pipefail

KERNEL_TREE="${KERNEL_TREE:-}"

if [ -n "$KERNEL_TREE" ]; then
	echo "Regenerating patches from topic/hbf-control-plane-v1 branch..."
	cd "$KERNEL_TREE"
	git checkout topic/hbf-control-plane-v1
	git format-patch --cover-letter -o patches master
	echo "Patches regenerated in $KERNEL_TREE/patches/"
else
	echo "KERNEL_TREE not set."
	echo "To regenerate patches from the kernel tree:"
	echo "  export KERNEL_TREE=/path/to/linux"
	echo "  cd \$KERNEL_TREE"
	echo "  git checkout topic/hbf-control-plane-v1"
	echo "  git format-patch --cover-letter -o patches master"
fi

echo ""
echo "The source of truth is the commit series in the kernel tree."
echo "Patches in this repo are generated artifacts."
