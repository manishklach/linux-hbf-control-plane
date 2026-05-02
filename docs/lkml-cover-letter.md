From: RFC Author <rfc@example.com>
To: linux-cxl@vger.kernel.org, linux-mm@kvack.org, linux-kernel@vger.kernel.org
Cc: dax@lists.linux.dev
Subject: [RFC PATCH 0/3] hbf: introduce experimental High Bandwidth Flash control plane

Hi,

This series is an RFC for discussion of a possible Linux control-plane abstraction for future High Bandwidth Flash (HBF) or HBF-like high-capacity memory tiers used by AI inference workloads.

Motivation
==========

Large inference workloads can create warm context and KV-cache working sets that do not fit efficiently in accelerator-local HBM, but also should not be treated as purely cold storage. Existing kernel mechanisms for memory tiering, page migration, DAX mappings, and CXL-attached memory already provide important building blocks, but they primarily operate at the page and device-management level.

Inference runtimes often have higher-level knowledge than the kernel about which ranges are likely to become hot soon, which ranges are cooling, and which context windows should be staged before a processor or accelerator demands them. This RFC explores whether a minimal hint interface could provide a useful runtime-to-kernel boundary without making premature assumptions about hardware specifics.

This is intentionally early and incomplete.

Honesty about Scope
===================

No public HBF hardware programming model is assumed here.

This patch intentionally avoids binding to vendor-specific hardware behavior, media details, or transport specifics. The goal is to discuss the kernel abstraction and the shape of a possible control plane, not to claim existing upstream support for HBF hardware.

Proposed Patch Series
=====================

1/3 Documentation: add HBF control-plane design note
2/3 hbf: add experimental uapi header and char device skeleton
3/3 samples: add hbfctl hint example

The implementation is intentionally narrow. It is meant to make the discussion concrete enough to review without forcing a conclusion on where this should ultimately live.

Discussion Goals
================

The main question is whether a small placement/prefetch hint interface is a useful kernel abstraction for AI memory hierarchies built around future CXL-era or otherwise heterogeneous high-capacity memory devices.

Open Questions
==============

- Should this live under CXL, DAX, MM, or a new driver class?
- Should hints be ioctls, prctl/madvise extensions, netlink, or cgroup policy?
- Should placement operate on virtual ranges, file offsets, dma-buf handles, or device memory objects?
- How should security/accounting/cgroup limits work?
- How should accelerators consume staged data?

Feedback on whether this belongs anywhere in the kernel at all is welcome. If the abstraction is wrong, it would be useful to understand whether the right path is a memory-tiering extension, a DAX/CXL interface, a userspace-only experiment, or something else entirely.

Thanks for reading.
