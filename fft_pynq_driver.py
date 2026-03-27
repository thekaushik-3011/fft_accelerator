import pynq
from pynq import Overlay
import numpy as np
import matplotlib.pyplot as plt

# 1. Load the Hardware Overlay
print("Loading Overlay...")
overlay = Overlay("fft_accelerator.bit")

# The names below must match your Block Design names exactly!
dma = overlay.axi_dma_0
fft_ip = overlay.fft_top_0

# 2. Allocate Physical Memory Buffers for the DMA
# Data type is complex floating point, but remember our hardware expects fixed-point Q16.16!
# Since AXI-Stream expects raw 32-bit words, we will pack the real and imaginary parts.
# However, PYNQ handles basic casting if configured correctly, but it's safest to send raw uint32 
# or use a struct dtype equivalent to 16-bit real, 16-bit imag.
# For simplicity, if we defined ap_fixed<32,16> for real and imag, our struct axis_t is 64-bits wide!
# Let's allocate contiguous memory.
in_buffer = pynq.allocate(shape=(1024,), dtype=np.complex64)
out_buffer = pynq.allocate(shape=(1024,), dtype=np.complex64)

# 3. Generate the Test Signal (Two sine waves at bin 10 and 50)
print("Generating test signal...")
fs = 1024
t = np.arange(1024)
# Bin 10 and Bin 50
signal = np.sin(2 * np.pi * 10 * t / fs) + 0.5 * np.sin(2 * np.pi * 50 * t / fs)

# Populate the input buffer
for i in range(1024):
    in_buffer[i] = signal[i] + 0j

# 4. Trigger the Hardware FFT
print("Starting Hardware FFT...")
# Start the DMA transfer
dma.sendchannel.transfer(in_buffer)
dma.recvchannel.transfer(out_buffer)

# Start the FFT IP via AXI-Lite
# (Usually not strictly required if auto-restart is on, but strictly safe)
fft_ip.write(0x00, 0x01) # Write 1 to AP_CTRL to start

# Wait for transfers to complete
dma.sendchannel.wait()
dma.recvchannel.wait()
print("FFT Complete!")

# 5. Extract and Plot Results
hw_out = np.array(out_buffer)
magnitudes = np.abs(hw_out)

plt.figure(figsize=(10, 5))
plt.plot(magnitudes[:100]) # Plot first 100 bins to see bin 10 and 50
plt.title("Hardware FFT Output (First 100 Bins)")
plt.xlabel("Frequency Bin")
plt.ylabel("Magnitude")
plt.grid(True)
plt.show()

# Clean up memory buffers
in_buffer.freebuffer()
out_buffer.freebuffer()
