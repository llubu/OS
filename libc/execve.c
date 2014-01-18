/*
* Execve function for the user space
*
*/

# include <syscall.h>


void execve(char *filename)
{

//	 __syscall3(SYSCALL_EXECVE,(uint64_t)filename, (uint64_t)argv, (uint64_t)envp);
  	 __syscall1(SYSCALL_EXECVE, (uint64_t) filename);


	
}


