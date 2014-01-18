/**
* Kernel Page table implementation, 4Kb pages
* Implements Paging for kernel, starts at kernmem in virtual address space
* 
**/

# include <stdio.h>
# include <defs.h>
# include <sys/pmem_manager.h>

extern void _set_k_ptable_cr3(uint64_t);

/* Implements page table for kernel */
void kern_pt(void *v_kern, uint64_t pbase, uint64_t pfree)
{
	uint64_t *p1, *p2, *p3, *p4; 	// PML4E, PDPE, PDE, PTE p9 is used to edit other page tables by mapping to this page
	int p1_va, p2_va, p3_va, p4_va, j;	// 9 bits each from v_kern for index into Page table hiearchy
	uint64_t vadd = (uint64_t) v_kern;
	uint64_t kva = (uint64_t) v_kern;	// loop variable for PTE
//	uint64_t cr;
	
	p1 = (uint64_t *) get_page();		// 4Kb physical page
	p2 = (uint64_t *) get_page();		// 4 level Page Tables
	p3 = (uint64_t *) get_page();
	p4 = (uint64_t *) get_page();
//	p9 = (uint64_t *) get_page();		// TO map processes page tables

	p1_va = (vadd << 16) >> (9 + 9 + 9 + 12 + 16);
	p2_va = (vadd << (16 +9)) >> (9 + 9 + 12 + 16 + 9);
	p3_va = (vadd << (16 + 9 + 9)) >> (9 + 12 + 16 + 9 + 9);
	p4_va = (vadd << (16 + 9 + 9 + 9)) >> (12 + 16 + 9 + 9 + 9);


	(*(p1 + p1_va)) = ((((uint64_t)p2)) | 7);
	(*(p2 + p2_va)) = ((((uint64_t)p3) ) | 7);
	(*(p3 + p3_va)) = ((((uint64_t)p4) ) | 7);

	/**** SELF REFRENCE of PML4E in index 510 */
	(*(p1+510)) = ((uint64_t)p1 | 7);
	/*****************************************/
//	(*(p1+509)) = ((uint64_t)p9 | 7);
	/*****************************************/
//	(*(p9+510)) = ((uint64_t)p9 | 7);
	/*****************************************/

	j = 0;
//	printf("\n physfree in KERN_PT:%x",pfree);
	for ( kva =(uint64_t) pbase; kva <= ((uint64_t)pfree); kva += 4096)
	{	
//		if (j> 511)
//		{
//			break;
//		}
		(*(p4 + p4_va + j)) = ((((uint64_t)kva) & 0xFFFFFFFFFF000) | 3);		// adding PTE entry for the new physical page
		j++;
	}
//	if (j > 511)
//	{

//	}
//	cr = (vadd + (pfree - pbase));		// v add where kernel ends

//	printf("\n p5:%x, j%d", cr, j);
//	printf("\n Kva:%x:%x:kva--%x:%x:%x", kva,((uint64_t)vadd + (uint64_t)pfree), (kva-4096) ,(uint64_t)pbase, (uint64_t)pfree);
//	printf("\n jk \na");
	(*(p4 + p4_va + j)) =  ((((uint64_t)0xB8000) & 0xFFFFFFFFFF000) | 3);			// Remapping Video buffer physical memory

	/* Setting Video Memory before setting the CR3 */
	(video) = (0xffffffff80000000 | kva); 
	/* Setting up free list head cast to kernel Virtual address */
	vm_cast = 0xffffffff80000000;								// Virual kernel mapping starts here
	/* Setting up paging & CR3 register */
	_set_k_ptable_cr3((uint64_t)p1);

//	printf("\nTrying:%x", (uint64_t *) get_page());
	printf("\nKernel Paging done: video:%x", video);
}

uint64_t construct_address(uint64_t add1, int b1, int b2, int b3)
{
	add1 = (((add1 >> b1) << 9 | b2) << (b3));
	return add1;
}	
	 

