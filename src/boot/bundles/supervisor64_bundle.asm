section .supervisor_bundle 
global supervisor_bin_start

supervisor_bin_start:
    incbin "build/supervisor64.bin"