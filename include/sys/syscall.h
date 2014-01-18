#ifndef _SYSCALL_H
#define _SYSCALL_H

#include<defs.h>
uint64_t sys_putint(int a);
uint64_t sys_putchar(char a);
uint64_t sys_puts(char* a);
extern uint64_t syscalls[32];

#endif