void self_refrence(uint64_t v_a)
{
	int p1_va, p2_va, p3_va, p4_va;	// 9 bits each from v_kern for index into Page table hiearchy -- INDEX into 4 level page tables
	uint64_t add = 0xFFFF000000000000;	// Base address to build on
	uint64_t *p1 = NULL, *p2 = NULL, *p3 = NULL, *p4 = NULL, *p5 = NULL;		// Pointers to each level Page table p1 == PML4E
	uint64_t v_ad;

	v_ad = v_a;
	p1_va = (v_ad << 16) >> (9 + 9 + 9 + 12 + 16);	// Index in PML4E
	p2_va = (v_ad << (16 +9)) >> (9 + 9 + 12 + 16 + 9); // Index in PDPE
	p3_va = (v_ad << (16 + 9 + 9)) >> (9 + 12 + 16 + 9 + 9); // Index in PDE 
	p4_va = (v_ad << (16 + 9 + 9 + 9)) >> (12 + 16 + 9 + 9 + 9); //Index in PTE
//	printf("VADD:%x", v_ad);
//	printf("\nIndex:p1:%d:p2:%d:p3:%d:p4:%d", p1_va, p2_va, p3_va, p4_va);
	/* Creating self looping for PML4E */	
// abhi Write a single function for this
	add = (((add >> 48) << 9 | 0x1FE) << 39);  // Sets the 1st 9 bits to 510 for  self refrenccing
	add = (((add >> 39) << 9 | 0x1FE) << 30);  // Sets the 2nd 9 bits to 510 for  self refrenccing
	add = (((add >> 30) << 9 | 0x1FE) << 21);  // Sets the 3rd 9 bits to 510 for  self refrenccing
	add = (((add >> 21) << 9 | 0x1FE) << 12);  // Sets the 4th 9 bits to 510 for  self refrenccing
	p1 = (uint64_t *)add;
	p2 = (uint64_t *)p1[p1_va];
//	printf("\n ADD1:p1:%x", add);

	if (!p2)
	{
//		printf("\n p2 not found in p1:Allocating");
		p2 = (uint64_t *) get_page();
		p1[p1_va] = ((((uint64_t)p2) & 0xFFFFFFFFFF000) | 7);
//		printf("\n p2 not found in p1:AllocatED:%x", p2);
	}
	
/**************************/	
	add = 0xFFFF000000000000;
	add = (((add >> 48) << 9 | 0x1FE) << 39);
	add = (((add >> 39) << 9 | 0x1FE) << 30);
	add = (((add >> 30) << 9 | 0x1FE) << 21);
	add = (((add >> 21) << 9 | p1_va) << 12);
	p2 = (uint64_t*)add;
	p3 = (uint64_t*)p2[p2_va];
//	printf("\n ADD2:p2:%x", add);

	if(!p3)
	{
//		printf("\n p3 not found in p2:Allocating");
		p3 = (uint64_t *) get_page();
		p2[p2_va] = ((((uint64_t) p3) & 0xFFFFFFFFFF000) | 7);
//		printf("\n p3 not found in p2:AllocatED:%x", p3);
	}

/*************************/

	add = 0xFFFF000000000000; 
	add = (((add >> 48) << 9 | 0x1FE) << 39);
	add = (((add >> 39) << 9 | 0x1FE) << 30);
	add = (((add >> 30) << 9 | p1_va) << 21);
	add = (((add >> 21) << 9 | p2_va) << 12);
	p3 = (uint64_t*)add;
	p4 = (uint64_t*)p3[p3_va];
//	printf("\n ADD3:p3:%x", add);
//	printf("\n P4:%x:Val:%x", p4, p3[p3_va]);
//while(1);
	if(!p4)
	{
//		printf("\n p4 not found in p3:Allocating");
//while(1);
		p4 = (uint64_t *) get_page();
//while(1);
		p3[p3_va] = ((((uint64_t) p4) & 0xFFFFFFFFFF000) | 7);
//while(1);
//		printf("\n p4 not found in p3:AllocatED:%x", p4);
	}

/**************************/
//while(1);
	add = 0xFFFF000000000000; 
	add = (((add >> 48) << 9 | 0x1FE) << 39);
	add = (((add >> 39) << 9 | p1_va) << 30);
	add = (((add >> 30) << 9 | p2_va) << 21);
	add = (((add >> 21) << 9 | p3_va) << 12);
	p4 = (uint64_t*)add;
	p5 = (uint64_t*)p4[p4_va];
//	printf("\n ADD4:p4:%x", add);
//while(1);				
	if(!p5)
	{
//		printf("\n p5 not found in p4:Allocating");
		p5 = (uint64_t *)get_page();
//		printf("Getting Page:%x", p5);
		p4[p4_va] = ((((uint64_t) p5) & 0xFFFFFFFFFF000) | 7);
//		printf("\n p5 not found in p4:AllocatED:%x", p5);
	}

//	printf("\nASTALAVISTA");
}	
