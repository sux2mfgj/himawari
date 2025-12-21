.intel_syntax noprefix
.section .text.boot16
.global _boot16
.code16

_boot16:
  # AP starts in real mode at CS:IP where CS = vector and IP = 0
  # For vector 0x10, CS = 0x1000, IP = 0, physical address = 0x10000
  # Disable interrupts
  cli

  # Set up data segment to match code segment
  mov ax, cs
  mov ds, ax

  # Load our own GDT pointer (segment-relative address)
  lgdt cs:[ap_gdt_ptr - _boot16]

  # Enable protected mode
  mov eax, cr0
  or al, 0x1           # CR0.PE = 1
  mov cr0, eax

  # Jump to 32-bit protected mode code
  # Use a far jump to load CS with 32-bit code segment
  .byte 0x66  # Operand size override to 32-bit
  .byte 0xea  # Far jump opcode
  .long (protected_mode_start - _boot16 + 0x10000)  # Offset
  .word 0x08  # Code segment selector

.code32
protected_mode_start:
  # Now in 32-bit protected mode
  # Set up segment registers
  mov ax, 0x10
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax

  # Enable PAE (required for long mode)
  mov eax, cr4
  or eax, 0x20          # CR4.PAE = 1
  mov cr4, eax

  # Load CR3 with the BSP's page table address
  mov eax, OFFSET pml4_table
  mov cr3, eax

  # Enable long mode in EFER MSR
  mov ecx, 0xC0000080   # EFER MSR number
  rdmsr
  or eax, 0x100         # Set LME (Long Mode Enable)
  wrmsr

  # Enable paging
  mov eax, cr0
  or eax, 0x80000000    # Set PG (Paging)
  mov cr0, eax

  # Now in compatibility mode, load the real GDT
  lgdt [gdt_ptr]

  # Jump to 64-bit long mode
  jmp 0x08:OFFSET ap_long_mode

.code64
ap_long_mode:
  # Now in 64-bit long mode
  # Setup segment registers
  mov ax, 0x10
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax

  # Setup stack (each AP needs its own stack)
  # For now, use a temporary stack
  # TODO: Allocate per-AP stacks
  mov rsp, OFFSET _early_stack_top
  sub rsp, 0x1000  # Offset stack to avoid BSP collision

  # Jump to AP C code
  call ap_main

  # If ap_main returns, halt
  cli
ap_halt_loop:
  hlt
  jmp ap_halt_loop

# Temporary GDT for AP bootstrap (must be in low memory)
.align 8
ap_gdt_start:
  .quad 0x0000000000000000    # Null descriptor
  .quad 0x00CF9A000000FFFF    # 32-bit code segment (selector 0x08)
  .quad 0x00CF92000000FFFF    # 32-bit data segment (selector 0x10)
ap_gdt_end:

ap_gdt_ptr:
  .word ap_gdt_end - ap_gdt_start - 1
  .long ap_gdt_start - _boot16 + 0x10000  # Physical address

# External symbols
.extern pml4_table
.extern gdt_ptr
.extern _early_stack_top
.extern ap_main
