# include <stdio.h>

# include <sys/pmem_manager.h>
# include <sys/v_mem_manager.h>
# include <sys/task_management.h>
# include <sys/tarfs.h>
# include <sys/gdt.h>

uint64_t cur_VK;
uint64_t cur_PK;
uint64_t uStack_Top;
extern void _set_k_ptable_cr3(uint64_t);

void init_VM(uint64_t phfree)
{
	cur_VK = (phfree + (16 * 1024) + 0xffffffff7fffff00) ;
	cur_PK = 0x300000;
	uStack_Top = (0xffffffff80000000 + UTOP);
}

// Kernel Malloc- Virtual Memory- Uses get_page() to get 4Kb physical pages
// Allocates memory only above PhysFree - for Kernel Data Structure
// param : No of Bytes needed by Kernel code (uint64_t)
// Global: cur_VK; strats from kernmem + Physfree
// return: void* pointer to the memory needed

void *k_malloc(uint64_t no_bytes)
{
	uint64_t pt = NULL;
	// abhi add check for top address boundary condition
	if (MAX_KERN <= (cur_VK + 4096))
	{
		//exit();
		printf("\n Kernel Virtual OVERSHOOT");
		return NULL;
	}
	pt = cur_VK;
	cur_VK += no_bytes;

	return (void *)pt;
}

// Code for Virtual Memory Free called from within exit
// param: VM address to be freed
// Return: VOID

void free(uint64_t add)
{
	VMA *tmp = running->mm_st;
	VMA *tmp1 = NULL;

	if (tmp->next)
	{
		tmp1 = tmp->next;
	}
	while(tmp1->next)
	{
		if (add == tmp1->start_add)
		{
			tmp->next = tmp1->next;
			tmp1 = NULL;
			break;
		}
		tmp = tmp1;
		tmp1 = tmp1->next;
	}
}

// Kernel Malloc- below kernmem + Physbase Virtual Memory- Uses
// get_page() to get 4Kb physical pages Allocates memory only below
// kernmem + PhysFree - for Processes mapping (sort of brk() equivelent
// param : No of Bytes needed by Process (uint64_t)
// Global: cur_VK; strats from 2MB to Kernmem + Physbase: return: void*
// pointer to the memory needed on success else returns NULL pointer

void *p_malloc(uint64_t no_bytes)
{
	uint64_t pt1 = NULL;

	if ((cur_PK + no_bytes) < (UBASE))
	{
		pt1 = cur_PK;
		cur_PK += no_bytes;
	}
	else
	{
		return (void *) 0;	// Error Condition
	}
	return (void *) pt1;
}

// This function reads the segments from the elf binary and maps them at the
// given V-address backed by Physical Pages which are allocated on the page
// fault. (The page fault handler calls the self refrence() function and maps
// a physical page to faulting Virtual address
// param: start_Vadd elf Segment start Virtual address
// param: source_add
// param: f_size size of the elf segment (bytes)
// param: m_size size of segment in memory (bytes)
// return: 0 on success 1 on error NOTE: ** THIS FUNCTION SHOULD
// ONLY BE CALLED WHEN p_malloc() call before is successful **


uint32_t m_map(uint64_t start_Vadd, uint64_t source_add,
	uint64_t f_size, uint64_t m_size)
{
	char *check = NULL, *source = NULL;
	uint64_t i = 0;
	if (f_size < 1)
	{
		return 1;
	}

	check = (char *) start_Vadd;
	source = (char *) source_add;

	for (i = 0; i < m_size; i++)
	{
		if ( i > f_size && i < m_size)
		{
			*check++ = 0;
		}
		else
		{
			*check++ = *source++;
		}
	}

	return 0;
}

/*
 * Maps the kernel Page Tables in Process Page Table by copying kernel PML4E 511 entry into Process Page Table 
 * Also creates Page Table for the new process 
 * returns: uint64_t base Physical Address of the PML4e page table for the new process 
 */

uint64_t map_pageTable(PCB *pb)
{
	uint64_t *p1, *tmp, *tmp1;
	uint64_t add = 0xFFFF000000000000;	// Base address to build on
	p1 = (uint64_t *) get_page();
	uint32_t i = 0;

	if (p1 == NULL)
	{
		return NULL;
	}	

	/* Looking into kernel page table */	
	add = (((add >> 48) << 9 | 0x1FE) << 39);  // Sets the 1st 9 bits to 510 for  self refrenccing
	add = (((add >> 39) << 9 | 0x1FE) << 30);  // Sets the 2nd 9 bits to 510 for  self refrenccing
	add = (((add >> 30) << 9 | 0x1FE) << 21);  // Sets the 3rd 9 bits to 510 for  self refrenccing
	add = (((add >> 21) << 9 | 0x1FE) << 12);  // Sets the 4th 9 bits to 510 for  self refrenccing
	tmp1 = (uint64_t *) add;

	/* In case when an existing process forks a new one */
	i = 509;
	while ((*(tmp1+i)) != 0x0)
	{
		i--;
	}	
	(*(tmp1 + i)) = ((uint64_t) p1 | 7);	// Pml4e for new forked process

	add = 0xFFFF000000000000;
	add = (((add >> 48) << 9 | 0x1FE) << 39);  // Sets the 1st 9 bits to 510 for  self refrenccing
	add = (((add >> 39) << 9 | 0x1FE) << 30);  // Sets the 2nd 9 bits to 510 for  self refrenccing
	add = (((add >> 30) << 9 | 0x1FE) << 21);  // Sets the 3rd 9 bits to 510 for  self refrenccing
	add = (((add >> 21) << 9 | i) << 12);  // Sets the 4th 9 bits to 509 to point to the extra page used to init the new process page table
	tmp = (uint64_t *) add;

	*(tmp + 511) = (uint64_t)tmp1[511];	   // Mapping Kernel PML4e entry into process 
	*(tmp + 510) = (((uint64_t) p1) | 7);	  // Self Refrencing Trick	
	pb->index = i;				// Setting the index in PML4E of parent	
	return (uint64_t) p1;			   // returns the PML4E base Physical address
}
/**********************************************************************/
/*
 * Helper function to get PT indexes
 */
