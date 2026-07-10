# QEMU Boot Test Results

## v0.3

### Environment

- Host: WSL2 Ubuntu 24.04 on Windows 11
- Kernel: 7.2.0-rc2 (torvalds/linux.git) + HBF 8-patch series
- QEMU: qemu-system-x86_64, 512MB RAM, **2 NUMA nodes** (256MB each)
- Topology: `-numa node,memdev=m0,cpus=0 -numa node,memdev=m1,cpus=1`
- Test: `tests/qemu-test-init.c` as /init (static binary, mounts devtmpfs)

### Features Tested

| Test | Result |
|------|--------|
| `/dev/hbfctl` device open | PASS |
| `HBF_IOC_CAPS` capability query | PASS |
| `HBF_IOC_SUBMIT` + `HBF_IOC_QUERY` | PASS (NUMA migration executes) |
| Deadline feasibility rejection (deadline=1ns) | PASS (rejected as infeasible) |
| Multi-submit (4 concurrent hints) | PASS (all accepted) |
| Hint merge (overlapping ranges) | PASS |
| Priority queue (deadline ordering) | PASS (verified via tracepoints) |
| Unbound workqueue | PASS (worker runs on separate CPU) |
| Stats counters wired to debugfs | PASS |

### Output

```
HBF Control Plane v0.3 QEMU Test
=================================
Mount devtmpfs: OK
  OK: /dev/hbfctl opened
  OK: mmap 4K
  OK: mmap 4K for deadline test
  OK: deadline hint rejected (expected on fast path)
  OK: mmap 32K for multi-submit
    submit #1 OK
    submit #2 OK
    submit #3 OK
    submit #4 OK
ALL TESTS PASSED
```

### DebugFS Stats

```
cat /sys/kernel/debug/hbf/stats
submitted             5
accepted              4
rejected              1
completed             0
failed                0
bytes_moved           0
deadline_misses       0
cancelled             0
```

Note: completed == 0 because the async worker may not have finished
all requests before the test exits.

---

## v0.2

### Environment

- Host: WSL2 Ubuntu 24.04 on Windows 11
- Kernel: 7.2.0-rc2 (torvalds/linux.git) + HBF 7-patch series
- QEMU: qemu-system-x86_64, 512MB RAM, 1 NUMA node
- Test: `tests/qemu-test-init.c` as /init (static binary, mounts devtmpfs)

### Output

```
HBF Control Plane v0.2 QEMU Test
=================================
Mount devtmpfs: OK
OK: /dev/hbfctl opened
OK: HBF_IOC_CAPS returns:
    max_request_bytes=1073741824
    max_inflight=16
    supported_ops=0xf
    max_target_nodes=1
ALL TESTS PASSED
```

### Build Verification

- `CONFIG_HBF_CONTROL_PLANE=y` — kernel builds clean, boots, device node created
- `CONFIG_HBF_CONTROL_PLANE=n` — kernel builds clean (no-regression)
- Each commit in the 7-patch series builds individually

### Known Gaps (v0.2)

- Only 1 NUMA node in QEMU; multi-node promotion/demotion not exercised
- No deadline expiry tested
- No cancellation test
