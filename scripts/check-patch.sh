#!/usr/bin/env bash
set -euo pipefail

PATCH_FILE="rfc-hbf-linux-control-plane.patch"
EXAMPLE_SRC="examples/hbfctl-demo.c"
EXAMPLE_BIN="examples/hbfctl-demo"

if [ ! -f "${PATCH_FILE}" ]; then
	echo "error: ${PATCH_FILE} not found in repository root" >&2
	exit 1
fi

if [ -n "${KERNEL_TREE:-}" ]; then
	if [ ! -d "${KERNEL_TREE}" ]; then
		echo "error: KERNEL_TREE does not point to a directory: ${KERNEL_TREE}" >&2
		exit 1
	fi

	if [ ! -x "${KERNEL_TREE}/scripts/checkpatch.pl" ]; then
		echo "error: ${KERNEL_TREE}/scripts/checkpatch.pl is not executable" >&2
		exit 1
	fi

	"${KERNEL_TREE}/scripts/checkpatch.pl" --strict "${PATCH_FILE}"
else
	echo "KERNEL_TREE is not set."
	echo "To run kernel patch style checks, use:"
	echo "  export KERNEL_TREE=/path/to/linux"
	echo "  \$KERNEL_TREE/scripts/checkpatch.pl --strict ${PATCH_FILE}"
fi

gcc -Wall -Wextra -O2 -o "${EXAMPLE_BIN}" "${EXAMPLE_SRC}"
echo "Built ${EXAMPLE_BIN}"
