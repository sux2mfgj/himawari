.section .text.boot16
.global _boot16
.code16

_boot16:
  # Disable interrupts
  cli

  hlt
