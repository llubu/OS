#ifndef _TASK_MANAGEMENT_H
#define _TASK_MANAGEMENT_H

#include <defs.h>
#define MAXPID 32  //32767	

// need to implement pid bitmap
//uint16_t pid_bitmap[32];
/* VMA Structure Definition */
struct vma_t
{
	uint64_t  start_add;  // Start address of a segment of elf binary
	uint64_t end_add;     // End address (First Address after end of VMA)	
	struct vma_t *next;
	// abhi check what else to add
};
typedef struct vma_t VMA;

/* PCB Structure definition */
struct pcb_t
{
	uint64_t pid;	// Process ID
	uint64_t ppid;	// Process Parent ID
	uint64_t index; // index where PML4E was mapped to in the parent process PML4E
	VMA *mm_st;	// Pointer to First VMA Block of the Process 
	uint64_t cr3;	// Coontents of CR3 Register. (add of PML4E page table)
	uint64_t *u_stack;
	uint64_t rsp;
	uint64_t rip;	// RIP register pointer
	uint64_t load_time;
	char name[64];
  	uint64_t sleep_init;	
	uint64_t sleep_tm;	
	uint64_t kernel_stack[256];	//abhi check for the size
};

typedef struct pcb_t PCB;

struct pro_pt
{
	struct pro_pt *next;			
	struct pro_pt *prev;			
	struct pro_pt *tail;	// Points to end of the lists(only used in the head)
	uint32_t count;
	PCB *pcb_li;	// pointer to the PCB for a process
};

typedef struct pro_pt PLIST;

extern PLIST *allPro;		// Queue for all processes present in the system
extern PLIST *waitQ;		// Queue for processes in wait state
extern PLIST *runableQ;		// Queue for processes READY to run

PCB *running;			// Pointer to PCB of the process currently running
PCB *idle;			// Pointer to idle Process
PCB *switchTo;			// Pointer to process to switch to

void init_task();	// Initializez the process management
uint32_t get_Newpid();	// Returns a free PID from the pid bitmap
uint32_t get_pid();	//returns current pid for the process running
PCB *create_pcb();	// DO initial Setup on a new process creation
VMA *create_vma(uint64_t start, uint64_t size);	// Creates VMA structure for each segment 

uint32_t get_pid(); 	//returns the PID of the process 
uint64_t doFork();

int add_toQ(PLIST *list, PCB *pc);
PCB *get_nextProcess(PLIST *list);	//returns the next process in the list 

int copyUST(uint64_t *src, PCB *p);	//copies the parent user space PT to child
void copyOnWritePageTables();		// Copy on write handler
uint64_t selfRef(uint64_t p4, uint64_t p3, uint64_t p2, uint64_t p1);
void deletePageTables();

#endif
