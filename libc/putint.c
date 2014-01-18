#include <stdio.h>


uint64_t sys_putint(int a)
{
    __asm__ volatile("int $80;"::"a"((uint64_t)a), "b" ((uint64_t)1));
    return 0;
}
