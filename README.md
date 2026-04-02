# Streaming 1024-Point Fixed-Point FFT Accelerator

Design and optimization of a streaming 1024-point Radix-2 fixed-point FFT accelerator for real-time DSP applications using Xilinx Vitis High-Level Synthesis (HLS), culminating in a physical deployment on the PYNQ-Z2 FPGA.

## Project Overview

This project demonstrates the complete hardware-software co-design flow — from a software C++ gold model to a fully optimized streaming hardware accelerator, and finally to physical deployment using PYNQ. By systematically applying HLS optimization pragmas (`pipeline`, `array_partition`, `dataflow`, `unroll`), we explore the classic **Area vs. Latency** trade-off, resolve strict AXI-Stream `TLAST` and `TDATA` protocol requirements, and validate the system at 100 MHz.

**Target Device:** Xilinx Zynq-7020 (`xc7z020clg400-1` / PYNQ-Z2)  
**Clock Period:** 10 ns (100 MHz)  
**Fixed-Point Format:** `ap_fixed<32, 16>` (Q16.16)

## Repository Structure

```text
.
├── README.md
├── Product_Requirements_Document.md
├── Tech_Stack.md
│
├── phase1_baseline/          # Phase 1: Software Gold Model
│   ├── fft.cpp / fft.h
│   └── fft_fixed.cpp / .h    # Fixed-point C++ model
│
├── phase2_hls/               # Phase 2: HLS Synthesis & Optimization
│   ├── fft_hls.h             # Native Xilinx AXI Stream header (ap_axiu)
│   ├── fft_hls.cpp           # Top-level HLS wrapper
│   ├── fft_hls_core_*.cpp    # 4 Architecture Variations (Naive, Pipeline, Partition, Dataflow)
│   ├── fft_hls_tb.cpp        # HLS C-simulation testbench
│   └── run_hls.tcl           # Vitis HLS automation script
│
├── docs/                     # Reports & Analysis
│   ├── phase1_report.md      
│   ├── phase2_report.md      
│   └── Phase3_TradeOff_Analysis.md # Pareto analysis against TI DSP
│
├── FPGA_implementation/      # Phase 4: Vivado Deployment
│   ├── fft_accelerator.bit   # Synthesized Hardware Bitstream
│   └── fft_accelerator.hwh   # Hardware Handoff File
│
└── fft_pynq_driver.py        # Phase 5: PYNQ Jupyter Deployment Script
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
| **4. SDF Dataflow** | `dataflow` + `bind_storage` | **3,085 cycles** | **32** | 72 | 15,129 | 9,725 |

> **Key Insight:** Standard `pipeline` and `array_partition` fail drastically on basic arrays due to FFT memory access stride collisions. We ultimately implemented a true **Single-path Delay Feedback (SDF) continuous pipeline**, dropping BRAM utilization immensely and increasing pipeline throughput down to just 3,085 cycles! Vitis maps the shift registers exactly to RAM, completely bypassing standard memory bottlenecks.

### Phase 3: Architecture Trade-Off Analysis
Plotted a Pareto curve identifying **Dataflow** as the optimal solution for streaming contexts. Compared against the industry-standard **TI C6678 DSP**, the FPGA Dataflow engine achieves comparable latency but guarantees deterministic, continuous AXI-Stream processing with no CPU overhead.

### Phase 4 & 5: Hardware Integration & PYNQ Deployment
- **AXI-Stream Formatting:** Resolved intense Vitis 2025.1 structural limitations by utilizing the native Xilinx `ap_axiu<64,0,0,0>` type to enforce a rigid 64-bit continuous TDATA width and automatically infer the TLAST side-channel without destructive struct padding.
- **PYNQ-Z2 Driver:** Built a robust Python driver using `pynq.allocate` to reserve physical contiguous DDR RAM. The ARM processor delegates FFT computation entirely to the hardware via AXI Direct Memory Access (DMA) transfers, analyzing real-time outputs successfully.

## How to Build & Run

### Phase 5 (Run on Physical PYNQ-Z2 Board)
1. Copy the `FPGA_implementation/` folder and `fft_pynq_driver.py` to your PYNQ Jupyter environment.
2. Run the deployment script. The DMA will handle the data transfer over the physical PL-PS boundary!
```bash
python3 fft_pynq_driver.py
```

### Phase 2 (Vitis HLS Synthesis)
Requires Vitis Unified IDE installed.
```bash
cd phase2_hls
vitis_hls -f run_hls.tcl
```

## Tools & Technologies
- **Vitis Unified IDE (2025.1)** — High-Level Synthesis
- **Vivado 2025.1** — Block Design & IP Integration
- **C++ / `ap_fixed`** — Fixed-point arithmetic
- **PYNQ (Python)** — Physical FPGA deployment
- **Zynq-7020 (PYNQ-Z2)** — Target platform

## License
This project was developed as part of the Semester 8 HLS coursework at IIT Jodhpur.
