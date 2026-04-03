# End-to-End FPGA Hardware Acceleration of a 1024-Point FFT
**Comprehensive Technical Report on Hardware/Software Co-Design and HLS Optimization**

## 1. Executive Summary
This project demonstrates the complete engineering lifecycle of transforming a mathematical algorithm (Fast Fourier Transform) from a standard C++ software model into a deeply optimized, high-throughput hardware accelerator deployed on a Xilinx Zynq-7020 (PYNQ-Z2) FPGA. 

By strategically utilizing Vitis High-Level Synthesis (HLS), we systematically resolved physical hardware constraints (like BRAM port starvation and stride collisions) to build a multi-stage **Single-path Delay Feedback (SDF) Dataflow** processing engine. The final IP achieves continuous real-time streaming throughput by processing blocks of 1024 complex numbers in exactly 3,085 clock cycles utilizing an AXI4-Stream interface.

---

## 2. Phase 1: The Software Gold Model & Fixed-Point Conversion
Hardware architectures built with pure floating-point math (`float` or `double`) consume massive amounts of precious DSP slices and incur severe timing penalties. Our first step was evaluating precision loss when moving to hardware-friendly fixed-point logic.

### Optimization 1: Fixed-Point Arithmetic
We replaced all standard C++ `float` types with Xilinx's Arbitrary Precision datatype: `ap_fixed<32, 16>`. This designates 16 bits for the integer part and 16 bits for the fractional part (Q16.16 format).

**The Result:** 
We developed two automated C++ testbenches and ran them live to validate correctness.

#### Testbench 1: Floating-Point Reference (`fft_tb.cpp`)
```
--- Floating-Point FFT ---
Execution Time : 312.582 µs   (pure sequential software on host CPU)
Bin 10 Magnitude : 512        ✅ (Expected ~512)
Bin 50 Magnitude : 256        ✅ (Expected ~256)
```
The two-tone sine wave (10 Hz and 50 Hz) was perfectly identified. This establishes our **golden reference**.

#### Testbench 2: Fixed-Point SNR Analysis (`fft_fixed_tb.cpp`)
```
--- Fixed-Point Q32.16 vs Floating-Point SNR ---
Signal Power : 655,360
Noise Power  : 0.00114596
SNR (dB)     : 87.5731 dB    ✅
```
- **Outcome:** An SNR of **87.57 dB** proves the precision loss from `ap_fixed<32,16>` quantization is completely negligible (essentially lossless for all DSP applications). The design was verified as hardware-ready.

#### Key Software vs Hardware Latency Comparison
| Mode | Execution Time | 
|:---|:---:|
| Software (floating-point C++ on CPU) | **312.58 µs** |
| Hardware (SDF FFT on FPGA @ 100 MHz) | **30.85 µs** |
| **Speedup** | **~10×** |

---

## 3. Phase 2: High-Level Synthesis (HLS) Architectural Space Exploration
The core of this project revolved around the **Area vs. Latency** trade-off. We wrote multiple variations of the FFT core and commanded Vitis HLS to synthesize them into hardware to systematically defeat physical bottlenecks.

### Architecture 1: The Naïve Approach
We synthesized the C++ code exactly as written, with no optimization limits.
- **Why we did it:** To get a baseline of how the compiler naturally maps loops to logic gates.
- **The Outcome:** The hardware simply instantiated a single multiplier and reused it sequentially. While it used very little hardware (Area), it was incredibly slow (High Latency) because it took thousands of clock cycles to step through the triple-nested FFT loops one by one.

### Architecture 2: The Pipelined Approach (`#pragma HLS pipeline`)
We attempted to force the inner butterfly loop to process one iteration every single clock cycle (Initiation Interval, II = 1).
- **The Bottleneck (BRAM Port Starvation):** A standard Radix-2 butterfly operation requires reading 2 inputs and writing 2 outputs in a single cycle (4 memory operations). However, physical Block RAM (BRAM) on an FPGA only has **2 physical ports**. 
- **The Outcome:** Synthesis **FAILED** to meet II=1 because you cannot physically perform 4 operations on a 2-port memory in one clock cycle. 

### Architecture 3: The Partitioned Approach (`#pragma HLS array_partition`)
To solve the BRAM port starvation, we told the compiler to split the 1024-element memory array in half (cyclic partitioning) so we could read from two separate physical RAM blocks simultaneously.
- **The Bottleneck (Stride Collisions):** The FFT algorithm is unique; the distance (stride) between the two numbers it needs to process doubles in every stage (stride=1, then 2, then 4...). As the algorithm runs, it eventually tries to read two numbers that just happen to reside in the exact same physical memory bank.
- **The Outcome:** Synthesis **FAILED** again. The memory bank collisions forced the compiler to halt the pipeline to avoid data corruption.

