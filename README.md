# Streaming 1024-Point Fixed-Point FFT Accelerator

Design and optimization of a streaming 1024-point Radix-2 fixed-point FFT accelerator for real-time DSP applications using Xilinx Vitis High-Level Synthesis (HLS).

## Project Overview

This project demonstrates the complete HLS design flow — from a software C++ gold model to a fully optimized streaming hardware accelerator. By systematically applying HLS optimization pragmas (`pipeline`, `array_partition`, `dataflow`, `unroll`), we explore the classic **Area vs. Latency** trade-off in FPGA design.

**Target Device:** Xilinx Zynq-7020 (`xc7z020clg400-1`)  
**Clock Period:** 10 ns (100 MHz)  
**Fixed-Point Format:** `ap_fixed<32, 16>` (Q16.16)

## Repository Structure

```
.
├── README.md
├── Product_Requirements_Document.md
├── Tech_Stack.md
│
├── phase1_baseline/          # Phase 1: Software Gold Model
│   ├── fft.h                 # Floating-point FFT header
│   ├── fft.cpp               # Floating-point FFT implementation
│   ├── fft_tb.cpp            # Floating-point testbench
│   ├── fft_fixed.h           # Fixed-point FFT header (ap_fixed<32,16>)
│   ├── fft_fixed.cpp         # Fixed-point FFT implementation
│   └── fft_fixed_tb.cpp      # Fixed-point testbench with SNR analysis
│
├── phase2_hls/               # Phase 2: HLS Synthesis & Optimization
│   ├── fft_hls.h             # Top-level HLS header (AXI interfaces)
│   ├── fft_hls.cpp           # Top-level HLS wrapper
│   ├── fft_hls_core.cpp      # Optimized FFT core (dataflow architecture)
│   ├── fft_hls_tb.cpp        # HLS C-simulation testbench
│   ├── twiddles.h            # Precomputed fixed-point twiddle factor ROM
│   └── run_hls.tcl           # Vitis HLS automation script
│
└── docs/                     # Reports & Analysis
    ├── phase1_report.md      # Phase 1 profiling & SNR results
    └── phase2_report.md      # Phase 2 synthesis comparison table
```

## Key Results

### Phase 1: Software Gold Model
- Floating-point 1024-point Radix-2 FFT implemented and profiled.
- Converted to fixed-point `ap_fixed<32, 16>` with **SNR of 87.57 dB** (negligible precision loss).
- Identified memory-bound bottleneck: strided access patterns cause heavy cache misses at later FFT stages.

### Phase 2: HLS Synthesis Results

| Solution | Pragmas Applied | Interval | BRAMs | DSPs | FFs | LUTs |
|---|---|---|---|---|---|---|
| **1. Naïve** | None | Sequential | 6 | 8 | 1,462 | 1,509 |
| **2. Pipeline** | `pipeline II=1` | Failed (BRAM conflict) | 6 | 8 | 1,222 | 1,482 |
| **3. Partition** | `pipeline` + `array_partition` | Failed (stride collision) | 12 | 8 | 1,206 | 2,191 |
| **4. Dataflow** | `dataflow` + `unroll` + `pipeline` | **14,448 cycles** | **86** | 8 | 8,348 | 9,173 |

> **Key Insight:** Standard BRAM has only 2 ports, insufficient for a pipelined Radix-2 butterfly. Cyclic partitioning fails because the FFT stride doubles each stage, causing bank collisions. The dataflow architecture resolves this by instantiating 10 independent hardware stages with dedicated ping-pong buffers.

## How to Build & Run

### Phase 1 (Software Simulation)
Requires a C++ compiler with Xilinx HLS headers (`ap_fixed.h`).
```bash
cd phase1_baseline
g++ -o fft_tb fft.cpp fft_tb.cpp -lm
./fft_tb

g++ -o fft_fixed_tb fft_fixed.cpp fft_fixed_tb.cpp -lm -I$XILINX_HLS/include
./fft_fixed_tb
```

### Phase 2 (Vitis HLS Synthesis)
Requires Vitis HLS 2024.2+ installed.
```bash
cd phase2_hls
vitis_hls -f run_hls.tcl
```
Or open the Vitis Unified IDE (`vitis`), create an HLS component, add the source files, and run C-Synthesis interactively.

## Tools & Technologies
- **Vitis HLS 2024.2+** — High-Level Synthesis
- **C++ / `ap_fixed`** — Fixed-point arithmetic
- **Vivado xsim** — C/RTL Co-simulation
- **Zynq-7020 FPGA** — Target platform

## License
This project was developed as part of the Semester 8 HLS coursework at IIT Jodhpur.
