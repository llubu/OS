# include <stdio.h>

# include <sys/task_management.h>
# include <sys/v_mem_manager.h>
# include <sys/pmem_manager.h>
# include <sys/task_switch.h>
# include <sys/tarfs.h>
# include <sys/gdt.h>
# include <sys/string.h>
# include <sys/dirent.h>
# include <sys/dirent.h>
# include <sys/irq.h>

uint64_t pid_bitmap[32] = {0};
extern void _set_k_ptable_cr3(uint64_t);
PLIST* allPro;
PLIST* waitQ;
PLIST* runableQ;

#define R0    (0xfffffffffffffffd) // rw bit ,will be set to readonly
#define COW   (0x0008000000000000) // COW BIt handled in software

uint32_t get_Newpid()
{
	uint32_t i = 1;

	while (pid_bitmap[i++] != 0 && i < MAXPID);

	if (MAXPID < i && pid_bitmap[MAXPID] != 0)
		return 0;
	pid_bitmap[i-1] = 1;
	return (i-1);
}

PCB* create_pcb()
{
	PCB* pro = NULL;

	pro = (PCB*) k_malloc(sizeof(PCB));
	if (pro == NULL)
		return (PCB *) 0;
	return pro;
}

VMA* create_vma(uint64_t start_add, uint64_t size)
{
	VMA* vm = NULL;

	vm = k_malloc(sizeof(VMA));

	if (!vm)
	{
		//exit();
		printf("\n Cant allocalte memory for VMA exit now");
	}
	vm->start_add = start_add;
	vm->end_add = (start_add + size);
	vm->next = NULL;

	return vm;
}

int add_toQ(PLIST* list, PCB* pc)
{
	PLIST* node = NULL;

	if((node = k_malloc(sizeof(PLIST))) == NULL)
	{
		//		printf("\n Cant Allocate Memory for PLIST node");
		return 1;
	}

	node->pcb_li = pc;
	node->next = NULL;
	if (list->next == NULL)
		list->next = node;
	else
		list->tail->next = node;
	list->tail = node;
	list->count += 1;	//Increasing count of process in this list
	return 0;
}

void init_task()
{
	if ((allPro = k_malloc(sizeof(PLIST))) == NULL)
	{
		//		printf("\n Cant allocate mem for allpro");
		//exit();
	}
	else
	{
		allPro->next = NULL;
		allPro->prev = NULL;
		allPro->tail = NULL;
		allPro->count = 0;
		allPro->pcb_li = NULL;
	}
	/* Initializing waitQ list */
	if ((waitQ = k_malloc(sizeof(PLIST))) == NULL)
	{
		printf("\n Cant allocate mem for waitQ");
		//exit();
	}
	else
	{
		waitQ->next = NULL;
		waitQ->prev = NULL;
		waitQ->tail = NULL;
		waitQ->count = 0;
		waitQ->pcb_li = NULL;
	}

	/* Initializing runableQ */
	if ((runableQ = k_malloc(sizeof(PLIST))) == NULL)
	{
		printf("\n Cant allocate mem for runableQ");
		//exit();
	}
	else
	{
		runableQ->next = NULL;
		runableQ->prev = NULL;
		runableQ->tail = NULL;
		runableQ->count = 0;
		runableQ->pcb_li = NULL;
	}
}

uint32_t get_pid()
{
	return (running->pid);
}

PCB* get_nextProcess(PLIST* list)
{
	PCB *pt = NULL;

	if (!list->next)
		return NULL;		//return Idle process here
	pt = list->next->pcb_li;
	list->next = list->next->next;	//returns from the head
	list->count -= 1;		//decresing count

	return pt;
}

int copyUST( uint64_t* uST, PCB* pg)
{
	uint64_t* dst = (uint64_t*)0xffffffff7fff0000ul;
	uint64_t tmp = 0;
	uint64_t i = 0;
	dst = uST;

	while (i < (512 * 8))
	{
		tmp = 0;
		tmp = *(uST - i);

		_set_k_ptable_cr3(pg->cr3); // child processes page table

		*(dst - i) = tmp;
		i++;
		// Setting back to parents
		_set_k_ptable_cr3(running->cr3);
	}
	pg->u_stack = (uint64_t*)0xffffffff7fff0000ul;
	return 0;
}

uint64_t selfRef(uint64_t pml4e, uint64_t pdpe, uint64_t pde, uint64_t pte)
{
	uint64_t base = 0xffff000000000000ul;

	if (pml4e < 256)
		base = 0ul;

	base = (((base >> (12 + 9 + 9 + 9 + 9)) << 9 | pml4e)
		<< (12 + 9 + 9 + 9));
	base = (((base >> (12 + 9 + 9 + 9)) << 9 | pdpe) << (12 + 9 + 9));
	base = (((base >> (12 + 9 + 9)) << 9 | pde) << (12 + 9));
	base = (((base >> (12 + 9)) << 9 | pte) << (12));

	return base;
}

