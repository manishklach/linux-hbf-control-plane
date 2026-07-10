# QEMU Boot Test Results — v0.2

## Environment

- Host: WSL2 Ubuntu 24.04 on Windows 11
- Kernel: 7.2.0-rc2 (torvalds/linux.git) + HBF 7-patch series
- QEMU: qemu-system-x86_64, 512MB RAM, 1 NUMA node
- Test: `tests/qemu-test-init.c` as /init (static binary, mounts devtmpfs)

## Output

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

## Build Verification

- `CONFIG_HBF_CONTROL_PLANE=y` — kernel builds clean, boots, device node created
- `CONFIG_HBF_CONTROL_PLANE=n` — kernel builds clean (no-regression)
- Each commit in the 7-patch series builds individually

## Known Gaps

- Only 1 NUMA node in QEMU; multi-node promotion/demotion not exercised
- No deadline expiry tested (single-threaded test)
- No cancellation test (requires async submit + concurrent cancel)
