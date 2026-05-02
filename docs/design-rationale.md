# Design Rationale

## Goal

The purpose of this RFC is to make a possible runtime-to-kernel control plane discussable without prematurely hard-coding it into the MM ABI. The proposed interface is intentionally modest: a small hint object and a narrow control endpoint.

## API Shapes Considered

### ioctl on a Char Device

Pros:

- isolated experiment boundary
- easy to evolve during RFC discussion
- does not immediately commit core MM ABI
- straightforward for samples and prototyping

Cons:

- can look driver-like even when the long-term home may be MM or CXL
- less idiomatic than an eventual memory-policy API

### `madvise()` Extension

Pros:

- naturally operates on virtual address ranges
- semantically close to placement and prefetch intent
- familiar to user space

Cons:

- changes the MM ABI early
- raises sharper questions around semantics, guarantees, and long-term support
- harder to revise once exposed

### `prctl()`

Pros:

- useful for process-scoped policy knobs

Cons:

- weak fit for fine-grained range-level hints
- likely too coarse for KV-cache or context-window control

### `sysfs`

Pros:

- simple for static configuration

Cons:

- poor fit for per-range, latency-sensitive hint traffic
- awkward for runtime-driven control messages

### `netlink`

Pros:

- extensible message format
- good for structured control planes

Cons:

- heavier than needed for an early per-process hint API
- adds complexity before the kernel object model is settled

### cgroup Controller

Pros:

- strong long-term fit for accounting, limits, and policy
- natural place for multi-tenant controls

Cons:

- too policy-heavy for an initial RFC
- does not replace the need for a per-range hint surface

## Recommendation for RFC v0

Start with a char device plus ioctl interface.

That is the least disruptive way to test whether the abstraction itself is interesting. It isolates the proposal, keeps the sample concrete, and avoids prematurely enshrining behavior in a mature MM-facing syscall ABI.

## Why Later Versions May Move

If the idea proves useful, later revisions may want to migrate toward:

- `madvise()` for range-level semantics
- cgroup integration for accounting and policy
- CXL or DAX-native integration if that becomes the dominant backend
- MM hooks if promotion/demotion decisions become tightly coupled to existing migration paths

The RFC should not assume that v0 placement of the interface determines the final kernel home.

## Why Prefetch Must Happen Before Demand

The core systems argument is timing.

If the kernel only starts staging data after a CPU or accelerator fault exposes demand, then HBF mostly behaves like a faster SSD tier with better bandwidth but the same basic lateness problem. The value of HBF comes from moving warm context before compute blocks on it.

That is why the runtime hint path matters:

- the runtime can see decode order and reuse patterns earlier
- the kernel can turn that intent into migration or staging work
- the system can hide part of the latency behind useful compute

Without that early hinting loop, the architectural advantage of a warm intermediate tier is significantly reduced.
