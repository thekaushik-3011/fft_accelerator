# Product Requirements Document (PRD)

## 1. Project Title
Design and Optimization of a Streaming 1024-Point Fixed-Point FFT Accelerator for Real-Time DSP Applications Using High-Level Synthesis.

## 2. Objective
Design a streaming 1024-point Radix-2 fixed-point FFT accelerator that processes continuous input samples and produces frequency-domain outputs while minimizing memory conflicts and demonstrating clear area-latency trade-offs. 

## 3. Key Deliverables & Phases

### Phase 1: Software Gold Model (Baseline)
- Develop a floating-point 1024-point FFT in C++.
- Profile the execution time and memory access patterns.
- Convert floating-point to fixed-point (Q-format).
- Conduct SNR analysis to evaluate precision loss.

### Phase 2: HLS Behavioral Synthesis & Optimization
- **Naïve Implementation**: Direct C++ to RTL translation with no pragmas. 
- **Progressive Optimization**: Apply the following directives:
  - `pipeline`: To reduce stage latency (Target II = 1).
  - `unroll`: To increase butterfly parallelism.
  - `array_partition`: For BRAM banking and parallel twiddle/data access.
  - `dataflow`: To overlap FFT stages for high streaming throughput.
  - `interface`: To define AXI-Stream for continuous I/O.

### Phase 3: Trade-Off Analysis
- Implement and compare multiple architectural configurations:
  1. **Baseline**: No optimizations.
  2. **Optimized-1**: Pipelined + Partial Unroll + Partial Partitioning.
  3. **Optimized-2**: Pipelined + Full Unroll + Full Partitioning + Dataflow.
- Extract metrics: Latency (cycles), Initiation Interval (II), LUTs, DSPs, and BRAMs.
- Generate an Area vs. Latency Pareto curve.

### Phase 4: Validation & Export
- Perform C/RTL Co-simulation to verify functional correctness.
- Export synthesizable RTL IP.
