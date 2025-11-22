.intel_syntax noprefix

# PVH ELF Note for direct 64bit boot
.section .note.Xen, "a"
.align 4

pvh_note_entry:
  .long 2f - 1f              # namesz
  .long 4f - 3f              # descsz
  .long 18                   # type = XEN_ELFNOTE_PHYS32_ENTRY
1:
  .asciz "Xen"
2:
  .align 4
3:
  .long _boot                # entry point (physical address)
4:
  .align 4

.section .text
.global _boot
.code32

_boot:
  # PVH boot: Start in 32-bit protected mode
  # EBX contains pointer to hvm_start_info struct

  # Disable interrupts
  cli

  # Enable PAE (required for long mode)
  mov eax, cr4
  or eax, 0x20          # CR4.PAE = 1
  mov cr4, eax

  # Load CR3 with page table address
  mov eax, OFFSET pml4_table
  mov cr3, eax

  # Enable long mode in EFER MSR
  mov ecx, 0xC0000080   # EFER MSR number
  rdmsr
  or eax, 0x100         # Set LME (Long Mode Enable)
  wrmsr

  # Enable paging and protection
  mov eax, cr0
  or eax, 0x80000001    # Set PG (Paging) and PE (Protection Enable)
  mov cr0, eax

  # Now in compatibility mode, need to load 64-bit code segment
  lgdt [gdt_ptr]
  jmp 0x08:long_mode

.code64
long_mode:
  # Now in 64-bit long mode
  # Setup segment registers
  mov ax, 0x10
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax

  # Setup stack
  mov rsp, OFFSET __early_stack_top

  # prepare an argument to pass hvm_start_info to C code.
  mov rdi, rbx

  # Jump to C code
  call kernel_cmain

  cli
hlt_loop:
  hlt
  jmp hlt_loop

# GDT for long mode
.align 16
gdt_start:
  .quad 0x0000000000000000    # Null descriptor
  .quad 0x00AF9A000000FFFF    # 64-bit code segment (selector 0x08)
  .quad 0x00CF92000000FFFF    # 64-bit data segment (selector 0x10)
gdt_end:

gdt_ptr:
  .word gdt_end - gdt_start - 1
  .quad gdt_start

# Page tables for identity mapping first 2MB
.align 4096
pml4_table:
  .quad pdp_table + 0x3   # Present, writable
  .fill 511, 8, 0

.align 4096
pdp_table:
  .quad pd_table + 0x3    # Present, writable
  .fill 511, 8, 0

.align 4096
pd_table:
  .quad 0x83              # 2MB page, present, writable, PS=1
  .fill 511, 8, 0
