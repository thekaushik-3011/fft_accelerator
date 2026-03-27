# Phase 3: Trade-Off Analysis & Hardware Evaluation

## 1. Area vs. Latency Trade-Off Summary

The following table summarizes the synthesis results across the four evolutionary architectural implementations on the Zynq-7020 (xc7z020clg400-1) at 100 MHz (10ns constraint):

| Implementation | BRAM_18K | DSP48E | Flip-Flops (FF) | LUTs | Key Architectural Choice |
| :--- | :---: | :---: | :---: | :---: | :--- |
| **1. Naïve** | 10 | 8 | 1,787 | 2,191 | Direct C++ translation, purely sequential operations. |
| **2. Pipelined** | 14 | 8 | 1,029 | 2,396 | `#pragma HLS pipeline II=1` forces 1 butterfly/cycle. |
| **3. Partitioned** | 14 | 10 | 987 | 3,126 | `unroll factor=2` + memory banking doubles math parallelism. |
| **4. Dataflow** | 86 | 8 | 8,381 | 9,185 | `#pragma HLS dataflow` with 11 ping-pong buffers for streaming. |

### Pareto Curve Analysis
As we move from Naïve to Dataflow, there is a clear **Area-for-Throughput** tradeoff:
* **LUTs & BRAMs Explode in Dataflow**: Enabling task-level streaming (`#pragma HLS dataflow`) required allocating 11 separate Ping-Pong buffer arrays (`stage_data[][1024]`), scaling BRAM utilization massively from 14 to 86. LUT utilization also spikes to ~9.1K due to managing the complex AXI-Stream I/O overlapping with unrolled execution stages and pipeline control logic.
* **DSP Usage**: DSP usage remained remarkably stable (8 to 10 slices). The synthesizer intelligently reuses multipliers or maps some constants (twiddles) directly to LUT logic, preserving scarce DSP compute blocks.
* **Pipelining is King for Area**: Interestingly, Solution 2 (Pipeline) had fewer Flip-Flops (1,029) than Solution 1 (1,787). Forcing the synthesizer into a strict pipeline schedule eliminated unnecessary multi-cycle state machines, resulting in a cleaner, leaner logic dataplane.

---

## 2. Industry Comparison: FPGA FFT vs. TI C6678 DSP

To validate our Streaming Dataflow FFT accelerator, we benchmark it against the industry-standard **Texas Instruments TMS320C6678** (an 8-core DSP heavily used in telecom and radar).

| Metric | TI C6678 DSP (1.25 GHz) | Our Zynq FPGA Dataflow (100 MHz) | Winner |
| :--- | :--- | :--- | :--- |
| **Execution Paradigm** | Instruction-driven (Software) | Spatial/Streaming Hardware | - |
| **1024-pt FFT Time** | ~10 µs | $\approx$ 14.4 µs (14,448 cycles) | **TI DSP** (Slight edge due to extreme clock rate) |
| **Clock Frequency** | 1,250 MHz | 100 MHz | **TI DSP** |
| **Streaming Throughput** | Batch processing | Continuous AXI-Stream overlap | **FPGA** |
| **Latency Determinism** | Variable (Cache misses, OS jitter) | 100% Deterministic | **FPGA** |
| **Power Consumption** | ~10 Watts | ~1.5 - 2 Watts | **FPGA** (5x more efficient) |

### Conclusion
While the **1.25 GHz TI DSP** processes a single standalone FFT slightly faster purely through raw clock speed (12.5x faster clock), the **100 MHz FPGA Accelerator** dominates in **power efficiency** and **hard-real-time streaming**. Because our Solution 4 utilizes HLS Dataflow, it inherently accepts continuous AXI streaming data without cache interrupts. In high-frequency radar or telecom applications (like 5G physical layers), the FPGA's 100% predictable latency, continuous throughput, and ultra-low power consumption make it the superior industry standard.
