/*
* Code for Task/Process Management
*/

# include <sys/task_management.h>
# include <sys/v_mem_manager.h>
# include <sys/pmem_manager.h>
# include <stdio.h>
# include <sys/task_switch.h>
# include <sys/tarfs.h>
# include <sys/gdt.h>
# include <sys/string.h>
# include <sys/dirent.h>
# include <sys/dirent.h>
# include <sys/irq.h>

uint64_t pid_bitmap[32] = {0};
extern void _set_k_ptable_cr3(uint64_t);
PLIST *allPro;
PLIST *waitQ;
PLIST *runableQ;

#define  R0 0xFFFFFFFFFFFFFFFD // RW bit ,will be set to Readonly
#define COW 0x0008000000000000 // COW BIt handled in software

/*
* Returns a free pid (32 bit unsigned integer for the new process)
*/
uint32_t get_Newpid()
{
	uint32_t i = 1;
	while (pid_bitmap[i++] != 0 && i < MAXPID);	// abhi convert that to bitmap with bit manipulation
	if (MAXPID < i && pid_bitmap[MAXPID] != 0)
	{
		return 0;				// 0 in this case shows error that is no free PID found abhi what to do in this case
	}
	pid_bitmap[i-1] = 1;
	return (i-1);
}

/*
* This function creates the PCB for each new process created
*
*
*/

PCB *create_pcb()
{
	PCB *pro = NULL;

	pro = (PCB *) k_malloc(sizeof(PCB));
//	pro = (PCB *) 0xfffffffffff00000;
	if (pro == NULL)
	{
//		printf("\n Can't Allocate Memory for PCB");
		return (PCB *) 0;
	}
	
	return pro;
}		
		
/*
* This code creates the VMA for each segments of the elf binary
* param: startaddress, size
* Return: VMA* structure pointer
*/

VMA *create_vma(uint64_t start_add, uint64_t size)
{
	VMA *vm = NULL;

	vm = k_malloc(sizeof(VMA));
	if (vm == NULL)
	{
		//exit();
//		printf("\n Cant allocalte memory for VMA exit now");
	}
	vm->start_add = start_add;
	vm->end_add = (start_add + size);
	vm->next = NULL;

	return vm;
}

/*
* Adds a given PCB to the given Process List in the end
* 
*/

int add_toQ(PLIST *list, PCB* pc)
{
	PLIST *node = NULL;

	if((node = k_malloc(sizeof(PLIST))) == NULL)
	{
//		printf("\n Cant Allocate Memory for PLIST node");
		return 1;
	}

	node->pcb_li = pc;
	node->next = NULL;
	if (list->next == NULL)
	{
		list->next = node;
	}
	else
	{
		list->tail->next = node;
	}	
	list->tail = node;
	list->count += 1;	//Increasing count of process in this list
	return 0;
}

/*
* Initializes the data structures for process management
*/

