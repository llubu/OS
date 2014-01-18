# include <sys/util.h>
# include <sys/execve.h>
# include <stdio.h>
# include <sys/gdt.h>
# include <sys/task_management.h>
# include <sys/tarfs.h>
# include <sys/string.h>

extern void _set_k_ptable_cr3(uint64_t);
/*
* Execve kernel function, can be called through a system call from execve wrapper
*
*/

void do_execve(char *fn)
{
	PCB *pro;
	pro = running;
  	char *filename;

  	filename = (char*)(k_malloc((sizeof(char)*32)));
  	strcpy(filename, fn);
  
  	// delete all page table entries
  	deletePageTables();
	_set_k_ptable_cr3(pro->cr3);
  
	read_tarfs(pro, filename);
	if ((pro->u_stack = process_stack()) == NULL)
	{
//		printf("\n Cant allocate memory for process User stack");
		//exit();
	}	
  
  
	pro->rsp = (uint64_t)(pro->u_stack);
//  	printf("user_stack_rsp%p",pro->rsp);
	tss.rsp0 = (uint64_t)  &(pro->kernel_stack[255]);
//	printf("In Execve()  GDT SET\n");
	__asm volatile("\
	push $0x23;\
	push %0;\
	push $0x200;\
	push $0x1B;\
	push %1"::"g"(pro->u_stack),"g"(pro->rip):"memory");
	__asm volatile("\
	iretq;\
  ");

}














