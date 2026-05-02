# No Block Layer

## Why HBF Should Not Primarily Be Modeled As A Block Device

The core problem in this repository is not durable storage access. The core problem is timely placement of warm memory before compute asks for it.

If an HBF-like tier is modeled primarily as a block device, the software stack naturally converges on:

- block I/O submission
- completion after demand is already visible
- file or block offsets instead of process address ranges
- a storage-centric latency model

That is a poor fit for warm context staging. It turns the idea into "faster storage" instead of a true warm memory tier.

## Why CXL, DAX, And Memory Tiering Are Better Targets

The better target model is:

- CXL Type-3 capacity surfaced through DAX or memory hotplug
- warm-tier awareness near `mm/`
- eventual interaction with migration, memory tiers, and mempolicy

Those mechanisms already speak the language that matters here:

- address ranges
- pages and folios
- NUMA nodes
- migration and demotion
- driver-managed memory and DAX mappings

That makes them a better architectural home than a new block-first model.

## Why `blk-mq` Or Filesystem Changes Are Not Required For The First Serious RFC

The first serious RFC only needs to answer:

- is a runtime-to-kernel hint boundary useful?
- can the kernel validate and admit range hints safely?
- can that hint path eventually steer mm-tier behavior?

None of those questions require:

- new block-layer queueing policy
- filesystem changes
- NVMe protocol changes

Those areas only become relevant if the chosen backend is storage-shaped rather than memory-tier-shaped, and that is explicitly not the primary target model here.

## When Block Or NVMe Can Still Be Useful

Block or NVMe devices can still be useful as proxy benchmark infrastructure:

- emulating a slower warm tier
- measuring hint-to-touch latency under staged reads
- comparing demand-only access against hinted access

That use is experimental and evaluative. It is not the claim that block semantics are the right permanent kernel model.
