# Likely LKML Objections

This document is intentionally blunt. The goal is not to "win" an argument. The goal is to know where the prototype is weak before wasting reviewer attention.

## Why Is This Not Just `madvise()`?

Strong answer:

`madvise()` is a plausible long-term home if the kernel decides that range-based advisory placement hints are valid. The reason to start elsewhere for RFC work is isolation: a miscdevice prototype lets us test a proposed hint vocabulary and observability model without immediately expanding a mature MM ABI.

Weaker or uncertain answer:

If reviewers conclude that only an MM-native interface is acceptable, a standalone ioctl path may be viewed as unnecessary churn.

What would strengthen the answer:

- a stripped-down prototype showing which semantics map cleanly to `madvise()` and which do not
- evidence that deadlines, staging intent, or backend-specific admission feedback do not fit well in existing advice mechanisms

## Why Is This Not Part Of CXL?

Strong answer:

CXL is the nearest upstream ecosystem match for byte-addressable or driver-managed high-capacity memory tiers, but the control-plane problem may outlive any single transport. Starting as a transport-neutral RFC keeps the discussion focused on the runtime-to-kernel hint boundary rather than on a specific bus.

Weaker or uncertain answer:

If the first credible implementation target is CXL Type-3 capacity surfaced as DAX or kmem, reviewers may reasonably insist that the discussion happen inside the CXL orbit from day one.

What would strengthen the answer:

- a clear map showing how the same hint model could target CXL-backed DAX, migration-backed pages, or accelerator-adjacent memory
- reviewer feedback from CXL maintainers on whether a neutral control plane helps or only adds indirection

## Why Is This Not Part Of DAX?

Strong answer:

DAX is relevant because it already represents memory-like device capacity. But the proposal is not only about direct mappings; it is also about promotion, demotion, staging, and interaction with tiered system memory. That may extend beyond a DAX-only framing.

Weaker or uncertain answer:

If all realistic early backends are DAX-facing, reviewers may see a separate top-level abstraction as overreach.

What would strengthen the answer:

- examples where the same hint API would govern both DAX-backed ranges and page-allocator-visible hotplugged memory
- a narrower explanation of why DAX alone is not the full abstraction boundary

## Why Is This Not Just Userspace Runtime Policy?

Strong answer:

Userspace can predict value, but it cannot safely execute migration, admission control, DMA staging, memcg accounting, or kernel placement decisions by itself. The kernel owns the physical layer and must remain authoritative over what actually happens.

Weaker or uncertain answer:

If the only thing the proposal adds is a new naming layer for userspace intent without any kernel-side leverage, reviewers will rightly ask why it exists.

What would strengthen the answer:

- prototypes showing kernel decisions informed by hints but constrained by policy
- evidence that userspace alone cannot bridge the gap without duplicating kernel placement state

## Why Does This Need A New Char Device?

Strong answer:

For RFC isolation only. A miscdevice is the least disruptive way to test whether the control surface is even understandable before entangling it with MM syscalls or transport-specific interfaces.

Weaker or uncertain answer:

The char device may never be accepted upstream. It may only be a temporary scaffold for discussion.

What would strengthen the answer:

- proof that the char device stays tiny and does not accrete policy
- a credible migration path to `madvise()`, cgroup policy, CXL integration, or deletion

## What Hardware Exists?

Strong answer:

No public hardware programming model is assumed in this repository. The RFC is about a possible OS abstraction for future high-capacity warm tiers, not a claim of immediate hardware enablement.

Weaker or uncertain answer:

Without a concrete hardware model, some reviewers will consider the proposal too speculative to justify new UAPI.

What would strengthen the answer:

- proxy experiments using CXL memory, DAX, pmem, or migration-backed tiers
- public references showing adjacent memory-tier behavior the kernel already handles

## What Are The Security Boundaries?

Strong answer:

Hints must be advisory, scoped to the calling process's own mappings unless privileged, and subject to backend admission control. Future versions need explicit accounting, cgroup policy, and traceability.

Weaker or uncertain answer:

The current prototype patch does not yet have a convincing security and isolation story.

What would strengthen the answer:

- a documented access-control model
- explicit range ownership validation strategy
- rate limits and memcg or cgroup budget discussion

## How Are cgroups And Accounting Handled?

Strong answer:

They are not solved in the prototype, and that is a serious limitation. Any upstream path would need an accounting story before hints can drive real resource movement.

Weaker or uncertain answer:

This may be enough to block the entire design until there is a better policy model.

What would strengthen the answer:

- a proposal for memcg-aware admission and per-cgroup budgets
- examples of how hint traffic and migration work would be charged

## How Does This Avoid Becoming A Vendor Dumping Ground?

Strong answer:

By keeping the core interface extremely small, requiring hints to remain advisory, and refusing vendor-specific hardware properties in the generic layer. The core should only describe admission and intent, not hardware-specific scheduling contracts.

Weaker or uncertain answer:

A new top-level subsystem always risks attracting backend-specific policy if its scope is not tightly controlled.

What would strengthen the answer:

- a very small generic ABI
- explicit rules for what cannot enter the core
- evidence that transport-specific code can stay in existing subsystems

## Why Should The Kernel Care About AI/KV Cache Semantics?

Strong answer:

The kernel should not care about AI branding. It should care about whether userspace can provide earlier, useful signals for memory placement. KV-cache is only an example workload that makes the timing problem obvious.

Weaker or uncertain answer:

If the UAPI bakes too much AI language into stable interfaces, reviewers will reject it as workload-specific policy.

What would strengthen the answer:

- a more generic hint vocabulary
- examples outside AI inference where warm-tier staging also matters
- evidence that the same mechanism can express generic reuse and urgency, not just AI labels
