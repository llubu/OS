# include <stdio.h>

# include <sys/util.h>
# include <sys/execve.h>
# include <sys/gdt.h>
# include <sys/task_management.h>
# include <sys/tarfs.h>
# include <sys/string.h>

#define FILE_NAME_SIZE    (32u)
extern void _set_k_ptable_cr3(uint64_t);

void do_execve(char *fn)
{
	PCB *pro;
	pro = running;
	char *filename;

	filename = (char*)(k_malloc((sizeof(char) * FILE_NAME_SIZE)));
	strncpy(filename, fn, FILE_NAME_SIZE);

	// delete all page table entries
	deletePageTables();
	_set_k_ptable_cr3(pro->cr3);

	read_tarfs(pro, filename);
	if ((pro->u_stack = process_stack()) == NULL)
	{
		//exit();
	}

	pro->rsp = (uint64_t)(pro->u_stack);
	tss.rsp0 = (uint64_t)  &(pro->kernel_stack[255]);

	__asm volatile("\
			push $0x23;\
			push %0;\
			push $0x200;\
			push $0x1B;\
			push %1"::"g"(pro->u_stack),"g"(pro->rip):"memory"
	);
	__asm volatile("\
			iretq;\"
	);
}


