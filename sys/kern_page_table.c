// Kernel Page table implementation, 4Kb pages
// Implements Paging for kernel, starts at kernmem in virtual address space

# include <stdio.h>
# include <defs.h>

# include <sys/pmem_manager.h>

extern void _set_k_ptable_cr3(uint64_t);

// Implements page table for kernel
void kern_pt(void* v_kern, uint64_t pbase, uint64_t pfree)
{
	// PML4E, PDPE, PDE, PTE p9 is used to edit other
	// page tables by mapping to this page
	uint64_t* p1;
	uint64_t* p2;
	uint64_t* p3;
	uint64_t* p4;
	// 9 bits each from v_kern for index into Page table hiearchy
	int p1_va, p2_va, p3_va, p4_va, j;
	uint64_t vadd = (uint64_t)v_kern;
	uint64_t kva = (uint64_t)v_kern;	// loop variable for PTE

	p1 = (uint64_t*)get_page();		// 4Kb physical page
	p2 = (uint64_t*)get_page();		// 4 level Page Tables
	p3 = (uint64_t*)get_page();
	p4 = (uint64_t*)get_page();

	p1_va = (vadd << 16) >> (9 + 9 + 9 + 12 + 16);
	p2_va = (vadd << (16 +9)) >> (9 + 9 + 12 + 16 + 9);
	p3_va = (vadd << (16 + 9 + 9)) >> (9 + 12 + 16 + 9 + 9);
	p4_va = (vadd << (16 + 9 + 9 + 9)) >> (12 + 16 + 9 + 9 + 9);


	(*(p1 + p1_va)) = ((((uint64_t)p2)) | 7);
	(*(p2 + p2_va)) = ((((uint64_t)p3) ) | 7);
	(*(p3 + p3_va)) = ((((uint64_t)p4) ) | 7);

	(*(p1 + 510)) = ((uint64_t)p1 | 7);

	j = 0;
	for (kva = pbase; kva <= ((uint64_t)pfree); kva += 4096)
	{
		// adding PTE entry for the new physical page
		(*(p4 + p4_va + j)) = (((uint64_t)kva & 0xffffffffff000) | 3);
		j++;
	}
	// Remapping Video buffer physical memory
	(*(p4 + p4_va + j)) = (((uint64_t)0xb8000 & 0xffffffffff000) | 3);

	// Setting Video Memory before setting the CR3
	(video) = (0xffffffff80000000 | kva);
	// Setting up free list head cast to kernel Virtual address

	// Virual kernel mapping starts here
	vm_cast = 0xffffffff80000000;
	// Setting up paging & CR3 register
	_set_k_ptable_cr3((uint64_t)p1);

	printf("\nKernel Paging done: video:%x", video);
}

uint64_t construct_address(uint64_t add1, int b1, int b2, int b3)
{
	add1 = ((((add1 >> b1) << 9) | b2) << b3);
	return add1;
}

void self_refrence(uint64_t v_a)
{
	// 9 bits each from v_kern for index into Page table hiearchy
	// -- INDEX into 4 level page tables
	int p1_va, p2_va, p3_va, p4_va;
	uint64_t add = 0xffff000000000000;    // Base address to build on
	// Pointers to each level Page table p1 == PML4E
	uint64_t* p1 = NULL;
	uint64_t* p2 = NULL;
	uint64_t* p3 = NULL;
	uint64_t* p4 = NULL;
	uint64_t* p5 = NULL;
	uint64_t v_ad;

	v_ad = v_a;
	p1_va = (v_ad << 16) >> (9 + 9 + 9 + 12 + 16);
	p2_va = (v_ad << (16 +9)) >> (9 + 9 + 12 + 16 + 9);
	p3_va = (v_ad << (16 + 9 + 9)) >> (9 + 12 + 16 + 9 + 9);
	p4_va = (v_ad << (16 + 9 + 9 + 9)) >> (12 + 16 + 9 + 9 + 9);

	// Creating self looping for PML4E
	// Sets the 1st 9 bits to 510 for  self refrenccing
	add = (((add >> 48) << 9 | 0x1fe) << 39);
	add = (((add >> 39) << 9 | 0x1fe) << 30);
	add = (((add >> 30) << 9 | 0x1fe) << 21);
	add = (((add >> 21) << 9 | 0x1fe) << 12);
	p1 = (uint64_t*)add;
	p2 = (uint64_t*)p1[p1_va];

	if (!p2)
	{
		p2 = (uint64_t *) get_page();
		p1[p1_va] = ((((uint64_t)p2) & 0xffffffffff000) | 7);
	}

	add = 0xffff000000000000;
	add = (((add >> 48) << 9 | 0x1fe) << 39);
	add = (((add >> 39) << 9 | 0x1fe) << 30);
	add = (((add >> 30) << 9 | 0x1fe) << 21);
	add = (((add >> 21) << 9 | p1_va) << 12);
	p2 = (uint64_t*)add;
	p3 = (uint64_t*)p2[p2_va];

	if (!p3)
	{
		p3 = (uint64_t *) get_page();
		p2[p2_va] = ((((uint64_t) p3) & 0xffffffffff000) | 7);
	}

	add = 0xffff000000000000;
	add = (((add >> 48) << 9 | 0x1fe) << 39);
	add = (((add >> 39) << 9 | 0x1fe) << 30);
	add = (((add >> 30) << 9 | p1_va) << 21);
	add = (((add >> 21) << 9 | p2_va) << 12);
	p3 = (uint64_t*)add;
	p4 = (uint64_t*)p3[p3_va];

	if(!p4)
	{
		p4 = (uint64_t *) get_page();
		p3[p3_va] = ((((uint64_t) p4) & 0xffffffffff000) | 7);
	}

	add = 0xffff000000000000;
	add = (((add >> 48) << 9 | 0x1fe) << 39);
	add = (((add >> 39) << 9 | p1_va) << 30);
	add = (((add >> 30) << 9 | p2_va) << 21);
	add = (((add >> 21) << 9 | p3_va) << 12);
	p4 = (uint64_t*)add;
	p5 = (uint64_t*)p4[p4_va];
	if(!p5)
	{
		p5 = (uint64_t *)get_page();
		p4[p4_va] = ((((uint64_t) p5) & 0xffffffffff000) | 7);
	}
}

