# Kernel Integration Map

This map reflects the updated target model: HBF-like capacity exposed through CXL Type-3, DAX, or memory hotplug, with the control plane living closer to `mm/` and `Documentation/mm/` than a standalone driver subtree.

## `drivers/cxl/core/`

Why it may be relevant:

- central CXL object model and region/device lifecycle
- natural place to understand where Type-3 capacity becomes visible to the rest of the kernel

Why touching it too early is risky:

- easy to make a transport-neutral memory-tier idea look prematurely CXL-specific

## `drivers/cxl/mem.c`

Why it may be relevant:

- CXL Type-3 memory device plumbing
- likely early backend touchpoint if warm-tier capacity is CXL-backed

Why touching it too early is risky:

- pushes backend assumptions into the first RFC before the hint abstraction itself is accepted

## `drivers/cxl/region.c`

Why it may be relevant:

- region construction and capacity exposure
- relevant if placement hints eventually need to understand region characteristics

Why touching it too early is risky:

- region policy is not the same thing as a userspace hint ABI

## `drivers/dax/kmem.c`

Why it may be relevant:

- bridge from device-backed memory to memory-hotplug-managed system memory
- strong example of how warm-tier capacity may reach the page allocator

Why touching it too early is risky:

- can make the proposal look DAX-specific when the intended abstraction is broader

## `mm/memory_hotplug.c`

Why it may be relevant:

- relevant when warm-tier capacity is onlined into normal memory
- important for any eventual story around dynamic capacity appearance

Why touching it too early is risky:

- hotplug flows are subtle and policy-heavy

## `mm/memory-tiers.c`

Why it may be relevant:

- natural long-term home for promote and demote semantics
- current kernel tiering decisions already operate here

Why touching it too early is risky:

- global tiering policy should not be rewritten around an immature RFC hint path

## `mm/migrate.c`

Why it may be relevant:

- real promote or demote work will likely intersect migration logic

Why touching it too early is risky:

- migration semantics are easy to get wrong and expensive to stabilize

## `mm/mempolicy.c`

Why it may be relevant:

- possible long-term fit for range policy and locality semantics

Why touching it too early is risky:

- user-visible MM policy APIs are not a good place for an unproven first draft

## `include/linux/migrate.h`

Why it may be relevant:

- shared migration declarations if the hint path eventually interacts with migration helpers

Why touching it too early is risky:

- header churn without a clear execution model just spreads an unresolved design

## `Documentation/driver-api/cxl/`

Why it may be relevant:

- explains how CXL memory and DAX surfaces behave today
- useful grounding for the target model

Why touching it too early is risky:

- documentation references should inform the RFC, not force a subsystem decision

## `Documentation/mm/`

Why it may be relevant:

- best place to explain the abstraction in memory-management terms first

Why touching it too early is risky:

- less risky than code churn, but still needs precise claims and no invented hardware facts
