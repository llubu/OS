.global _set_k_ptable_cr3
.global _set_paging

_set_paging:
	movq %rdi, %rax
	movq %rax, %cr0
	retq

_set_k_ptable_cr3:
	movq %rdi, %rax
	movq %rax, %cr3
	retq