void copyPageTables(PCB *child, PCB *parent)
{
	uint64_t i = 0;
	uint64_t j = 0;
	uint64_t k = 0;
	uint64_t l = 0;
	volatile uint64_t *pml4eAdd;
	volatile uint64_t *pdpeAdd;
	volatile uint64_t *pdeAdd;
	volatile uint64_t *pteAdd;
	volatile uint64_t *new_pml4eAdd;
	volatile uint64_t *new_pdpeAdd;
	volatile uint64_t *new_pdeAdd;
	volatile uint64_t *new_pteAdd;
	uint64_t child_pml4e_entry;

	child_pml4e_entry = child->index;
	pml4eadd = (uint64_t *)(selfref(0x1fe, 0x1fe, 0x1fe, 0x1fe));
	new_pml4eadd = (uint64_t *)(selfref(0x1fe, 0x1fe, 0x1fe,
		child_pml4e_entry ));

	for (i = 0; i < 510; i++)
	{
		if (i == child_pml4e_entry)
			continue;

		if (pml4eAdd[i]) // if some entry exists we have to copy
		{
			new_pml4eAdd[i] = (((uint64_t)get_page()) | 7);
			pdpeAdd = (uint64_t *)(selfRef(0x1fe, 0x1fe, 0x1fe, i));
			new_pdpeAdd = (uint64_t *)(selfRef(0x1fe, 0x1fe,
				child_pml4e_entry, i));

			for (j = 0; j < 512; j++)
			{
				if (pdpeAdd[j])
				{
					new_pdpeAdd[j] = (((uint64_t)
						get_page()) | 7);
					pdeAdd = (uint64_t*)(selfRef(0x1fe,
						0x1fe, i, j));
					new_pdeAdd = (uint64_t*)(selfRef(0x1fe,
						child_pml4e_entry, i, j));

					for (k = 0; k < 512; k++)
					{
						if(pdeAdd[k])
						{
							new_pdeAdd[k] =
								(((uint64_t)
								  get_page())
								 | 7);
							pteAdd = (uint64_t*)
								(selfRef(0x1fe,
									 i, j, k));
							new_pteAdd = (uint64_t*)
								(selfRef(
								 child_pml4e_entry,
								 i, j, k));

							for (l = 0; l < 512;
									l++)
							{
								if(pteAdd[l])
								{
									new_pteAdd[l] = ((((uint64_t)pteAdd[l]) & R0) | COW);
									pteAdd[l]     = ((((uint64_t)pteAdd[l]) & R0)| COW);
								}
							}
						}
					}
				}
			}
		}
	}
	pml4eAdd[child_pml4e_entry] = 0x0;
}

void deletePageTables()
{
	uint64_t i; // iterators for pml4e, pdpe, pde, pte
	uint64_t *pml4eAdd;
	pml4eAdd = (uint64_t *)(selfref(0x1fe, 0x1fe, 0x1fe, 0x1fe));

	for(i=0; i<510; i++)
		pml4eAdd[i] = 0ul;
}

void copyOnWritePageTables()
{
	volatile uint64_t i = 0;
	volatile uint64_t j = 0;
	volatile uint64_t k = 0;
	volatile uint64_t l = 0;
	volatile uint64_t m = 0;
	volatile uint64_t newindex = 0;
	volatile uint64_t* pml4eAdd;
	volatile uint64_t* pdpeAdd;
	volatile uint64_t* pdeAdd;
	volatile uint64_t* pteAdd;
	volatile uint64_t* copycon;
	volatile uint64_t* new_Add = NULL;
	char content;
	uint64_t a1 = 1;

	printf("IN COW func");
	pml4eadd = (uint64_t *)(selfref(0x1fe, 0x1fe, 0x1fe, 0x1fe));

	for (i = 0; i < 509; i++)
	{
		if (!pml4eAdd[i])
			break;
	}
	a1 = i;

	pml4eAdd[a1] = ((uint64_t)get_page() | 7);
	new_add = (uint64_t *)(selfref(0x1fe, 0x1fe, 0x1fe, a1));

	for (i = 0; i < 510; i++)
	{
		if (i == a1)
			continue;

		if (pml4eAdd[i])
		{
			pdpeAdd =
				(uint64_t*)(selfref(0x1fe, 0x1fe, 0x1fe, i));

			for(j = 0; j < 512; j++)
			{
				if (pdpeAdd[j])
				{
					pdeadd = (uint64_t*)(selfref(0x1fe,
								0x1fe, i, j));
					for(k = 0; k < 512; k++)
					{
						if (pdeAdd[k])
						{
							pteAdd =(uint64_t*)
								(selfref(0x1fe,
									 i, j, k));
							for( l= 0; l < 512;
									l++)
							{
								if (pteAdd[l])
								{
									uint64_t picAdd = (uint64_t )(selfRef(i,j,k,l));
									new_Add[newindex] = ((uint64_t)get_page() | 7);

									copycon = (uint64_t *)(selfRef(0x1fe, 0x1fe,a1,newindex));
									char *src = (char *) picAdd;
									char *dst = (char *) copycon;
									for(m=0;m<4096;m++)
									{
										content = src[m];
										dst[m] = content;
									}
									pteAdd[l] = ((((uint64_t)new_Add[newindex] >>12)<<12) | 7);
									newindex++;
								}
							}
						}
					}

				}
			}
		}
	}
	pml4eAdd[a1] = ((uint64_t)0x0);
}

