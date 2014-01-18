/*
* Contain functions for scheduler and related function
*
*/

# include <sys/task_switch.h>
# include <sys/gdt.h>
# include <stdio.h>
# include <sys/v_mem_manager.h>
# include <sys/process_que.h>

extern void _set_k_ptable_cr3(uint64_t);

void schedule1()
{

//	PCB *current = NULL;
	uint64_t kern_id = 234; 
	switchTo = get_nextProcess(runableQ);
	if (switchTo == NULL)
	{
		switchTo = running;
	}	
	if (-1 == (lookintoQ(waitQ, running->pid)))
	{
		add_toQ(runableQ, running);
//		current = running;	
//		printf("TO BE SCHED:%x:current:%x:",switchTo->pid, current->pid);
	}	
	else
	{
//		printf("\n current process is in wait Q, scheduling next in runable");
	}	

/* Saving current process stuff */ 
// Add current process to runableQ
//	current->c_PK = cur_PK;
//	current->c_VK = cur_VK;
/* CALL init_VM() in execeve() */

/* Setting CR3 .. CHANGING PAGE TABLES */
	running = switchTo;		// Pointing global pointer to to be scheduled process
	_set_k_ptable_cr3(switchTo->cr3);


/*************************************************************************/
	tss.rsp0 = (uint64_t) &(switchTo->kernel_stack[255]);

	if (switchTo->kernel_stack[255] == 0x0)
	{
		if (switchTo->kernel_stack[254] != 0x0 && switchTo->kernel_stack[254] == 0x23)	//somethig extra was pushed on kernel stack last time adjust index
		{
			kern_id = 235;
		}
	}
	else if (switchTo->kernel_stack[255] == 0x23)
	{
		kern_id = 236;
	}
		
	__asm volatile(
		"movq %0, %%rsp;\n\t"
		"popq %%r15\n\t"
		"popq %%r14\n\t"
		"popq %%r13\n\t"
		"popq %%r12\n\t"
		"popq %%r11\n\t"	
		"popq %%r10\n\t"	
		"popq %%r9\n\t"	
		"popq %%r8\n\t"
//		"popq %%rsp\n\t"
		"popq %%rbp\n\t"
		"popq %%rdi\n\t"
		"popq %%rsi\n\t"
		"popq %%rdx\n\t"
		"popq %%rcx\n\t"
		"popq %%rbx\n\t"
		"popq %%rax\n\t"
		:
		:"r"(&(switchTo->kernel_stack[kern_id]))
	        :"memory"
	);
	
	__asm volatile(
		"iretq"
		);

}
