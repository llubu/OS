#include <defs.h>
#include <stdio.h>

#include <sys/pmem_manager.h>
#include <sys/kern_page_table.h>
#include <sys/gdt.h>
#include <sys/tarfs.h>
# include <sys/idt.h>
# include <sys/v_mem_manager.h>
# include <sys/tarfs.h>

#define INITIAL_STACK_SIZE    (4096u)

char stack[INITIAL_STACK_SIZE];
uint32_t* loader_stack;
extern char kernmem, physbase;
struct tss_t tss;

void start(uint32_t* modulep, void* physbase, void* physfree)
{
	clear_screen();
	while(modulep[0] != 0x9001)
		modulep += (modulep[1] + 2);
	for (smap = (struct smap_t*)(modulep + 2);
		(smap < (struct smap_t*)((char*)modulep + modulep[1] + 2 * 4));
		++smap)
	{
		if ((1 == smap->type) && (smap->length))
			free_page_list(smap, physbase, physfree);
	}
	// kernel starts here
	// increasing physfree by 1 MB accomodating free page list
	physfree +=  1048576u;
	kern_pt((void *) &kernmem, (uint64_t)physbase, (uint64_t)physfree);
	init_VM((uint64_t) physfree);
	init_task();
	init_fdtable();
	clear_screen();
	init_shell();
	while(1);
}

void boot(void)
{
	__asm__(
		"movq %%rsp, %0;"
		"movq %1, %%rsp;"
		:"=g"(loader_stack)
		:"r"(&stack[INITIAL_STACK_SIZE])
	);
	__asm volatile("cli");
	reload_gdt();
	reload_idt();
	setup_tss();
	__asm volatile("sti");
	start((uint32_t*)((char*)(uint64_t)loader_stack[3]
		+ (uint64_t)&kernmem - (uint64_t)&physbase), &physbase,
		(void*)(uint64_t)loader_stack[4]);

	while(1);
}

