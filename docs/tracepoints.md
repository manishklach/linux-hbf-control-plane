# Tracepoints

If the control plane remains advisory, observability is mandatory.

## Proposed Tracepoints

- `hbf_hint_received`
- `hbf_hint_rejected`
- `hbf_hint_accepted`
- `hbf_prefetch_start`
- `hbf_prefetch_done`
- `hbf_promote_start`
- `hbf_promote_done`
- `hbf_demote_start`
- `hbf_demote_done`
- `hbf_fault_after_hint`

## Why More Than Submit/Complete

The prototype patch currently proposes coarse submission and completion tracepoints. That is not enough to answer:

- whether the kernel rejected the hint up front
- whether the backend accepted but ignored the hint
- whether the prefetch actually completed before demand
- whether a later fault shows the hint arrived too late or had no effect

## Suggested Event Payload Themes

Common fields that may be useful:

- process or cgroup identity
- hint op
- address and length
- target node or tier
- object or correlation tag if retained
- backend id
- acceptance or rejection reason
- timestamps for admission and completion

## Sample `bpftrace` Scripts

Count hints by operation:

```bash
bpftrace -e '
tracepoint:hbf:hbf_hint_received
{
  @[args->op] = count();
}'
```

Measure latency between accepted hint and later fault:

```bash
bpftrace -e '
tracepoint:hbf:hbf_hint_accepted
{
  @ts[args->object_id] = nsecs;
}

tracepoint:hbf:hbf_fault_after_hint
/@ts[args->object_id]/
{
  @lat_us = hist((nsecs - @ts[args->object_id]) / 1000);
  delete(@ts[args->object_id]);
}'
```

Detect ignored hints:

```bash
bpftrace -e '
tracepoint:hbf:hbf_hint_received
{
  @seen[args->object_id] = nsecs;
}

tracepoint:hbf:hbf_prefetch_done,
tracepoint:hbf:hbf_promote_done,
tracepoint:hbf:hbf_demote_done
/@seen[args->object_id]/
{
  delete(@seen[args->object_id]);
}

interval:s:10
{
  print(@seen);
}'
```

These scripts are illustrative only. The exact fields depend on the final tracepoint schema.
