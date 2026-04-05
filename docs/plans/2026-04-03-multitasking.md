# Multitasking Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement cooperative and preemptive multitasking so multiple user-mode processes run concurrently with round-robin scheduling.

**Architecture:** Each process has a PCB (process control block) with a separate kernel stack and page directory. Context switching is done via a `switch_context` assembly routine that saves callee-saved registers on the current stack, switches ESP/CR3, and restores from the new stack. Cooperative scheduling uses a `SYS_YIELD` syscall. Preemptive scheduling uses the PIT (Programmable Interval Timer) at 50 Hz to trigger timer interrupts that call the scheduler. New processes are created via `SYS_SPAWN` which looks up programs in the SOFS filesystem.

**Tech Stack:** i686-elf-gcc, NASM, QEMU

---

## File Structure

**New files:**

| File | Responsibility |
|------|---------------|
| `include/process.h` | PCB struct, process API, scheduler declarations |
| `src/process.c` | Process table, creation, round-robin scheduler |
| `src/context_switch.s` | Assembly: switch_context, jump_to_process, process_start_trampoline |
| `include/pit.h` | PIT driver declaration |
| `src/pit.c` | PIT configuration and IRQ0 unmasking |
| `programs/hello.c` | Second test program for demonstrating multitasking |

**Modified files:**

| File | Change |
|------|--------|
| `include/syscall.h` | Add SYS_YIELD (3) and SYS_SPAWN (4) |
| `src/syscall.c` | Handle yield, spawn, and proper exit with scheduling |
| `src/idt.c` | Timer interrupt calls scheduler for preemptive scheduling |
| `src/kmain.c` | Use process system instead of direct enter_usermode |
| `programs/syscall.h` | Add yield() and spawn() user-side wrappers |
| `programs/program.c` | Use yield to demonstrate cooperative scheduling |
| `Makefile` | New source files, second user program |

---

### Task 1: Context Switch Assembly

- Create: `src/context_switch.s`

### Task 2: Process Control Block and Scheduler

- Create: `include/process.h`
- Create: `src/process.c`

### Task 3: New Syscalls (yield, spawn, proper exit)

- Modify: `include/syscall.h`
- Modify: `src/syscall.c`

### Task 4: PIT Driver for Preemptive Scheduling

- Create: `include/pit.h`
- Create: `src/pit.c`
- Modify: `src/idt.c`

### Task 5: Update kmain to Use Process System

- Modify: `src/kmain.c`

### Task 6: User-Mode Programs

- Modify: `programs/syscall.h`
- Modify: `programs/program.c`
- Create: `programs/hello.c`

### Task 7: Update Makefile, Build and Test

- Modify: `Makefile`

Expected output in QEMU:
```
SOFS: 2 file(s) loaded.
Starting processes...
Process A started
Process B started
Process A resumed
Process B resumed
Process A exiting
Process B exiting
```
