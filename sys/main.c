#include <defs.h>
#include <stdio.h>
#include <sys/pmem_manager.h>
#include <sys/kern_page_table.h>
#include <sys/gdt.h>
#include <sys/tarfs.h>
//#include "idt.h"
//#include "irq.h"
# include <sys/idt.h>
# include <sys/v_mem_manager.h>
# include <sys/tarfs.h>


#define INITIAL_STACK_SIZE 4096
char stack[INITIAL_STACK_SIZE];
uint32_t* loader_stack;
extern char kernmem, physbase;
struct tss_t tss;
//uint64_t opfree = 0;

void start(uint32_t* modulep, void* physbase, void* physfree)
{
	clear_screen();
//	printf("Pysfree in mainbefore:BASE:%x:%x", physfree, physbase);
	
	while(modulep[0] != 0x9001) modulep += modulep[1]+2;
	for(smap = (struct smap_t*)(modulep+2); smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); ++smap) {
		if (smap->type == 1 /* memory */ && smap->length != 0) {
//			printf("Available Physical Memory [%x-%x]\n", smap->base, smap->base + smap->length);
			free_page_list(smap, physbase, physfree);	// Creates free page list (linked list implementation)
		}
	}
//	printf("tarfs in [%p:%p]\n", &_binary_tarfs_start, &_binary_tarfs_end);
	// kernel starts here	
	physfree +=  1048576;	// increasing physfree by 1 MB accomodating free page list 
//	uint64_t cur_VK = ((uint64_t )physfree + 0xffffffff80000000);		// Free Virtual Memory above Kernel starts from here
//	uint64_t cur_PK = 0x2097152;	// Starts at 2 MB mark (abhi confirm)
//	uint64_t pbase99 = (uint64_t)((uint64_t)physfree + 0xffffffff80000000);
//	uint64_t pid_bitmap[32]= {0};
//	printf("PhysFREE in mainnow:%x", physfree);
//	printf("\nFREE_PAGE:%x:%x:%x", (uint64_t *) get_page());
//	printf("\n VK:%x", cur_VK);
	kern_pt((void *) &kernmem, (uint64_t)physbase, (uint64_t)physfree);	// Mapping Kernel to new Page Table
	init_VM((uint64_t) physfree);
	init_task();
	init_fdtable();
	clear_screen();
	init_shell();
//	uint64_t te = 0xFFFF00FF80000000;
//	*te = 1;
//	void *te = (void *)  0xFFFFFFFF80000000;
//	void *te = (void *)  0xFFFF000080000000;
//	uint64_t te =(uint64_t) (0xffffffff80000000 + physbase);
//	uint64_t te =  0xFFFFFEFF80000000;
//	self_refrence(te);	
//	call_first();
	//update physfree
//	printf("\n CHAR:%c:%s:%d:\n:%x:%p", 'A', "STONY !@#$ BROOK", 9999, 0xFFFF1B, &(stack));
//	printf("\n STRING:%s","ABHIROOP DABRAL");
//	printf("\n INT:%d", 0xFFC);
//	printf("\n HEX:%x", 510);
//	printf("\n PT:%p",(uint64_t *) te);

	while(1);
}

void boot(void)
{
	// note: function changes rsp, local stack variables can't be practically used
//	register char *temp1, *temp2;
	__asm__(
		"movq %%rsp, %0;"
		"movq %1, %%rsp;"
		:"=g"(loader_stack)
		:"r"(&stack[INITIAL_STACK_SIZE])
	);
	__asm volatile("cli");	
	reload_gdt();
//	init_idt();
	reload_idt();
//	init_timer();
	setup_tss();
	__asm volatile("sti");
	start(
		(uint32_t*)((char*)(uint64_t)loader_stack[3] + (uint64_t)&kernmem - (uint64_t)&physbase),
		&physbase,
		(void*)(uint64_t)loader_stack[4]
	);
//	for(
//		temp1 = "!!!!! start() returned !!!!!", temp2 = (char*)0xb8000;
//		*temp1;
//		temp1 += 1, temp2 += 2
//	) *temp2 = *temp1;
	while(1);
}
