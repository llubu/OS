#ifndef _V_MEM_MANAGER
#define _V_MEM_MANAGER

# include <sys/task_management.h>

# define MAX_KERN 0xFFFFFFFFFFFFFFFF
# define BASE 0x200000
# define UBASE 0xFFFFFFFF800F8000	// BASE of User Stack
# define UTOP 0x100000			// TOP OF USER STACk
# define PAGE_SIZE 4096

uint64_t opfree;
extern uint64_t cur_PK;		// Current free Virtual Memory below kernmem + Physfree
extern uint64_t cur_VK;		// Current free Virtual Memory above Physfree;
extern uint64_t uStack_Top;	// Top of User stack

void *k_malloc(uint64_t no);	// VM allocator
void *p_malloc(uint64_t no);	// VM allocator for Process space brk() 

/* maps elf binaries segments into Virtual Memory*/
uint32_t m_map(uint64_t start, uint64_t source, uint64_t f_size, uint64_t m_size);

/* creates and map Process page table (only PML4E) */
uint64_t map_pageTable(PCB *);

/* Creates Process Stack */
uint64_t *process_stack();	

/* Clone system call - user space wrapper fork() */
void clone();

void init_VM(uint64_t pfree);		// Initializes the Virtual Memory Manager
void free(uint64_t);
void init_shell();


struct ptab
{
	uint64_t p[512];	
};

typedef struct ptab PTAB;
extern PTAB *popo;
uint64_t ptov_map(uint64_t vadd);	//physical to virtual address mapping returned
#endif
