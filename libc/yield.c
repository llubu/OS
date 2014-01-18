# include <libc/malloc.h>
# include <stdio.h>
# include <syscall.h>

/*
* User level wrapper for schedule() system call
*
*/

/*
* Tells processes I wont to leave, schedule some other process
*/
void yield()
{
	u_printf("\n IN YIELD");	
	__syscall0(SYSCALL_SCHD);

}
	