### Architecture 4: The Final SDF Dataflow Engine (The Holy Grail)
Because standard array-passing always resulted in memory bottlenecks, we completely ripped out the array loops and implemented a true **Single-path Delay Feedback (SDF) streaming pipeline**.
- **The Solution:** We instantiated 10 physically independent hardware stages connected purely by AXI `hls::stream`. Inside each stage, we created a static internal **Shift Register**. Stage 1 delays data by 512 elements, Stage 2 by 256, all the way down to Stage 10 delaying by 1. Because each stage perfectly consumes and produces data stream tokens inline with its calculation, the total pipeline memory requirement plummets.
- **HLS Optimizations:** We utilized `#pragma HLS bind_storage` to perfectly map the shift registers to Block RAM, and crucially utilized `#pragma HLS dependence variable=sr type=inter false` to instruct Vitis that our read and write addresses would never collide. This completely resolved the false loop-carried dependency block and gave us 100MHz green timing closure!
- **The Outcome:** **SUCCESS**. Because the stages never share the same memory, there are zero bank collisions. All 10 stages run in parallel on an assembly line. 

### Synthesis Results Comparison Table
| Solution | Pragmas Applied | Interval | BRAMs | DSPs | FFs | LUTs |
|---|---|---|---|---|---|---|
| **Naïve** | None | Sequential | 6 | 8 | 1,462 | 1,509 |
| **Pipeline** | `pipeline II=1` | Failed | 6 | 8 | 1,222 | 1,482 |
| **Partition** | `partition` | Failed | 12 | 8 | 1,206 | 2,191 |
| **SDF Dataflow** | `dataflow` + `dependence false` | **3,085 cycles** | **32** | 72 | **15,129** | **9,725** |

> [!TIP]
> **Why we chose SDF:** Standard Dataflow utilizing Ping-Pong Arrays typically balloons to over 86 BRAMs. By engineering a custom SDF pipeline with shift registers, we kept the exact same ultra-fast continuous streaming capabilities (3,085 Interval!) while dropping memory utilization structurally by 62%. It handles endless block streaming gracefully.

---

## 4. Phase 3: Vivado IP Integration & AXI Interfaces
To deploy the IP on physical silicon, it must be able to communicate with the rest of the world. We utilized the industry-standard **AXI4-Stream** protocol.

### Resolving AXI Protocol Strictness in Vivado 2025.1
Modern AMBA AXI standards require specific hardware signals (Side-Channels) to dictate when a stream of data ends, specifically the `TLAST` wire. 
- **The Challenge:** Vitis HLS will sometimes incorrectly pad custom C++ structs, misaligning the wire widths (e.g., generating 72-bit widths instead of 64-bit), completely halting bitstream generation.
- **The Optimization:** We refactored our top-level C++ interface to utilize the native Xilinx library `ap_axiu<64,0,0,0>`. This explicitly forced the compiler to format the 64-bit payload (32-bit Real + 32-bit Imaginary) into perfectly aligned hardware wires with an instantly recognizable `TLAST` signal.
- **The Outcome:** Vivado successfully passed IP Validation and generated the `design_1_wrapper.bit` SoC bitstream.

---

## 5. Phase 4: PYNQ-Z2 Deployment (Software Driver)
The final step was commanding the hardware from the Zynq's ARM processor using Python. 

Because we designed a continuous SDF streaming hardware architecture, typical software lists would be too slow. We utilized a **Direct Memory Access (DMA)** engine.
1. The Python script reserves a block of continuous physical DDR RAM using `pynq.allocate`.
2. A mathematical test signal (sine wave) is written to that physical RAM.
3. The ARM processor sends a start command to the DMA and goes to sleep.
4. The DMA hardware takes outright control of the memory bus, reads the 1024 samples, perfectly streams them into the FPGA FFT logic over the physical AXI bus, catches the answers, and writes them back to RAM.
5. Python wakes up and plots the highly-accelerated hardware output.

## 6. Conclusion
This project successfully bridged the gap between theoretical software algorithms and physical hardware manipulation. By proving why pipelining fails on standardized memory, identifying the superiority of SDF Dataflow architectures for stream processing, and successfully orchestrating a DMA-backed PYNQ deployment, we achieved a deterministic, real-time hardware accelerated Signal Processing system.
