# include <stdio.h>
# include <libc/malloc.h>
# include <syscall.h>

void ps()
{
	__syscall0(SYSCALL_PS);
}

uint32_t getpid()
{
	uint32_t ret = 0;
	ret = __syscall0(SYSCALL_GETPID);
	return ret;
}

void exit(int status)
{
	__syscall1(SYSCALL_EXIT,(uint64_t) status);
}	
