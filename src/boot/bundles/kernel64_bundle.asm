section .kernel_bundle 
global kernel_bin_start

kernel_bin_start:
    incbin "build/kernel64.bin"