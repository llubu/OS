#ifndef _KERN_PAGE_TABLE
#define _KERN_PAGE_TABLE

void kern_pt(void *v_kern, uint64_t pbase, uint64_t pfree);		// Implements paging for kernel
uint64_t construct_address(uint64_t add1, int b1, int b2, int b3);	// Constructs the Virtual address based on masks passed
void self_refrence(uint64_t v_add);		// Implements Self refrencing in Page tables to edit them once the CR3 register is set

#endif

