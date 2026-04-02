open_project hls_fft_project
set_top fft_top
add_files fft_hls.cpp
add_files fft_hls_core.cpp
add_files -tb fft_hls_tb.cpp

open_solution "solution4_sdf"
set_part {xc7z020clg400-1}
create_clock -period 10 -name default

puts "----------------------------------------"
puts "Running C Simulation..."
puts "----------------------------------------"
csim_design

puts "----------------------------------------"
puts "Running C Synthesis..."
puts "----------------------------------------"
csynth_design

puts "----------------------------------------"
puts "Running C/RTL Co-Simulation..."
puts "----------------------------------------"
cosim_design

puts "----------------------------------------"
puts "Exporting RTL IP..."
puts "----------------------------------------"
export_design -format ip_catalog -rtl verilog

exit
