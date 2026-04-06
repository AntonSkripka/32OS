# 32OS: The System of Absolute Order
32OS is a specialized 64-bit operating system built on the philosophy of Static Determinism. Unlike traditional operating systems that manage resources through "dynamic chaos" (heap fragmentation, unpredictable scheduling, and shared memory risks), 32OS enforces a strict, immutable hierarchy of execution.

> [!IMPORTANT]
> 32OS is in its embryonic stage of development. Most of the architectural concepts described below are currently in the design and specification phase and have not yet been fully implemented in code.

## The Architectural Philosophy
The core mission of 32OS is to eliminate the unpredictability of software behavior by hardware-level isolation and a rigid memory model.

1. The Supervisor (The Invisible Judge)
The Supervisor is a high-privilege layer (Ring 0) residing beneath the Kernel. It does not manage files or users; it manages Regions. It operates the Region-Array—a global, protected table containing the "passports" of every piece of code in the system.

Physical Binding: Every program is assigned a fixed memory range and disk sector at registration.

Hardware Enforcement: If a program attempts to access a single byte outside its designated Region, the Supervisor issues a hardware-level block. The program doesn't just "crash"—it is deactivated by the law of the system.

2. The Structural Unit: The Region
In 32OS, there are no "programs" in the classical sense. There are only Regions. A Region is a self-contained vault consisting of:

Tooling (Code): Essential functions.

State (Database): Private, encrypted storage for the program’s data. No other Region can read this memory directly.

Mailbox (Inbox/Outbox): The only legal interface for communication.

3. Communication: Program Messenger
32OS completely abandons the concept of Shared Memory to prevent data corruption and race conditions.

Function Snapshots: If Region A requires a service from Region B, the Supervisor creates a virtual "Snapshot" (clone) of B’s functions and maps it into A’s address space. The original code remains untouched.

Transient Databases: Data exchange occurs through temporary, short-lived memory spaces created by the Supervisor, which are wiped immediately after the transaction is complete.

One-Time Keys: Access to system resources (like the Task Manager) is granted via dynamic, single-use keys. If intercepted, the key becomes instantly void.

## Current Progress (v0.0.3)
[CHANGELOG](CHANGELOG.md)

## Getting Started
Prerequisites
You will need an x86_64-elf cross-compiler toolchain.

Compiler: `x86_64-elf-gcc`

Assembler: `nasm`

Emulator: `qemu-system-x86_64`

## Build & Run
```Bash
# Clone the repository
git clone https://github.com/AntonSkripka/32OS.git
cd 32OS

# Compile the OS image
make all

# Launch in QEMU
make run
```
## Security Modes
Operational Mode: Full hierarchical protection under the Supervisor's watch.

Terminal Zero (Safe Mode): A "Pure Logic" state. The Kernel is not loaded. Only the Supervisor and the Region-Array are active. In this mode, the user has direct access to the program table and can delete malicious or failing code like a row in a database.
