# Control Plane

## Overview

The proposed control plane exists because the runtime and kernel each know something the other does not.

Target model:

- HBF-like warm capacity exposed through CXL Type-3, DAX, or memory hotplug
- control-plane logic living close to `mm/`
- `/dev/hbfctl` used only as an RFC frontend if present at all

The runtime knows:

- tokens
- sequences
- KV blocks
- approximate reuse distance
- deadlines and upcoming attention windows

The kernel knows:

- pages and folios
- VMAs and mappings
- NUMA nodes
- memory tiers
- DMA engines or migration backends
- DAX mappings and driver-managed memory

The translation problem is converting runtime semantic value into safe physical placement decisions without making correctness depend on the hint path.

## Runtime Semantic Layer

Inference runtimes do not naturally think in pages. They think in:

- prompt ranges
- decoded token progression
- KV-cache block lifetimes
- sequence eviction priority
- near-term reuse probability
- latency deadlines for the next compute phase

That information is often available earlier than any CPU or accelerator fault that would reveal demand to the kernel.

## Kernel Physical Layer

Linux ultimately acts on:

- virtual address ranges owned by a process
- folios and page state
- node locality
- migration and demotion paths
- DAX and memory-hotplug surfaces
- backend queueing, DMA, and capacity constraints

The kernel cannot trust userspace to dictate final placement, but it can potentially use userspace intent as one more advisory input.

## Hint Lifecycle

### `PREFETCH`

Meaning:

- stage data toward a more useful tier before demand arrives

Typical intent:

- "this range will be touched soon"

### `PROMOTE`

Meaning:

- prefer migration toward a faster tier

Typical intent:

- "this range is becoming hot"

### `DEMOTE`

Meaning:

- prefer migration toward a slower but higher-capacity tier

Typical intent:

- "this range is cooling but should remain warm"

### `RELEASE`

Meaning:

- remove prior urgency or placement preference

Typical intent:

- "the runtime no longer expects this range to matter soon"

## Failure Semantics

Hints are advisory.

That means:

- the kernel may ignore a hint
- the kernel may reject a hint
- the backend may lack support
- the kernel may partially act on a hint
- the eventual placement may differ from the request

Hints must never be correctness-critical. They are performance signals only.

## Security Model

The default expectation should be:

- an unprivileged process may only hint its own mappings
- privileged control of other mappings would require explicit capability and policy
- future versions need cgroup-aware budgets and accounting
- backends must remain free to reject hints based on policy or pressure

The current prototype patch does not yet provide a complete security story. That is acceptable for RFC discussion only if the limitation is acknowledged clearly.

For v1, pinning is intentionally omitted from the patch UAPI. If it returns later, it should likely be privileged and justified separately.

## Observability

Any serious control plane needs visibility into whether hints mattered. A plausible future tracepoint set would include:

- `hint_received`
- `hint_accepted`
- `hint_dropped`
- `prefetch_started`
- `prefetch_completed`
- `fault_after_hint`

Without that observability, the interface risks becoming impossible to debug and impossible to justify.
