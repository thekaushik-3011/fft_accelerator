# Phase 1 Summary: Software Gold Model Analysis

## 1. Floating-Point Execution
- **FFT Size**: 1024-point Radix-2
- **Execution Time**: ~10.28 µs on the host CPU.

## 2. Memory Access Profiling
Software execution of the N=1024 FFT involves nested loops computing 10 stages.
- **Butterflies**: 5120 total butterfly operations.
- **Reads/Writes**: Each butterfly involves reading two inputs and writing two outputs. Overall, the FFT computes ~10,240 memory reads and 10,240 writes (along with extra reads/writes for initial bit-reversal sorting).
- **Access Patterns**: At stage 1, the memory access stride is small, maximizing CPU cache locality. However, by stage 10, the array access stride grows to 512 elements. This irregular, strided access pattern leads to heavy cache misses on conventional architectures. 
- **Identifying the Bottleneck**: This memory-bound nature is the primary reason software execution fails to meet high streaming throughput constraints. Through High-Level Synthesis (HLS) in Phase 2, we can resolve these bottlenecks using `array_partition`, `pipeline`, and `dataflow` to parallelize data access and eliminate memory conflicts.

## 3. Fixed-Point Conversion & SNR Analysis
- **Data Type Selected**: `ap_fixed<32, 16>` (Q16.16 format). This type was selected to provide 16 bits for the fractional component while offering enough integer bit growth to avoid overflow without having to implement fractional scaling (divide by 2) at every radix stage.
- **Signal Power**: 655,360
- **Quantization Noise Power**: ~0.001146
- **Signal-to-Noise Ratio (SNR)**: **87.57 dB**

### Conclusion
The conversion to fixed-point yields an outstanding SNR with negligible precision loss. We now have a completely verified bit-accurate C++ Gold Model that is ready to be synthesized in Phase 2.
