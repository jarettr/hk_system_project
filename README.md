# Heuristic OS: A Dynamic Polymorphic Execution Framework

## 📌 Overview
**Heuristic OS** is a high-assurance security research platform built on top of the **seL4 Microkernel** and **CAmkES** (Component Architecture for microkernel-based Embedded Systems). It is designed to implement **Moving Target Defense (MTD)** strategies at the binary execution level. 

Standard operating systems are static; once compiled, their binary structure remains constant, allowing attackers to develop reliable exploits (like ROP-chains). Heuristic OS mitigates this by constantly mutating its internal executable code at runtime while maintaining exact functional consistency.

> **Project Status & Lineage:** This repository represents a specialized development branch (fork) of the seL4/CAmkES ecosystem. It is tailored specifically for heuristic security and polymorphic execution research and does not represent the upstream seL4 project.

## 🎯 Purpose & Motivation
The primary goal of this architecture is to create a Trusted Execution Environment (TEE) where:
1. **Static Analysis is Obsolete:** The binary code changes so frequently that a captured memory dump becomes invalid within seconds.
2. **Exploit Development is Mitigated:** Because memory offsets, instruction sequences, and register usages are randomized at runtime, an attacker cannot reliably predict the address space layout.
3. **Integrity is Mathematically Proven:** Every piece of dynamic code is cryptographically verified by a hardware-root-of-trust (fTPM) before a single instruction is executed.

## 🛠 Core Technical Features

### 1. Runtime Dynamic Polymorphism (LLVM-based)
The system contains a dedicated **Generator** component utilizing a custom LLVM Pass. It synthesizes unique x86_64 instruction sequences at runtime, injecting dead code and applying logic masking to produce entirely different binary representations of the same algorithm.

### 2. ChaCha20-based CSPRNG
To ensure maximum entropy and unpredictability, the mutation engine is powered by a **ChaCha20 Cryptographically Secure Pseudo-Random Number Generator**. It is initialized directly using the CPU's hardware timestamp counter (`rdtsc`).

### 3. fTPM Attestation (HMAC-SHA256)
A simulated **Firmware Trusted Platform Module (fTPM)** secures the component interactions. 
* **Signing:** Every newly generated code variant is hashed (Jenkins One-at-a-Time) and signed with a 256-bit secret key stored inside the isolated TPM memory space.
* **Verification:** The system monitor (Dispatcher) performs cross-component attestation, ensuring the code was not tampered with during transit in shared memory.

### 4. Secure Runtime Injection (JIT Sandbox)
The system leverages seL4's fine-grained capability-based memory permissions. The **Worker** component operates in a strict sandbox with `RX` (Read-Execute) permissions, executing only the verified polymorphic payloads securely injected by the Dispatcher.

## 🏗 System Architecture & Execution Flow

1. **Orchestration:** The `Dispatcher` triggers a mutation request via RPC based on the `Scheduler` tick.
2. **Synthesis:** The `Generator` compiles new opcodes into a `SharedData` buffer.
3. **Registration:** The `Repository` calculates the hash of the new binary.
4. **Attestation:** The `TPM_Device` generates an HMAC-SHA256 signature for the hash.
5. **Verification:** The `Dispatcher` reads the code, re-hashes it locally, and verifies the signature against the TPM.
6. **Hot-Swap:** If verified, the `Dispatcher` performs a secure memory copy into the `Worker` arena.
7. **Execution:** The `Worker` executes the newly injected polymorphic task hardware-natively.

## 🚀 Build and Simulation

The project requires an `x86_64` environment with the standard seL4/CAmkES build toolchain installed.

```bash
# 1. Setup the build directory
mkdir build && cd build

# 2. Run the automated Docker-based build and simulation script
../build_and_run.sh

# 3. The script will automatically compile the system and launch the QEMU simulator
