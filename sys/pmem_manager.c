#include <defs.h>
#include <stdio.h>

#include <sys/pmem_manager.h>

#define PAGE_SIZE    (4096u)

Page* head = NULL;    // extern in header file
Page* tail = NULL;    // tail of the free list
Page* ck= NULL;       // To check the first free page allocated

uint64_t i = 0;       // Global varaible
uint64_t vm_cast = 0; // Extern in same heder

void zero_page(uint64_t pa)
{
	uint64_t* tmp = (uint64_t*)pa;
	uint32_t i = 0;

	while (i < 512)
	{
		*tmp = 0;
		tmp++;
		++i;
	}
}

void free_page_list(struct smap_t* sp, void* physbase, void* physfree)
{
	uint64_t sm;
	uint64_t* kern_free = (uint64_t*)physfree;
	Page* tmp  = NULL;

	if (!sp->base)
		return;

	if (!head) // if free list is empty
	{
		// free list will be part of kernel memory space
		head = (Page*)kern_free;
		// tail of the free list
		tail = head;
		// it will store address for first free page in memory
		head->page_base = NULL;
		head->next = NULL;
	}

	i = 0;

	for (sm = sp->base; sm < (sp->base + sp->length); sm += PAGE_SIZE)
	{
		if (1 == i)
			ck = head;
		if (((sm >= (uint64_t)physbase) && (sm <=
			((uint64_t)physfree + (1024 * 1024))))
			|| ((sm >= 0xb8000) && (sm <= 0xbc096))
			|| (sm < (uint64_t)physbase))
		{
			// Taking care of kernel in memory and
			// Video memory (does not map them in free list)
			continue;
		}
		i++;
		// expanding physfree by 128 Bits
		// (memory for two 64bit pointers in each link list node)
		kern_free += 2;
		tmp = (Page*)kern_free;
		tmp->page_base = sm; // Page boundry on 4 Kb
		zero_page(sm);
		tmp->next = head;
		head = tmp;
	}

	printf("\n Kern_Free:%x:HEAD%x:Next%x:sm%x:i%d:\n%x", kern_free,
		head, head->next, sm, i, physfree+(i*16));
	printf("\nFirstFreePAge:%x:ckNext:%x", ck, ck->page_base);
}

void* get_page()
{
	Page* vm_tmp = NULL;
	Page* tail_tmp = NULL;
	void* pt = NULL;

	// This is done as free link list is refrenced through
	// physical 1:1 mapping before 1st CR3 is set..
	vm_tmp = (Page*)((uint64_t)head | vm_cast);
	tail_tmp = (Page*)((uint64_t)tail | vm_cast);

	if (tail_tmp == vm_tmp)
	{
		printf("\n OUT OF PHYSICAL MEMORY!!!");
		return (void*)0;
	}

	// So each time head is derefrenced we need to cast
	// it to kernmem to get the current virtual mapping.
	pt = (void*)vm_tmp->page_base;
	head = vm_tmp->next;
	return pt;
}

void free_page(void* pt)
{
	Page* tmp = NULL;
	tmp->page_base = (uint64_t)pt;
	tmp->next = head;
	head = tmp;
}