void init_task()
{
	/* Initializing allPro list */
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


/*
* Function to get the pid of the current process
* param: PCB* structure pointer
* return: uint32_t ineger for process's PID
*/

uint32_t get_pid()
{
	return (running->pid);		
}


/*
* Returns the next available process in runableQ from head
* return: PCB * pointer
*
*/

PCB * get_nextProcess(PLIST *list)
{
	PCB *pt = NULL;
	
	if ((list->next == NULL))
	{
//		printf("\n Some Error NO Processes on runableQ to schedule returning IDLE process");
		return NULL;		//return Idle process here
	}	
	pt = list->next->pcb_li;
	list->next = list->next->next;	//returns from the head
	list->count -= 1;		//decresing count

	return pt;
}
/*
* Copies User page table when a process does fork
*/

int copyUST( uint64_t *uST, PCB *pg)
{
	uint64_t *dst = (uint64_t *)0xFFFFFFFF7FFF0000;
	uint64_t tmp = 0;
	uint64_t i = 0;
	dst = uST;
	
	while (i < (512 * 8))
	{
		tmp = 0;
		tmp = *(uST - i);
	
		_set_k_ptable_cr3(pg->cr3);			// child processes page table

		*(dst - i) = tmp;
		i++;
		_set_k_ptable_cr3(running->cr3);		// Setting back to parents	
	}
	pg->u_stack = (uint64_t *) 0xFFFFFFFF7FFF0000;
	return 0;
}

uint64_t selfRef(uint64_t pml4e, uint64_t pdpe, uint64_t pde, uint64_t pte)
{
	uint64_t base;
	base = 0xFFFF000000000000; 
	if (pml4e < 256)
	{
		base = 0x0000000000000000;
	}	
	base = (((base >> (12+9+9+9+9))<<9 | pml4e ) << (12+9+9+9) );
        base = (((base >> (12+9+9+9))<<9   | pdpe  ) << (12+9+9) );
	base = (((base >> (12+9+9))<<9     | pde   ) << (12+9) );
	base = (((base >> (12+9))<<9       | pte   ) << (12) );
	return base;
}


/****************************************************************/

void copyPageTables(PCB *child, PCB *parent)
{
	uint64_t i = 0, j = 0, k = 0, l = 0; // iterators for pml4e, pdpe, pde, pte
	volatile uint64_t *pml4eAdd, *pdpeAdd, *pdeAdd, *pteAdd;
	volatile  uint64_t *new_pml4eAdd, *new_pdpeAdd, *new_pdeAdd, *new_pteAdd;
  	uint64_t child_pml4e_entry;

	child_pml4e_entry = child->index;
	//printf("IN COPY PAGE TABLE");
  	pml4eAdd = (uint64_t *)(selfRef(0x1FE, 0x1FE, 0x1FE, 0x1FE));
  	new_pml4eAdd = (uint64_t *)(selfRef(0x1FE, 0x1FE, 0x1FE, child_pml4e_entry ));

  	for(i=0; i<510; i++)
  	{
    		if( i==child_pml4e_entry)
      		continue;

    	if(pml4eAdd[i]!=0x0) // if some entry exists we have to copy
    	{
		new_pml4eAdd[i] = (((uint64_t) get_page()) | 7);
      		pdpeAdd = (uint64_t *)(selfRef(0x1FE, 0x1FE, 0x1FE, i));
      		new_pdpeAdd = (uint64_t *)(selfRef(0x1FE, 0x1FE, child_pml4e_entry, i));
      
      		for( j=0; j<512; j++)
      		{
        		if(pdpeAdd[j])
        		{
	  			new_pdpeAdd[j] = (((uint64_t) get_page()) | 7);
          			pdeAdd = (uint64_t *)(selfRef(0x1FE, 0x1FE, i, j));
          			new_pdeAdd = (uint64_t *)(selfRef(0x1FE, child_pml4e_entry, i, j));

          			for( k=0; k<512; k++)
          			{
            				if(pdeAdd[k])
            				{
	            				new_pdeAdd[k] = (((uint64_t) get_page()) | 7);
              					pteAdd = (uint64_t *)(selfRef(0x1FE, i, j, k));
              					new_pteAdd = (uint64_t *)(selfRef(child_pml4e_entry, i, j, k));

              					for(l=0; l<512; l++)
              					{
                					if(pteAdd[l])
                					{
                  						new_pteAdd[l] = ((((uint64_t)pteAdd[l]) & R0) | COW);
                  						pteAdd[l]     = ((((uint64_t)pteAdd[l]) & R0)| COW);
                  			//			total_count++;
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
  pml4eAdd = (uint64_t *)(selfRef(0x1FE, 0x1FE, 0x1FE, 0x1FE));
  // Iterate through all the page table entries in the PML4E of the Parent process 
  for(i=0; i<510; i++)
	pml4eAdd[i] = ((uint64_t)0x0000000000000000) ;
}
/*******************************************************************************************/

void copyOnWritePageTables()
{
  volatile uint64_t i=0, j=0, k=0, l=0, m=0, newindex = 0; 
  volatile uint64_t *pml4eAdd, *pdpeAdd, *pdeAdd, *pteAdd, *copycon;
  volatile uint64_t *new_Add=NULL;
  char content;		//content to be copied

	printf("IN COW func");
  	pml4eAdd = (uint64_t *)(selfRef(0x1FE, 0x1FE, 0x1FE, 0x1FE));
  	for(i=0; i<509; i++)
  	{
    		if(pml4eAdd[i] == 0x0) // if some entry exists we have to copy
    		{
      			break;
    		}
  	}
  	uint64_t a1=1;
  	a1 = i;

 	pml4eAdd[a1] = ((uint64_t)get_page() | 7);
  	new_Add = (uint64_t *)(selfRef(0x1FE, 0x1FE, 0x1FE, a1));
  
  	for(i=0; i<510; i++)
  	{
    		if(i==a1)
		{
      			continue;
		}
    		if(pml4eAdd[i] != 0x0) 
    		{
      			pdpeAdd = (uint64_t *)(selfRef(0x1FE, 0x1FE, 0x1FE, i));
      			for( j=0; j<512; j++)
      			{
        			if(pdpeAdd[j] != 0x0)
        			{ 
          				pdeAdd = (uint64_t *)(selfRef(0x1FE, 0x1FE, i, j));
          				for( k=0; k<512; k++)
          				{
            					if(pdeAdd[k] != 0x0)
            					{
              						pteAdd = (uint64_t *)(selfRef(0x1FE, i, j, k));
              						for(l=0; l<512; l++)
              						{
                						if(pteAdd[l])
                						{
		  							uint64_t picAdd = (uint64_t )(selfRef(i,j,k,l));
		  							new_Add[newindex] = ((uint64_t)get_page() | 7);
		  
		  							copycon = (uint64_t *)(selfRef(0x1FE, 0x1FE,a1,newindex));
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


/*******************************************************************************************/

/****************************************************************************************************************************/

/* Copy Kernel Stack function */
void cpKS(uint64_t *src, uint64_t *dst)
{
	uint64_t i = 0;

	while (i < 512)
	{
		dst[i] = src[i];
		++i;
	}	
}	

/***************************************************************************************/




/*****************************************************************************************************************************/
/*
* test function to check the pte entries content in pte table
*/
void ckop()
{
	uint64_t i = 0, j = 0, k = 0, l =0;
	uint64_t *p4 = NULL, *p3 = NULL, *p2 = NULL, *p1 = NULL;

	p4 = (uint64_t *) selfRef(0x1FE, 0x1FE, 0x1FE, 0x1FE);
	
	for (i = 0; i < 510; i ++)
	{
		if (p4[i])
		{
			p3 = (uint64_t *) selfRef(0x1FE, 0x1FE, 0x1FE, i);
			for (j = 0; j < 512; j++)
			{
				if (p3[j])
				{
					p2 = (uint64_t *) selfRef(0x1FE, 0x1FE, i, j);
					for (k = 0;k < 512; k++)
					{
						if (p2[k])
						{
							p1 = (uint64_t *) selfRef(0x1FE, i, j, k);
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
/*****************************************************************************************************************************/

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