void cpKS(uint64_t *src, uint64_t *dst)
{
	uint64_t i = 0;

	while (i < 512)
	{
		dst[i] = src[i];
		++i;
	}
}

void ckop()
{
	uint64_t i = 0, j = 0, k = 0, l =0;
	uint64_t *p4 = NULL, *p3 = NULL, *p2 = NULL, *p1 = NULL;

	p4 = (uint64_t *) selfRef(0x1fe, 0x1fe, 0x1fe, 0x1fe);

	for (i = 0; i < 510; i ++)
	{
		if (p4[i])
		{
			p3 = (uint64_t *) selfRef(0x1fe, 0x1fe, 0x1fe, i);
			for (j = 0; j < 512; j++)
			{
				if (p3[j])
				{
					p2 = (uint64_t *) selfRef(0x1fe, 0x1fe, i, j);
					for (k = 0;k < 512; k++)
					{
						if (p2[k])
						{
							p1 = (uint64_t *) selfRef(0x1fe, i, j, k);
							for (l = 0; l< 512; l++)
							{
								if (p1[l])
								{
									printf("\n PTE:%d:%d:%d:%d:%x", i,j,k,l,p1[l]);
								}
							}
						}
					}
				}
			}
		}
	}
}

uint64_t doFork()
{
	PCB *pro = NULL;
	PCB *parent_process;
	uint64_t pid, k_cid = 234, k_pid = 234;
	parent_process = running;
	pro = create_pcb();
	m_map((uint64_t)pro, (uint64_t) parent_process, (uint64_t)(sizeof(struct pcb_t)), (uint64_t)(sizeof(struct pcb_t)) );
	cpKS((running->kernel_stack), (pro->kernel_stack));

	pro->cr3 = map_pageTable(pro);	// Storing Base Physical address of PML4e for new process

	pro->ppid = parent_process->pid;
	if ((pro->pid = get_Newpid()) == 0)
	{
		//		printf("\n Error No Free PID found");	// abhi abort ?
		//		return -1;
	}
	pid = pro->pid;	
	//copy the page tables of parent process !!
	copyPageTables(pro, parent_process);
	_set_k_ptable_cr3(pro->cr3);		//flush TLB
	//	copyOnWritePageTables();
	//	printf("\n abhi");
	//	ckop();
	//   _set_k_ptable_cr3(pro->cr3);
	//	printf("\nabhi");
	//	ckop();
	//  _set_k_ptable_cr3(running->cr3);
	add_toQ(allPro, pro);		// Add process to All Process list
	add_toQ(runableQ, pro);
	/* Hack for changing kernel stack values, should work */
	if (pro->kernel_stack[255] == 0x0)
	{
		if (pro->kernel_stack[254] != 0x0 && pro->kernel_stack[254] == 0x23)	//somethig extra was pushed on kernel stack last time adjust index
		{
			k_cid = 235;
		}
	}
	else if (pro->kernel_stack[255] == 0x23)
	{
		k_cid = 236;
	}

	if (running->kernel_stack[255] == 0x0)
	{
		if (running->kernel_stack[254] != 0x0 && running->kernel_stack[254] == 0x23)	//somethig extra was pushed on kernel stack last time adjust index
		{
			k_pid = 235;
		}
	}
	else if (running->kernel_stack[255] == 0x23)
	{
		k_pid = 236;
	}
	// Setting up the rax for both parent and child
	GREG *pp1 = (GREG *) &(running->kernel_stack[k_pid]);
	GREG *cc1 = (GREG *) &(pro->kernel_stack[k_cid]);

	pp1->rax = pro->pid;	// parent return value
	cc1->rax = 0x0;		// chaild return value


	//Add parent_pid in the structure as well
	if (pid == running->pid)
		pid = 0;
	return pid;
}

