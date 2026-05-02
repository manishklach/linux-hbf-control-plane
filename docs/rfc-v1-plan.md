# RFC v1 Plan

Working series title:

`[RFC PATCH 0/4] hbf: explore runtime-guided control plane for high-capacity AI memory tiers`

This plan assumes the new target model:

- HBF-like capacity exposed through CXL Type-3, DAX, or memory hotplug
- warm memory tier semantics, not block-device semantics
- code living near `Documentation/mm/` and `mm/`
- `/dev/hbfctl` allowed only as an RFC frontend

## 1/4 Documentation: add memory-tier control-plane design note

Purpose:

- explain the memory-tier problem in `Documentation/mm/`
- say clearly why the first RFC is not driver-first
- explain why block layering is not the primary target

Files touched:

- `Documentation/mm/hbf-control-plane.rst`

## 2/4 uapi: add experimental HBF hint range ABI

Purpose:

- define a minimal, debatable range-hint ABI
- keep the operation set small: prefetch, promote, demote, release

Files touched:

- `include/uapi/linux/hbf.h`

## 3/4 mm: add minimal hint ingestion skeleton

Purpose:

- add `mm/hbf.c`
- validate ranges
- return `-EOPNOTSUPP` after basic admission checks
- keep TODO markers close to the future MM, CXL, and DAX integration points

Files touched:

- `mm/hbf.c`
- `mm/Kconfig`
- `mm/Makefile`

## 4/4 samples/selftests: add ABI examples

Purpose:

- make the range-hint ABI concrete
- provide one sample and one selftest-shaped consumer

Files touched:

- `samples/hbf/hbfctl-demo.c`
- `tools/testing/selftests/mm/hbf_hint_abi.c`

## Reviewer Questions To Expect

- why not `madvise()`?
- why not put this directly into existing MM policy?
- why is `/dev/hbfctl` needed even as an RFC frontend?
- what evidence shows that CXL/DAX-backed warm tiers benefit from this shape?
- what belongs in `mm/` versus CXL or DAX code later?
