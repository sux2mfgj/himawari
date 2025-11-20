.intel_syntax noprefix

# PVH ELF Note for direct 64bit boot
.section .note.Xen, "a"
.align 4
pvh_note_start:
  .long 2f - 1f              # namesz
  .long 4f - 3f              # descsz
  .long 0x12                 # type = XEN_ELFNOTE_PHYS32_ENTRY (18 = 32bit PVH)
1:
  .asciz "Xen"
2:
  .align 4
3:
  .long _boot                # entry point (physical address)
  .long 0                    # padding for 64bit
4:
  .align 4

.section .text
.global _boot

_boot:
  # PVH boot: already in 64bit long mode
  # EBX contains pointer to hvm_start_info struct

  mov rsp, OFFSET __early_stack_top

  call kernel_cmain

  cli
hlt_loop:
  hlt
  jmp hlt_loop
