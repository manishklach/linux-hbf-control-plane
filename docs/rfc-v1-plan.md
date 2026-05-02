# RFC v1 Plan

Working series title:

`[RFC PATCH 0/4] hbf: explore runtime-guided control plane for high-capacity AI memory tiers`

This plan intentionally turns the current prototype into a smaller, cleaner, more reviewable RFC. The goal is to separate problem statement, UAPI sketch, minimal code skeleton, and sample usage.

## 0/4 Cover Letter Goals

The cover letter should:

- avoid claiming public HBF standardization
- avoid implying that upstream Linux already has an HBF subsystem
- position the work near CXL, DAX, memory hotplug, migration, and tiering
- ask whether a new ABI is warranted at all
- be explicit that the code is for abstraction review, not hardware enablement

## 1/4 Documentation: add HBF control-plane design note

Purpose:

- define the problem in Linux terms before adding code
- explain why a warm context tier is not just block I/O
- explain why hints are advisory
- explain why the proposal is experimental and may never become a standalone subsystem

Files touched:

- `Documentation/` design note in a kernel tree
- optional references to existing CXL, DAX, and MM docs

Why it belongs in this patch:

- reviewers need the rationale before they review the ABI
- subsystem placement arguments should be visible before code structure locks in assumptions

Risks:

- reviewers may still say the idea is too speculative
- documentation may invite "prove it with userspace first" feedback

Reviewer questions:

- why is a new kernel abstraction needed at all?
- why is this not just an MM policy extension?
- what existing hardware model is this abstracting?

## 2/4 uapi: add experimental HBF hint ABI definitions

Purpose:

- provide a narrow, obviously experimental ABI sketch for discussion
- define a minimal hint object and operation set
- keep the ABI small enough to revise or even delete if review goes badly

Files touched:

- `include/uapi/linux/hbf.h`

Why it belongs in this patch:

- reviewers can debate the ABI separately from backend mechanics
- keeping UAPI isolated clarifies that the series is still debating the control surface

Risks:

- UAPI is the most sensitive part of the series
- reviewers may reject the idea of a new ioctl family entirely
- AI-specific semantics may still be seen as too workload-specific

Reviewer questions:

- why not `madvise()`?
- why does the ABI mention AI object classes?
- what guarantees, if any, does a hint imply?

## 3/4 hbf: add minimal miscdevice skeleton for hint ingestion

Purpose:

- provide a minimal control endpoint for local experimentation
- validate struct copy, version checks, and basic admission flow
- keep policy intentionally absent or trivial

Files touched:

- `drivers/memory/hbf/Kconfig`
- `drivers/memory/hbf/Makefile`
- `drivers/memory/hbf/core.c`
- possibly `MAINTAINERS`

Why it belongs in this patch:

- it demonstrates how the UAPI would be ingested without forcing backend commitments
- it allows local testing without pretending real hardware exists

Risks:

- reviewers may still object to the existence of a new miscdevice
- adding a new directory may look like subsystem capture before consensus
- too much scaffolding may distract from the problem statement

Reviewer questions:

- why a char device?
- why not stay entirely in userspace until a backend exists?
- what keeps this from becoming a dumping ground for vendor policy?

## 4/4 samples: add hbfctl userspace hint example

Purpose:

- show intended userspace interaction
- demonstrate that the ABI is at least mechanically usable
- make the series easier to evaluate without inventing real hardware

Files touched:

- `samples/` or `tools/testing/selftests/` depending on reviewer preference
- example source file and minimal build integration

Why it belongs in this patch:

- samples help reviewers understand the control surface
- sample code can evolve independently from the core discussion

Risks:

- samples may be seen as validating an ABI that is still under debate
- selftests may be preferred over samples if the interface is expected to be kernel-testable

Reviewer questions:

- should this live in `samples/` or `tools/testing/selftests/`?
- what constitutes a meaningful test if no backend exists?
- does the sample reveal ABI awkwardness that the kernel side hides?

## v1 Series Design Rules

- no claim of public HBF hardware binding
- no claim that a standalone HBF subsystem is the final answer
- no mail submission tooling in this repository
- no benchmark claims without a proxy methodology
- no requirement that hints be correctness-critical

## Likely v1 Deletions Relative To The Prototype

- likely no standalone sysfs class yet
- likely no rich device registration API yet
- likely fewer AI-specific enums in UAPI
- likely more documentation and less code
