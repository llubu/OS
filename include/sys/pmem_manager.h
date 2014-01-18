#ifndef _PMEM_MANAGER
#define _PMEM_MANAGER

typedef struct page 		// Structure for a page in physical memory
{
	uint64_t page_base;
	struct page *next;
}Page;

struct smap_t
{
	uint64_t base, length;
	uint32_t type;
}__attribute__((packed)) *smap;

extern Page *head;					// Global definition of head
extern uint64_t vm_cast;				// This cast the free list head to new kern mem
extern uint64_t p_free;					// Global symbol for Physfree
void free_page_list(struct smap_t *sp, void *physbase, void *phyfree);	// base address & end address for free memory.
void * get_page();					// To get a free page 
void free_page(void * pt);				// To return a page to the free list
//void *get_contig_pages();				// To return contiguous free pages
#endif











