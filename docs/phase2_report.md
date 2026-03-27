# Phase 2 Synthesis Results & Trade-Off Analysis

During Phase 2, we systematically applied HLS Pragmas to the C++ Gold Model to explore architectural trade-offs. 

## 1. Synthesis Data Summary

| Configuration | Pipelined | Partitioned | Dataflow | Interval / Latency | BRAMs | DSPs | FFs | LUTs |
|---|---|---|---|---|---|---|---|---|
| **Solution 1 (Naïve)** | No | No | No | ~ Sequential | 6 | 8 | 1,462 | 1,509 |
| **Solution 2 (Pipeline)** | Yes | No | No | Failed (Negative Slack) | 6 | 8 | 1,222 | 1,482 |
| **Solution 3 (Partition)** | Yes | Partial | No | Failed (Cyclic collision) | 12 | 8 | 1,206 | 2,191 |
| **Solution 4 (Dataflow)** | Yes | No | Yes | **14,448 cycles** | **86** | 8 | 8,348 | 9,173 |

## 2. Architectural Findings

1. **The BRAM Port Bottleneck**: As observed in Solutions 2 and 3, attempting to simply pipeline the butterfly operations failed. Standard BRAM block RAM on Xilinx FPGAs only has 2 memory ports. A Radix-2 butterfly requires 2 reads and 2 writes simultaneously per clock to achieve II=1. 
2. **The Stride Collision**: In Solution 3, we attempted to use `#pragma HLS array_partition type=cyclic factor=2`. This failed because the memory access stride in the Cooley-Tukey FFT algorithm doubles at each stage ($1, 2, 4, 8...$). By Stage 2, the stride is an even number, meaning the hardware attempts to read two even indices simultaneously resulting in contention on Bank 0.
3. **The Dataflow Resolution**: In Solution 4, we abandoned trying to pipeline a single array in place. Instead, we used `#pragma HLS dataflow` and unrolled the outer `STAGE_LOOP`. This instantiated **10 physically separate hardware stages** connected by continuous Ping-Pong buffers (`stage_data`).

## 3. The Area vs. Latency Conclusion
Solution 4 achieved a highly deterministic, streaming Initiated Interval of exactly **14,448 cycles** for the entire 1024-point dataset (which equates to an immense data throughput capability). 

However, looking at the resource utilization, the **BRAM count exploded from 6 to 86**, and the **LUT count increased from 1,500 to over 9,000**. This perfectly demonstrates the classic hardware paradigm: **To achieve maximum streaming speed, we must sacrifice silicon area.**
