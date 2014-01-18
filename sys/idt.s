.global _x86_64_asm_lidt
_x86_64_asm_lidt:
#  cli
	lidt (%rdi)
#  sti
  retq
