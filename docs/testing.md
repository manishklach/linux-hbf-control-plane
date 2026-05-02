# Testing

## Basic Validation

Run the local helper:

```bash
./scripts/check-patch.sh
```

That script is expected to:

- verify that `rfc-hbf-linux-control-plane.patch` exists
- run `checkpatch.pl` if `KERNEL_TREE` points at a Linux source tree
- compile `examples/hbfctl-demo.c` with `gcc -Wall -Wextra -O2`

## Patch Style

If a Linux source tree is available:

```bash
export KERNEL_TREE=/path/to/linux
$KERNEL_TREE/scripts/checkpatch.pl --strict rfc-hbf-linux-control-plane.patch
```

## Static Checking

Within a suitable Linux tree, the RFC code should be evaluated with:

- `scripts/checkpatch.pl`
- `sparse`

Example direction:

```bash
make C=1 CF="-D__CHECK_ENDIAN__" M=path/to/rfc/code
```

## Formatting Guidance

Use kernel style for kernel material.

Use `clang-format` only for userspace examples such as `examples/hbfctl-demo.c`, not as a substitute for kernel coding style in patch content.

## Mock Test Plan

1. Compile `examples/hbfctl-demo.c`.
2. Confirm the UAPI-like structs and ioctl definitions compile cleanly.
3. If a mock `/dev/hbfctl` exists, run the sample with `--prefetch`, `--promote`, and `--demote`.
4. Confirm that absent-device behavior is friendly and clearly explains the RFC/mock status.

Example commands:

```bash
gcc -Wall -Wextra -O2 -o examples/hbfctl-demo examples/hbfctl-demo.c
./examples/hbfctl-demo --prefetch 0x100000 4096
./examples/hbfctl-demo --promote 0x200000 8192
./examples/hbfctl-demo --demote 0x300000 4096
```

## Future Real-Hardware Test Areas

If real hardware or realistic emulation emerges, useful next-stage validation would include:

- CXL memory device exposure and management
- DAX mapping behavior
- page migration latency across tiers
- KV-cache trace replay from inference runtimes
- eBPF and tracepoint visibility for hint -> prefetch -> fault latency

## Success Criteria for Early RFCs

For this repository, success is not benchmark leadership. Success is:

- a credible kernel-facing abstraction
- code and docs that survive basic kernel review expectations
- a test surface that can grow once a backend becomes concrete
