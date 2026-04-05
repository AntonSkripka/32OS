## V. 0.0.1 - 2026-03-17
The system currently boots into a 64-bit environment with a clear separation between the Supervisor and the Kernel.
Multi-boot Compliance: Compatible with GRUB.
Long Mode Initialization: Successful jump to 64-bit execution.
Supervisor Layer: Active at the 2MB mark with its own dedicated IDT (Interrupt Descriptor Table).
Kernel Layer: Operates at the 8MB mark, communicating through a protected VGA driver.
Hardware Protection: Initial GDT/IDT setup for 64-bit fault handling.

## V. 0.0.1-1 - 2026-03-18
Added interrupt `int 0x32` (test)

## V. 0.0.2 - 2026-03-31
Added
APIC/LAPIC Support. 
APIC Timer Calibration: Implemented timer calibration using the Legacy PIC, achieving a precision of 1ms. 
RTC Integration: Added Real-Time Clock support to retrieve and track system wall clock time. 
System Timekeeping: Implemented a global uptime counter that increments every second, driven by every 1000th APIC interrupt.

## V. 0.0.2-1 - 2026-04-02
Added
Created CHANGELOG.md to track version history. 

Fixed
Code Refactoring: Eliminated redundant and duplicated code across the kernel source to improve maintainability and reduce binary size.

## V. 0.0.2-2 - 2026-04-05
### Added
- **Higher Half Kernel Mapping:** The kernel is now fully mapped to the virtual address space starting at `0xFFFFFFFF80000000`.
- **Per-Region Paging:** Implemented unique PML4 structures for different memory regions to isolate kernel and user spaces.
- **Strict MMIO Policies:** Explicitly applied `PAGE_PCD` (Cache Disable) and `PAGE_PWT` (Write-Through) flags to APIC memory regions.

### Changed
- **Bootstrapping:** Updated the 64-bit transition logic (trampoline) to perform a long jump into the higher half virtual address space.
- **Stack Relocation:** Moved the initial kernel stack to high memory addresses to prevent Page Faults during the transition.
- **APIC Mapping:** Relocated Local APIC registers to the kernel's device memory range.