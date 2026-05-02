# API Options

The updated prototype patch uses an ioctl on `/dev/hbfctl` only as an RFC frontend. That is acceptable for isolation, but it is not a commitment that the long-term upstream form should remain a char device.

## Comparison

| API surface | Invasiveness | ABI risk | Ease of prototyping | Fit for address ranges | Fit for per-process policy | Fit for system-wide policy | Likely LKML acceptability |
| --- | --- | --- | --- | --- | --- | --- | --- |
| ioctl on `/dev/hbfctl` | Medium | Medium to high | High | Good | Medium | Poor | Mixed |
| `madvise()` extension | High | High | Medium | Very good | Good | Poor | Mixed to good if semantics are tight |
| `prctl()` | Medium | Medium | Medium | Poor | Good | Poor | Weak fit |
| `sysfs` | Low to medium | Medium | Medium | Poor | Poor | Good | Weak for per-range hints |
| `netlink` | Medium | Medium | Medium | Medium | Medium | Good | Mixed |
| cgroup controller | High | High | Low | Poor | Medium | Very good | Plausible only after policy matures |
| `io_uring` opcode | High | High | Low to medium | Good | Medium | Poor | Likely skeptical early |
| perf/eBPF-only observability first | Low | Low | High | None | Indirect | Good | Strong for early instrumentation |

## Notes By Option

### ioctl on `/dev/hbfctl`

Why it helps:

- isolates the RFC from core MM ABI commitments
- easy to mock locally
- can carry richer structs during early discussion

Why it is risky:

- new device interfaces are easy to add and hard to delete
- it may look like a vendor staging area
- it may never be accepted upstream

### `madvise()` Extension

Why it helps:

- natural fit for virtual ranges
- clearly tied to process memory
- avoids a separate device interface if the kernel decides the concept belongs in MM

Why it is risky:

- hardens semantics early
- forces stronger review expectations immediately

### `prctl()`

Useful only if the primary abstraction is process-scoped policy rather than per-range hints. That does not match the current problem very well.

### `sysfs`

Good for static capability and configuration, not for frequent per-range hint traffic.

### `netlink`

Could make sense for richer control-plane messaging, but it is more machinery than a first RFC needs.

### cgroup Controller

Probably relevant eventually for admission control and budgets, but too early for a first RFC. It does not replace a range-level hint surface.

### `io_uring` Opcode

Interesting only if the control plane becomes deeply asynchronous and latency-sensitive. That would be a poor first move without proving the core abstraction first.

### perf/eBPF-Only Observability First

A very credible conservative path is to add only observability around migration, prefetch, and faults first, then decide later whether a new hint ABI is justified.

## Recommendation

Start with a char device or miscdevice only for RFC isolation.
Keep the real design center in `mm/`, CXL, DAX, and memory-tier policy rather than a driver-first subsystem story.

But be explicit that the long-term upstream form may be:

- `madvise()`
- cgroup or mempolicy integration
- CXL or DAX-specific policy hooks
- pure observability plus userspace experimentation
- no new ABI at all

That honesty is part of making the RFC credible.
