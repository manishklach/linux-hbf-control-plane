# Kernel Integration Map

This map is intentionally speculative. It describes where the concept might intersect Linux, and why touching each area too early can backfire.

## `drivers/cxl/`

Why it may be relevant:

- CXL Type-3 memory is the closest upstream ecosystem match for high-capacity attached memory tiers
- Linux already documents paths from CXL to DAX and memory hotplug

Why touching it too early is risky:

- it may prematurely imply that the abstraction is CXL-specific
- CXL maintainers may reasonably reject a transport-neutral policy layer dropped into their tree without proof

## `drivers/dax/`

Why it may be relevant:

- DAX is the existing byte-addressable path for device-backed memory-like capacity
- some future warm-tier capacity may surface as DAX mappings

Why touching it too early is risky:

- DAX alone may not cover migration-driven or page-allocator-visible implementations
- reviewers may see a DAX tie-in as hiding an unresolved MM policy question

## `mm/memory-tiers.c`

Why it may be relevant:

- promotion and demotion are ultimately tiering problems
- future integration may want to cooperate with existing memory-tier logic

Why touching it too early is risky:

- tiering policy is sensitive and global
- an immature hint surface should not be wired directly into core tiering decisions

## `mm/migrate.c`

Why it may be relevant:

- any real promote or demote path will eventually intersect migration logic

Why touching it too early is risky:

- migration behavior is subtle
- adding a new externally visible hint source before admission and accounting are clear would be premature

## `mm/mempolicy.c`

Why it may be relevant:

- locality preferences and range policy may eventually map better to mempolicy than to a custom device ABI

Why touching it too early is risky:

- mempolicy semantics are user-visible and established
- a speculative abstraction can do more damage here than in an isolated RFC skeleton

## `include/uapi/linux/`

Why it may be relevant:

- any real userspace hint API eventually lands here

Why touching it too early is risky:

- UAPI is the hardest thing to retract
- workload-specific naming mistakes become expensive quickly

## `samples/`

Why it may be relevant:

- a sample can explain the interface more clearly than prose

Why touching it too early is risky:

- samples can make immature ABIs look more settled than they are

## `tools/testing/selftests/`

Why it may be relevant:

- selftests are the right place for mechanical ABI validation

Why touching it too early is risky:

- tests can validate plumbing while leaving the core design unjustified
- adding selftests without stronger semantics can be more theater than evidence

## `trace/events/`

Why it may be relevant:

- observability is essential if hints are advisory

Why touching it too early is risky:

- event schema becomes another interface surface
- tracepoints should follow a clearer model of accepted, rejected, and acted-upon hints
