# Tech Stack

## Core Tools
- **Vitis HLS / Vivado HLS**: Main synthesis tool for converting C/C++ to RTL.
- **C/C++ Compiler (GCC/Clang)**: For compiling and testing the software gold model.
- **Vivado**: For RTL simulation, synthesis, and implementation.

## Languages
- **C++**: Primary language for the algorithmic implementation. Needs to support fixed-point data types (`ap_fixed.h`).
- **Verilog / VHDL**: Generated RTL languages. We will interact with these during RTL co-simulation.
- **Python / Tcl**: For automating Vivado/Vitis workflows, collecting trade-off data, and plotting the Area vs. Latency Pareto analysis.

## Hardware Target (Reference)
- **Target FPGA**: e.g., Xilinx Zynq-7000 (Pynq-Z2) or UltraScale+ (To be verified depending on explicit constraints, standard choice is usually suitable for DSP).