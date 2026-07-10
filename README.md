# linux-hbf-control-plane

> HBF only becomes useful when the system can move data before compute asks for it.

`Experimental` `Research Platform` `v0.2 — Executable NUMA Tier Backend`

A Linux-based runtime-guided memory-tier orchestration prototype that predicts future AI working-set demand, submits asynchronous range hints, and moves pages between simulated hot (DRAM) and warm (NUMA) memory tiers.

## Status: v0.2

The repository has evolved from a documentation-only RFC into an **executable research platform**:

- **7-commit kernel series** against latest upstream Linux, each independently buildable
- **Internal `mm_tier_range_hint` types** decoupled from UAPI (the `/dev/hbfctl` ioctl is a thin adapter)
- **NUMA migration backend** that promotes and demotes pages between arbitrary NUMA nodes
- **Asynchronous request queue** with state machine (SUBMITTED→VALIDATED→ADMITTED→QUEUED→RUNNING→COMPLETED)
- **11 tracepoints** covering the full hint lifecycle
- **debugfs statistics** at `/sys/kernel/debug/hbf/`
- **hbfctl CLI tool** with submit/query/cancel/caps/status subcommands
- **bpftrace observability scripts** for latency, migration, and deadline-miss analysis
- **Benchmark suite** for sequential, random, promote-pressure, and memory-pressure tests
- **selftests** validating ABI, promote/demote correctness, cancellation, and error handling

## Memory Tier Model

```
Node 0 (fast DRAM)  →  simulated HBM (hot tier)
Node 1 (slower)     →  simulated HBF (warm tier)
Storage             →  cold tier (not yet controlled)
```

The NUMA backend is a proxy for future CXL-attached memory, HBF hardware, or any byte-addressable capacity tier. The control plane is backend-independent.

## Quick Start

```bash
# Clone kernel and apply patches
export KERNEL_TREE=/path/to/linux
cd $KERNEL_TREE && git am /path/to/patches/000*.patch

# Build with HBF enabled
./scripts/config -e CONFIG_HBF_CONTROL_PLANE
make -j$(nproc)

# Build without HBF (no-regression check)
./scripts/config -d CONFIG_HBF_CONTROL_PLANE
make -j$(nproc)

# Boot in QEMU
./scripts/qemu-test.sh

# Build and use the CLI
cd samples/hbf && gcc -Wall -O2 -o hbfctl hbfctl.c -lnuma
./hbfctl caps
./hbfctl submit 0x7f1234560000 2097152 -o 2 -n 0 -d 1000000000
./hbfctl query 1
```

## Patch Series

| # | Commit | Files |
|---|---|---|
| 1 | `doc: mm: describe runtime-guided memory tiering` | `Documentation/mm/hbf-control-plane.rst` |
| 2 | `mm: hbf: add internal memory range hint types` | `mm/hbf/*.h`, `include/uapi/linux/hbf.h` |
| 3 | `mm: hbf: add observability infrastructure` | `hbf_tracepoints.h`, `hbf_debugfs.c` |
| 4 | `mm: hbf: add request lifecycle and ioctl frontend` | `hbf_request.c`, `hbf_ioctl.c`, `hbf_main.c`, Kconfig |
| 5 | `mm: hbf: add NUMA migration backend` | `hbf_backend.c`, `hbf_numa.c` |
| 6 | `selftests: mm: add HBF tests` | 4 test programs in `tools/testing/selftests/mm/` |
| 7 | `samples: hbf: add hbfctl and bpftrace tools` | CLI tool, 3 bpftrace scripts |

## Repository Layout

```
patches/                    — 7 kernel patches (generated from commits)
benchmarks/                 — sequential, random, promote-pressure, memory-pressure
scripts/                    — build, QEMU test, style check, patch regeneration
docs/                       — architecture, design rationale, API options, etc.
samples/hbf/hbfctl.c        — userspace CLI tool
tools/hbftrace/*.py         — bpftrace observability scripts
```

## Architecture

```
AI Runtime → hbfctl hints → request queue → NUMA backend → page migration → hot/warm placement
                    ↓            ↓
               tracepoints   debugfs stats
```

Key principles:
- **Advisory hints**: never correctness-critical
- **Async execution**: SUBMIT returns request_id immediately
- **Internal-first types**: core uses `mm_tier_range_hint`; ioctl is a replaceable adapter
- **Observability-first**: every lifecycle event captured via tracepoints
- **No LKML submission**: this is a local research platform

## Acceptance Criteria Met

- [x] Kernel builds with `CONFIG_HBF_CONTROL_PLANE=y` and `=n`
- [x] 7 commits, each independently buildable
- [x] `/dev/hbfctl` registeres as miscdevice
- [x] SUBMIT returns request_id without blocking
- [x] PROMOTE moves pages to hot node
- [x] DEMOTE moves pages to warm node
- [x] Invalid hints return proper errors (EINVAL, EFAULT, EPERM)
- [x] CANCEL sets request to CANCELLED state
- [x] Process exit cleans up queued requests
- [x] Tracepoints visible via `perf list | grep hbf`
- [x] debugfs `stats` shows counters

## v0.3+ Roadmap

- CXL Type-3 memory expander backend
- DAX/kmem-backed warm nodes
- Admission control with cost-benefit scoring
- Anti-thrashing hysteresis
- Cgroup per-tenant budgets
- KV-cache trace-replay benchmark
- API bake-off (madvise vs ioctl vs shared ring)
- Prediction-quality sensitivity analysis

## References

See [docs/references.md](docs/references.md) for upstream documentation links.
