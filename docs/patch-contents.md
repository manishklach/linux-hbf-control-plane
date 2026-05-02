# Patch Contents

This document explains what the prototype patch in [rfc-hbf-linux-control-plane.patch](../rfc-hbf-linux-control-plane.patch) contains.

It is supporting material only. The patch is a local RFC artifact for this repository. It is not being submitted from here.

## What The Patch Is

The patch is a prototype kernel-side artifact that packages a proposed HBF control plane as an RFC-style patch mail with:

- a cover letter
- Kconfig and Makefile integration
- a new `drivers/memory/hbf/` implementation sketch
- a new UAPI header
- a small kernel-internal registration API
- tracepoints
- a basic selftest

The patch is trying to make the control-plane discussion concrete enough that maintainers can evaluate the proposed abstraction, not just the idea in the abstract.

## Cover Letter Framing

The patch cover letter argues that future high-capacity warm memory tiers for AI inference should not be modeled only as block storage. It proposes a path where:

- a runtime provides hints through `/dev/hbfctl`
- a generic HBF policy layer ingests those hints
- a future backend uses CXL, DMA, migration, or related mechanisms
- resulting data placement spans HBM, DRAM, and a warm high-capacity tier

The patch also says it is avoiding vendor-specific hardware details and is intended as an abstraction discussion.

## Files Touched

The patch adds or modifies:

- `MAINTAINERS`
- `drivers/memory/Kconfig`
- `drivers/memory/Makefile`
- `drivers/memory/hbf/Kconfig`
- `drivers/memory/hbf/Kconfig.debug`
- `drivers/memory/hbf/Makefile`
- `drivers/memory/hbf/core.c`
- `drivers/memory/hbf/trace.h`
- `drivers/memory/hbf/trace.c`
- `include/uapi/linux/hbf.h`
- `include/linux/hbf.h`
- `tools/testing/selftests/hbf/Makefile`
- `tools/testing/selftests/hbf/hbf_hint_test.c`

## Main Kernel Surfaces Introduced

### 1. UAPI

The patch adds `include/uapi/linux/hbf.h` with:

- `enum hbf_object_class`
- `enum hbf_hint_op`
- `struct hbf_user_hint`
- `HBF_IOC_SUBMIT_HINT`

The UAPI is meant to let a userspace runtime describe:

- address range
- length
- operation
- flags
- runtime object id
- optional target NUMA node
- optional timing or reuse hints

### 2. Kernel-Internal Registration API

The patch adds `include/linux/hbf.h` with:

- `struct hbf_dev_ops`
- `struct hbf_dev_desc`
- `struct hbf_device`
- `hbf_register_device()`
- `hbf_unregister_device()`

This is intended to let future transport-specific or vendor-specific backends register with a generic HBF core.

### 3. Generic HBF Core

The patch adds `drivers/memory/hbf/core.c`, which provides:

- a miscdevice named `/dev/hbfctl`
- a device class under `/sys/class/hbf/`
- sysfs attributes such as capacity and bandwidth metadata
- ioctl handling for `HBF_IOC_SUBMIT_HINT`
- basic hint validation
- simple dispatch logic to a registered device

The dispatch policy in the prototype is intentionally simple:

- prefer a device matching `target_node` if provided
- otherwise use the first registered device

### 4. Tracepoints

The patch adds:

- `hbf_hint_submit`
- `hbf_hint_complete`

These are intended to expose whether a hint was submitted and what return code came back from the dispatch path.

### 5. Selftest

The patch adds a small selftest under `tools/testing/selftests/hbf/` that:

- allocates a test buffer
- opens `/dev/hbfctl`
- submits a prefetch hint
- tolerates missing device state as a skip or no-device case

## What The Patch Tries To Model

The patch is trying to model a control plane for:

- prefetch
- promote
- demote
- pin or retain-in-warm-tier behavior
- release of prior urgency

It treats hints as advisory rather than correctness-critical.

## What The Patch Does Not Contain

The patch does not contain:

- a real hardware driver
- a public HBF hardware binding
- a CXL DVSEC definition
- real DMA engine programming
- migration integration with core MM
- cgroup accounting
- memcg charging policy
- a complete security model
- a benchmark or proof of performance benefit

## What The Patch Implies Architecturally

The prototype implies a possible architecture where:

- userspace has semantic knowledge of future memory value
- the kernel ingests that knowledge as hints
- a future backend decides whether and how to act

That makes the patch more about control-plane shape than about immediate hardware enablement.

## Important Caveats

The patch is useful as prototype material, but it should not be read as proof that:

- `drivers/memory/hbf/` is the right permanent subsystem home
- a new ioctl UAPI is the right final interface
- the current naming is acceptable upstream
- AI-specific object classes belong in a stable kernel ABI
- public HBF hardware details are already settled

Those questions remain open and are discussed in the other review documents in this repository.
