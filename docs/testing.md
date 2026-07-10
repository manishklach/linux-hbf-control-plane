# Testing

## Prerequisites

- Linux kernel tree (clone of `torvalds/linux.git`)
- Build tools (`gcc`, `make`, `flex`, `bison`, `openssl-dev`, `elfutils-libelf-dev`)
- QEMU (for boot tests)
- `numactl` and `libnuma-dev` (for NUMA tests)
- `bpftrace` (for eBPF observability)

## Apply Patches

```bash
export KERNEL_TREE=/path/to/linux
cd $KERNEL_TREE
git am /path/to/patches/000*.patch
```

## Build Kernel

```bash
cd $KERNEL_TREE
make defconfig
./scripts/config -e CONFIG_HBF_CONTROL_PLANE
make -j$(nproc)
```

To verify no-regression without HBF:

```bash
./scripts/config -d CONFIG_HBF_CONTROL_PLANE
make clean && make -j$(nproc)
```

## Check Each Commit Builds

```bash
cd $KERNEL_TREE
git rebase --exec "make defconfig && ./scripts/config -e CONFIG_HBF_CONTROL_PLANE && make -j$(nproc)" master
```

## Check Style

```bash
cd $KERNEL_TREE
for p in /path/to/patches/000*.patch; do
    scripts/checkpatch.pl --strict "$p"
done
```

## QEMU Boot Test

```bash
cd $KERNEL_TREE
./scripts/qemu-test.sh
```

This boots a minimal initrd that checks `/dev/hbfctl` presence and runs the selftests.

## Run Selftests

On a test machine or QEMU VM:

```bash
cd $KERNEL_TREE/tools/testing/selftests/mm
make
sudo ./hbf_hint_abi
sudo ./hbf_reject_invalid
sudo ./hbf_cancel
sudo ./hbf_promote_demote   # requires 2+ NUMA nodes
```

## Run Benchmarks

```bash
cd benchmarks
make
./sequential-access 64
./random-access 64
./promote-pressure 128
./memory-pressure 128 512
```

## Verify Tracepoints

```bash
sudo perf list | grep hbf
sudo perf trace -e hbf:hbf_hint_submit,hbf:hbf_hint_complete ./hbfctl_caps
sudo bpftrace tools/hbftrace/hbf_latency.py
```

## Verify Debugfs

```bash
sudo mount -t debugfs none /sys/kernel/debug
cat /sys/kernel/debug/hbf/stats
```

## Expected Behavior

- `/dev/hbfctl` exists with mode 0600
- `hbfctl caps` returns nonzero capabilities
- `hbfctl submit` returns a nonzero request_id within 1ms
- `hbfctl query` shows state progression: SUBMITTED → QUEUED → COMPLETED
- Invalid hints (bad op, unmapped range, overflow) return negative error codes
- `hbfctl cancel` transitions request to CANCELLED
- PROMOTE moves 90%+ pages to hot node (verified via numa_maps)
- DEMOTE moves 90%+ pages back to warm node
- No kernel memory leak on process exit (kmemleak)

## Known Limitations

- The NUMA backend migrates one page at a time; large ranges are slow
- No cgroup integration yet
- No admission control beyond queue depth
- No anti-thrashing hysteresis
- Deadline-aware scheduling is basic (check + skip, no priority queue)
