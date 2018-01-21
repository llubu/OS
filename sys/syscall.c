#include <stdio.h>
#include <defs.h>

#include <sys/gdt.h>
#include <sys/syscall.h>

uint64_t syscalls[32] =
{
	0, (uint64_t)&printf
};

uint64_t sys_putint(int a)
{
	__asm__ volatile("int $80;"::"a"((uint64_t)a), "b" ((uint64_t)1));
	return 0;
}

