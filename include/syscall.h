#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <defs.h>

#define SYSCALL_PROTO(n) static __inline uint64_t __syscall##n
#define SYSCALL_PUTINT 1
#define SYSCALL_PUTS 2 
#define SYSCALL_FORK 3 
#define SYSCALL_MALLOC 4
#define SYSCALL_READ 5
#define SYSCALL_SCHD 6
#define SYSCALL_EXECVE 7
#define SYSCALL_EXIT 8
#define SYSCALL_WAITPID 9
#define SYSCALL_WAIT 10
#define SYSCALL_READDIR 11
#define SYSCALL_OPENDIR 12
#define SYSCALL_CLOSEDIR 13
#define SYSCALL_STDERR 14
#define SYSCALL_OPEN 15
#define SYSCALL_CLOSE 16
#define SYSCALL_PS 17
#define SYSCALL_GETPID 18


SYSCALL_PROTO(0)(uint64_t n) 
{
	uint64_t ret;
   	__asm volatile( "movq %1,%%rax\n\t"
                   "int $80\n\t"
                   "movq %%rax,%0\n\t;"
                   :"=r"(ret):"r"(n):"rax","memory");
	return ret;
}

SYSCALL_PROTO(1)(uint64_t n, uint64_t a1) 
{
	uint64_t ret;
   	__asm volatile( "movq %1,%%rax\n\t"
                   "movq %2,%%rbx\n\t"
                   "int $80\n\t"
                   "movq %%rax,%0\n\t;"
                   :"=r"(ret):"r"(n),"r"(a1):"rax","rbx","memory");
	return ret;
}

SYSCALL_PROTO(2)(uint64_t n, uint64_t a1, uint64_t a2) 
{
 	 uint64_t ret;
   	__asm volatile( "movq %1,%%rax\n\t"
                   "movq %2,%%rbx\n\t"
                   "movq %3,%%rcx\n\t"
                   "int $80\n\t"
                   "movq %%rax,%0\n\t;"
                   :"=r"(ret):"r"(n),"r"(a1),"r"(a2):"rax","rbx","rcx","memory");
	return ret;
}

SYSCALL_PROTO(3)(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3) 
{
	uint64_t ret;
   	__asm volatile( "movq %1,%%rax\n\t"
                   "movq %2,%%rbx\n\t"
                   "movq %3,%%rcx\n\t"
                   "movq %4,%%rdx\n\t"
                   "int $80\n\t"
                   "movq %%rax,%0\n\t;"
                   :"=r"(ret):"r"(n),"r"(a1),"r"(a2),"r"(a3):"rax","rbx","rcx","rdx","memory");
	return ret;
	return 0;
}

SYSCALL_PROTO(4)(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
	return 0;
}

#endif
