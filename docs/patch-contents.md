# Patch Contents

This document explains what the current prototype patch in [rfc-hbf-linux-control-plane.patch](../rfc-hbf-linux-control-plane.patch) contains.

It is supporting material only. The patch is a local RFC artifact for this repository. It is not being submitted from here.

## What The Patch Is

The patch is a prototype kernel-side artifact generated as a real git patch. It now frames HBF-like capacity as an `mm`/CXL/DAX memory-tier hint proposal rather than a standalone driver-first subsystem.

The target model is:

- HBF-like warm capacity exposed through CXL Type-3, DAX, or memory hotplug
- memory-tier hints handled close to `mm/`
- `/dev/hbfctl` treated only as an RFC frontend, not a long-term ABI claim

## Files Touched

The patch adds or modifies:

- `Documentation/mm/hbf-control-plane.rst`
- `include/uapi/linux/hbf.h`
- `mm/Kconfig`
- `mm/Makefile`
- `mm/hbf.c`
- `samples/hbf/hbfctl-demo.c`
- `tools/testing/selftests/mm/hbf_hint_abi.c`

## Main Kernel Surfaces Introduced

### 1. Documentation

The patch adds `Documentation/mm/hbf-control-plane.rst` to describe:

- the target model for HBF-like warm capacity
- why block-first modeling is not the primary goal
- why the RFC should live near `Documentation/mm/` and `mm/`
- why `/dev/hbfctl` is at most a temporary frontend

### 2. UAPI

The patch adds `include/uapi/linux/hbf.h` with:

- `enum hbf_hint_op`
- `struct hbf_hint_range`
- `HBF_IOC_HINT`

The operation set is intentionally small in v1:

- `HBF_HINT_PREFETCH`
- `HBF_HINT_PROMOTE`
- `HBF_HINT_DEMOTE`
- `HBF_HINT_RELEASE`

`HBF_HINT_PIN` is intentionally omitted from this patch.

### 3. `mm` Skeleton

The patch adds `mm/hbf.c`, which provides:

- `hbf_hint_range(struct mm_struct *mm, unsigned long start, unsigned long len, ...)`
- basic range validation
- opcode validation
- an `-EOPNOTSUPP` stub return after validation
- TODO markers pointing toward:
  - `mm/migrate.c`
  - `mm/memory-tiers.c`
  - `mm/mempolicy.c`
  - `drivers/cxl/core/`
  - `drivers/cxl/mem.c`
  - `drivers/cxl/region.c`
  - `drivers/dax/kmem.c`

The current implementation does **not** perform real migration or prefetch work.

### 4. RFC Frontend

The patch currently includes a miscdevice-based `/dev/hbfctl` frontend in `mm/hbf.c`.

That is explicitly an RFC-only frontend:

- it provides a mechanical way to ingest hints
- it is not presented as the required long-term ABI
- future review may replace it with `madvise()`, mempolicy, cgroup policy, or no new ABI at all

### 5. Sample And Selftest

The patch adds:

- `samples/hbf/hbfctl-demo.c`
- `tools/testing/selftests/mm/hbf_hint_abi.c`

These are there to make the ABI shape concrete. They do not prove backend usefulness or upstream suitability.

## What The Patch Does Not Contain

The patch does not contain:

- a real HBF hardware driver
- a standalone `drivers/hbf/` subsystem
- a block-layer implementation
- real migration execution
- real memory-tier placement policy
- tracepoint implementation
- cgroup accounting
- memcg charging
- benchmark claims

## Architectural Meaning

The patch is no longer asking Linux to accept a new driver-first HBF subsystem. It is asking whether a range-hint control plane for future warm capacity tiers belongs near existing MM, CXL, and DAX machinery.
