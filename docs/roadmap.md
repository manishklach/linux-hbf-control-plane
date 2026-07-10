# Roadmap

## v0 — Foundation (*shipped*)

- Documentation set
- UAPI sketch
- Char device skeleton

## v1 — Observability (*shipped*)

- Tracepoints (11 events in `include/trace/events/hbf.h`)
- DebugFS interface (`/sys/kernel/debug/hbf/`)
- Per-request statistics

## v2 — NUMA Tier Backend (*shipped as v0.2*)

- Request lifecycle with async queue (system_unbound_wq)
- 4 ioctls via /dev/hbfctl (SUBMIT, QUERY, CANCEL, CAPS)
- Folio-based NUMA page migration
- Userspace hbfctl CLI tool and bpftrace scripts
- QEMU boot-verified

## v3 — Multi-node & Hint Scheduling

- Multi-node NUMA testing (QEMU with -numa topology, real HW)
- Hint merge logic for overlapping ranges
- Deadline-aware priority queue
- Anti-thrashing hysteresis
- Expanded test suite for concurrent submit/cancel

## v4 — CXL/DAX Backend & Runtime Integration

- Pluggable backend framework
- CXL type-2 device migration path
- Runtime integration with `vLLM` and `llama.cpp`-style KV-cache traces
- Trace replay harnesses
- Policy experiments around deadlines and user tags
