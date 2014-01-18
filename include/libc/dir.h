#include <defs.h>
#include <stdio.h>
#include <syscall.h>
#include <sys/gdt.h>
#include <sys/idt.h>
#include <sys/isr.h>
#include <sys/irq.h>
#include <sys/pmem_manager.h>
#include <sys/kern_page_table.h>
#include <sys/tarfs.h>
#include <sys/dirent.h>
#include <sys/string.h>
#include <sys/errno.h>
#include <sys/v_mem_manager.h>
#include <libc/malloc.h>

struct dirent* u_readdir(DIR* directory);
DIR* u_opendir(char* str);
uint64_t u_closedir(DIR* directory);
uint64_t open(char* str);
uint64_t close(uint64_t fd);
uint64_t read(uint64_t fd,uint64_t num, char* str );
