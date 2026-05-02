# Architecture

## Overview

The proposed HBF control plane is not a claim that Linux already has an HBF subsystem. It is an RFC sketch for how a future high-capacity, warm-memory tier could be integrated into Linux in a way that is useful to AI inference runtimes.

The central idea is simple: the runtime knows semantic intent before hardware demand arrives, while the kernel is responsible for physical placement, migration, accounting, and observability.

## Why HBF Should Not Be Modeled Only as Block Storage

Treating HBF as a fake SSD or ordinary block device forces the runtime onto a demand-driven I/O path:

- queue I/O after the miss is already visible
- complete data movement after compute is stalled
- reason in file offsets and request queues instead of placement state

That can be appropriate for cold storage, but it is a poor fit for a warm context tier meant to stage KV-cache or prompt state ahead of use. If the software stack only discovers need at the moment of access, HBF collapses into "faster storage" rather than becoming a true tier in a memory hierarchy.

The more plausible model is a control plane that cooperates with:

- byte-addressable mappings
- hotplugged memory tiers
- page migration and promotion/demotion
- accelerator-facing staging paths

## Path Comparison

### Block I/O Path

The block path is organized around requests, bios, queues, and completion of explicit read/write operations. It is optimized for durable or semi-durable storage semantics, not for low-latency promotion of warm memory state.

Best fit:

- filesystems
- swap-like spill
- cold checkpoints

Poor fit:

- fine-grained warm context staging
- proactive placement before page faults
- expressing runtime reuse intent

### DAX / Byte-Addressable Path

DAX exposes capacity through direct mappings rather than page cache mediation. This is useful when user space or the kernel wants memory-like access to device-backed ranges and when the system needs a path that looks closer to memory than storage.

Relevant benefits:

- memory-like access semantics
- explicit `mmap()`-driven usage
- clean relationship to `dax_kmem` and memory hotplug

### CXL-Attached Memory Path

CXL Type-3 memory is the closest upstream ecosystem match for this RFC. Linux already documents how CXL capacity may be surfaced as:

- a DAX device for direct mappings, or
- normal memory via memory hotplug and the page allocator

That makes CXL a sensible anchor for a future HBF-like control plane, even if no public HBF hardware binding exists today.

### Accelerator-Local HBM Path

HBM is the hot tier closest to active compute. It is capacity-constrained but bandwidth-optimized. Inference runtimes want HBM to hold the currently active working set, not the entire long-tail context.

That makes HBF conceptually different from HBM:

- HBM is the active execution tier
- HBF is the warm staging tier
- SSD or object storage remains the cold persistence tier

## Runtime Hint Model

This RFC centers on a minimal hint interface. The kernel remains free to reject, defer, downgrade, or reinterpret hints based on policy, permissions, memory pressure, and backend capability.

### `HBF_HINT_PREFETCH`

Request that a range be staged toward a nearer or more immediately usable tier before compute demand arrives.

Use case:

- upcoming KV-cache segments predicted from decode order

### `HBF_HINT_PROMOTE`

Request migration or placement toward a faster tier because a range is expected to become hot soon.

Use case:

- prompt segments about to enter active attention windows

### `HBF_HINT_DEMOTE`

Request migration toward a slower but higher-capacity tier because a range is expected to cool.

Use case:

- older context blocks with lower near-term reuse probability

### `HBF_HINT_PIN`

Request temporary placement stability for data that should not be displaced while an operation is in flight.

Use case:

- short critical regions during accelerator submission

### `HBF_HINT_RELEASE`

Release previously expressed urgency or pinning so the kernel can resume normal placement and reclaim policy.

Use case:

- context range no longer needed by the next inference window

## Possible Future Kernel Integration Points

This RFC does not assume the first skeleton is the final subsystem home. Plausible long-term integration points include:

- `drivers/cxl/`
- `drivers/dax/`
- `mm/migrate.c`
- memory tiering
- NUMA balancing
- `dma-buf` or HMM-style heterogeneous memory plumbing
- tracepoints and eBPF observability

Each option reflects a different center of gravity:

- CXL if the dominant abstraction is tiered attached capacity
- DAX if byte-addressable device-backed mappings are the entry point
- MM if the abstraction becomes page-placement policy first
- heterogeneous memory frameworks if accelerators become first-class consumers

## Suggested Control-Plane Shape

For an RFC, a thin control plane is enough:

- a small UAPI struct describing address, length, operation, flags, deadline, and a user tag
- a char device or similarly isolated control endpoint
- backend-specific translation into migration, prefetch, or staging work

That keeps the experiment focused on kernel abstraction rather than pretending the hardware contract is already settled.