int getPTEindex(uint64_t vadd)
{
	return ((vadd << (16+9+9+9)) >> 55); 
}

int getPDEindex(uint64_t vadd)
{
	return ((vadd << (16+9+9)) >> 55); 
}

int getPDPEindex(uint64_t vadd)
{
	return ((vadd << (16+9)) >> 55); 
}

int getPML4Eindex(uint64_t vadd)
{
	return ((vadd << 16) >> 55); 
}


/**********************************************************************/
/*
 * Creates User mode dynamic stack for the process through Page Faults and Self Refrencing
 * Returns: Pointer where the stack starts
 * NOTE ** Stack grows downwards i.e.from high address to low addresses
 */
// abhi make it dynamic
uint64_t *process_stack()
{
	uint64_t *st = NULL;
	uint64_t *top = NULL;
	uint32_t i = 0;
	while (i < 8)
	{
		st = (uint64_t *) p_malloc(4096);
		i++;

		if (st == NULL)
		{
			printf("\n Cant allocate Process Stack");
			//exit();
			return (void *)0;
		}
	}
	top = (uint64_t *) (st + 512); 
	//	top = (uint64_t *) uStack_Top;
	return top;	//returns the top of the virtual page 4KB page as stack grows downwards
}	
/*
 * Gets the Physical address from Virtual Address passed as argument
 * return: physical address
 */

uint64_t ptov_map(uint64_t vadd)
{
	int pml4eindex = getPML4Eindex(vadd);
	int pdpeindex = getPDPEindex(vadd);
	int pdeindex = getPDEindex(vadd);
	int pteindex = getPTEindex(vadd);

	uint64_t *pml4eAdd, *pdpeAdd, *pdeAdd, *pteAdd;

	pml4eAdd   = (uint64_t *)(selfRef(0x1FE, 0x1FE, 0x1FE, 0x1FE));
	pdpeAdd    = (uint64_t *)(selfRef(0x1FE, 0x1FE, 0x1FE, pml4eindex));
	pdeAdd     = (uint64_t *)(selfRef(0x1FE, 0x1FE, pml4eindex, pdpeindex));
	pteAdd     = (uint64_t *)(selfRef(0x1FE, pml4eindex, pdpeindex, pdeindex));
	if(pml4eAdd[pml4eindex]==0)
		return 0;
	if(pdpeAdd[pdpeindex]==0)
		return 0;
	if(pdeAdd[pdeindex]==0)
		return 0;
	if(pteAdd[pteindex]==0)
		return 0;
	return ((uint64_t)(pteAdd[pteindex])); 
}



void init_shell()	// sort of execve in current scenario
{
	PCB *pro = NULL;
	VMA *tm = NULL;
	char *name = "bin/shell";
	pro = create_pcb();
	pro->cr3 = map_pageTable(pro);
	if (!(pro->pid = 0)) //= get_Newpid()) == 0)
	{
		//		printf("\n Error No Free PID found");	// abhi abort ?
		//	return (PCB *) 0;
	}
	pro->ppid = 0;

	_set_k_ptable_cr3(pro->cr3);
	read_tarfs(pro, name);
	//	printf("\n BACK IN TEST");
	if ((pro->u_stack = process_stack()) == NULL)
	{
		//		printf("\n Cant allocate memory for process User stack");
		//exit();
	}
	/* Adding a new VMA for process stack */
	tm = create_vma(((uint64_t)(UBASE)),((uint64_t) (8 * 4096)));
	tm->next = pro->mm_st;
	pro->mm_st = tm;
	/******************************************/
	/* Setting the CR3 with the new process PML4E */	
	//	printf("\n CR3 set done");
	/* Running new process */
	if ((add_toQ(allPro, pro)))
	{
		//		printf("\n Eroor in adding to list");
	}

	add_toQ(allPro, pro);		//Adding process to the all process list
	//	add_toQ(runableQ, pro);
	running = pro;	// Pointer which keeps track of currently running process	


	pro->u_stack[0] = pro->rip;
	pro->rsp = (uint64_t)(pro->u_stack);
	tss.rsp0 = (uint64_t)  &(pro->kernel_stack[255]);
	//	printf("\n GDT SET");
	/*  Put this in seprate function for ring switch */ 
	uint64_t tem = 0x28; 
	__asm volatile("mov %0,%%rax;"::"r"(tem));
	__asm volatile("ltr %ax");
	__asm volatile("\
			push $0x23;\
			push %0;\
			pushf;\
			push $0x1B;\
			push %1"::"g"((pro->u_stack)),"g"(pro->rip):"memory");
	__asm volatile("\
			iretq;\
			");
} 
