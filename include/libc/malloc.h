#ifndef _MALLOC_H
#define _MALLOC_H

# include <defs.h>

void *malloc(uint64_t no_bytes);	// User space malloc implementation
uint64_t fork();			// User space fork wrapper
void execve(char *filename);
void yield();
void ps();
#endif
