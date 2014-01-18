/* Implements process management  */

#include <stdio.h>
#include <defs.h>
//#include <sys/tasking.h>
//#include <sys/page_table.h>
//#include <sys/phy_mem.h>
#include <sys/gdt.h>
#include <sys/syscall.h>

uint64_t syscalls[32] = {0,(uint64_t)&printf};//, (uint64_t)&putchar, (uint64_t)&puts};
//syscalls[1] = 100;// (uint64_t)&putint;
//syscalls[2] = (uint64_t)&putchar;
//syscalls[3] = (uint64_t)&puts;

uint64_t sys_putint(int a)
{
//  int ret = 0;
/*  __asm volatile(
      "movq %0,%%rax;"
      :
      :"a"((uint64_t)a)
      :"memory"
      );
  __asm volatile(
      "movq %0,%%rbx;"
      :
      :"b"((uint64_t)1)
      :"memory"
      );*/
  __asm__ volatile("int $80;"::"a"((uint64_t)a), "b" ((uint64_t)1));
  return 0;
}
/*
uint64_t sys_putchar(char a)
{
//  int ret = 0;

  __asm volatile(
      "movq %0,%%rax;"
      :
      :"r"((uint64_t)a)
      );
  __asm volatile(
      "movq %0,%%rbx;"
      :
      :"r"((uint64_t)2)
      );
  __asm__ volatile("int $80;");
  
  return 0;
     
}

uint64_t sys_puts(char* a)
{
//  int ret = 0;

  __asm volatile(
      "movq %0,%%rax;"
      :
      :"r"((uint64_t)a)
      );
  __asm volatile(
      "movq %0,%%rbx;"
      :
      :"r"((uint64_t)3)
      );
  __asm__ volatile("int $80;");
  
  return 0;
}
*/
