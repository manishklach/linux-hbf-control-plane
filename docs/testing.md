# Testing

## Local Validation

Run:

```bash
./scripts/check-patch.sh
```

The script should:

- verify that `rfc-hbf-linux-control-plane.patch` exists
- compile `examples/hbfctl-demo.c`
- run `scripts/checkpatch.pl --strict` when `KERNEL_TREE` is set
- print instructions when `KERNEL_TREE` is absent

The absence of `KERNEL_TREE` should not be treated as a failure. C compile failures should.

## Kernel Workflow References

Before any manual RFC preparation in a real kernel tree, review:

- Linux patch submission guide:
  [https://kernel.org/doc/html/next/process/submitting-patches.html](https://kernel.org/doc/html/next/process/submitting-patches.html)
- Linux patch submission checklist:
  [https://www.kernel.org/doc/html/latest/process/submit-checklist.html](https://www.kernel.org/doc/html/latest/process/submit-checklist.html)

And run:

- `scripts/checkpatch.pl`
- `sparse`

## Patch Style

If a Linux source tree is available:

```bash
export KERNEL_TREE=/path/to/linux
$KERNEL_TREE/scripts/checkpatch.pl --strict rfc-hbf-linux-control-plane.patch
```

## Static Checking

Within a suitable Linux tree, prefer checks such as:

```bash
make C=1 CF="-D__CHECK_ENDIAN__" M=path/to/rfc/code
```

The point is not to declare the prototype ready. The point is to keep the mechanics honest while the design is still under debate.

## Userspace Example Checks

Compile the demo:

```bash
gcc -Wall -Wextra -O2 -o examples/hbfctl-demo examples/hbfctl-demo.c
```

Run examples:

```bash
./examples/hbfctl-demo --prefetch 0x100000 4096
./examples/hbfctl-demo --promote 0x200000 8192
./examples/hbfctl-demo --demote 0x300000 4096
./examples/hbfctl-demo --pin 0x400000 4096
./examples/hbfctl-demo --release 0x500000 4096
```

Expected behavior without a prototype kernel implementation:

- friendly message that `/dev/hbfctl` is absent
- no claim that the interface exists upstream

## Future Validation

If a real backend or credible proxy backend appears later, useful tests include:

- CXL or DAX-backed capacity exposure
- page migration latency across tiers
- trace replay of KV-cache access patterns
- hint acceptance versus actual prefetch completion
- fault-after-hint timing
- accounting behavior under memory pressure
