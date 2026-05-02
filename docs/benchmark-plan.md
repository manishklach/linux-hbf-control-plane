# Benchmark Plan

There is no public HBF hardware programming model assumed in this repository. That means performance discussion must be proxy-based and explicit about its limitations.

## Goal

Evaluate whether runtime-guided hints can reduce the gap between "data becomes valuable" and "data is actually in a useful tier before demand."

## Proxy Hardware Setups

Potential stand-ins:

- DRAM as the hot tier
- CXL memory or DAX-backed capacity as a warm tier
- pmem as a slower warm tier where available
- fast NVMe as an intentionally imperfect lower-bound proxy

These are not claims that such devices are HBF. They are only ways to test the control-plane idea under asymmetric bandwidth and latency conditions.

## Workload Proxy

Use trace replay rather than claiming a production model stack:

- capture or synthesize KV-cache access traces
- replay sequence growth and eviction patterns
- annotate upcoming reuse windows
- compare demand-only behavior against advisory prefetch or demotion behavior

## Metrics

Measure:

- hint-to-touch latency
- fraction of touches that still fault after a prefetch hint
- demand fault latency after prior hinting
- promotion or migration cost
- demotion cost
- proxy stall time on CPU or GPU submission paths
- useful prefetch rate versus ignored or wasted hints

## Experimental Comparisons

At minimum compare:

- no hints
- prefetch hints only
- demotion hints only
- combined prefetch plus demotion hints

## Important Caveats

- no result here would prove real HBF hardware behavior
- backend differences may dominate outcomes
- any benchmark must separate hint admission cost from backend data-movement cost
- negative results are still useful if they show that the abstraction does not buy enough
