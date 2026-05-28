<p align="center">
  <img src="architecture.png" alt="C6531 Core Instruction Layout" width="1200" height="600">
</p>

# C6531 Architecture

**C6531** is an experimental, non-power-of-two computer architecture designed for low-level research. It utilizes **65-bit machine words** and features a hardware-enforced **31-bit backward compatibility mode**.

This repository contains a functional hardware emulator cross-compiled for multiple systems, a low-level assembler written in Python, and updated execution logic supporting hardware loops, branching, and a cross-platform data layout.

---

## Technical Specifications

The C6531 core breaks away from traditional 32-bit and 64-bit boundaries to explore non-standard bit alignment.

### 1. Register File
* **32 General-Purpose Registers** (`R0` to `R31`), each 65 bits wide.
* **Program Counter (PC)**: 64-bit instruction pointer.

### 2. Instruction Format (65-bit fixed-length)
Every instruction is exactly 65 bits wide, laid out across the following bit fields:
* **Bits 64–57 (8 bits)**: Opcode (Allows up to 256 instructions)
* **Bits 56–52 (5 bits)**: Destination Register (`Reg Dst`)
* **Bits 51–47 (5 bits)**: Source Register 1 (`Reg Src1`)
* **Bits 46–42 (5 bits)**: Source Register 2 (`Reg Src2`)
* **Bits 41–0 (42 bits)**: Immediate Value / Direct Memory Address

### 3. Custom 65-bit Floating-Point Format (c65_float)
The integrated Floating-Point Unit (FPU) bypasses standard IEEE 754 constraints to offer an alternative dynamic range for precise numerical math:
* **1 Sign bit** (Bit 64)
* **12 Exponent bits** (Bits 63–52) with a bias of 2047 (handles values up to approx $10^{614}$)
* **52 Mantissa bits** (Bits 51–0)

---

## Dual-Mode Execution

The core handles execution logic differently depending on the active architectural state, controlled by the `SETMODE` instruction:

* **MODE 65 (Native)**: Full 65-bit data processing. The FPU and integer ALU utilize the complete register width.
* **MODE 31 (Compatibility)**: Emulates a legacy 31-bit system. The hardware automatically applies a strict bitmask (`0x7FFFFFFF`) to all ALU outputs. Any data overflow past the 31st bit is discarded immediately.

---

## Instruction Set Architecture (ISA)

| Opcode | Mnemonic | Arguments | Description |
| :--- | :--- | :--- | :--- |
| 0x01 | **ADD** | R_dst, R_src1, R_src2 | Integer addition |
| 0x02 | **SETMODE** | Immediate | Hardware mode switch (31 or 65) |
| 0x03 | **LOAD** | R_dst, Address | Loads a 65-bit word from RAM into a register |
| 0x04 | **STORE** | R_src1, Address | Stores a 65-bit word from a register into RAM |
| 0x05 | **FADD** | R_dst, R_src1, R_src2 | Floating-point addition via custom FPU |
| 0x06 | **JMP** | Address | Unconditional jump to direct memory address |
| 0x07 | **JZ** | R_src1, Address | Jump to direct memory address if R_src1 equals 0 |

---

## Project Structure

### Directory Layout
* `/emulator` — Modular C++ CPU simulation and memory controller source files. Fully compatible with GCC (Linux) and MSVC (Windows).
* `/assembler` — Python compiler toolchain.
* `test_program.asm` — Sample assembly source file using conditional branching logic.

---

## Compilation from Source

The emulator utilizes a platform-agnostic `uint65_t` architecture built on native standard integers, eliminating dependencies on GCC-specific compiler extensions such as `__int128`. It allocates system memory dynamically on the heap to avoid stack overflows.

### Windows (via GCC / MinGW):
```bash
g++ emulator/main.cpp emulator/cpu.cpp -o emulator.exe -O3
```

### Windows (via Visual Studio Developer Command Prompt / MSVC):
```bash
cl /EHsc /O2 emulator/main.cpp emulator/cpu.cpp /Fe:emulator.exe
```

### Linux:
```bash
g++ emulator/main.cpp emulator/cpu.cpp -o emulator -O3
```

---

## Quick Start Guide (How to Run)

The emulator functions as a CLI tool. Since it prints the machine state and exits instantly, do not execute it via double-click. Follow these steps to run a program:

### 1. Compile Assembly Code
Open your terminal in the project root folder and compile the sample assembly file into a machine-readable hex file using the Python toolchain:
```bash
python assembler.py testprogram.asm program.hex
```

### 2. Run the Emulator
Make sure the compiled emulator binary (`emulator.exe` or `emulator`) and the generated `program.hex` are located in the same directory. Open your command prompt (cmd/terminal) inside that folder and execute:
```bash
emulator.exe
```
*(On Linux: `./emulator`)*

The emulator will process the 65-bit instructions step-by-step and output the final internal state of all registers (R0–R31) directly into your console session.

---

## Sample Program Layout (Branching & Loop)
Contents of `test_program.asm` showing multiplication via conditional branching:

```nasm
SETMODE 65     ; Enable native 65-bit mode
LOAD R1, 5     ; Load value 5 into R1
LOAD R2, 3     ; Load loop counter value 3 into R2
LOAD R3, 0     ; Clear R3 to store the results
JZ R2, 8       ; If R2 matches 0, break loop and jump to STORE
ADD R3, R3, R1 ; R3 = R3 + R1
LOAD R2, 0     ; Clear the counter (simulating manual decrement loop step)
JMP 4          ; Return to the conditional jump check step
STORE R3, 100  ; Save the calculated product into RAM address 100
```
