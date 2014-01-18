# include <sys/idt.h>
# include <sys/isr.h>
# include <stdio.h>
# include <sys/kern_page_table.h>
# include <sys/task_management.h>
# include <sys/v_mem_manager.h>
# include <sys/execve.h>
# include <sys/task_switch.h>
# include <sys/dirent.h>
# include <sys/string.h>
# include <sys/execve.h>
# include <sys/process_que.h>

//GREG *regs;		// GLobal register variable
#define COW 0x0008000000000000 //52 bit used for COW in software
extern void _set_k_ptable_cr3(uint64_t);

void isr_handler_0(registers_t regs)
{
	printf("\nDivide by zero FAULT");
	printf("Interrupt Number = %d %d\n", 0x0, regs.err_code);
        while(1);
} 

void isr_handler_13(registers_t regs)
{
     printf("Exception: General Protection Fault\n");
     uint64_t cr2;
     __asm volatile(
	 "movq %%cr2, %0"
	 :"=g"(cr2)
	 :
	 :"memory"
     );
//     printf("GPF::INT Number = %x:ErrCode=%x cr=%x\n", 0xD, regs.err_code, cr2);
     while(1);
}

void isr_handler_14(registers_t regs)
{
	int re = 0;
//	printf("Exception: Page Fault\n");
    	uint64_t cr2, ccow = 0, ppentry = 0;
  	__asm volatile(
		"movq %%cr2, %0"
		:"=g"(cr2)
		:
	  :"memory"
	 );
	
	// Handling SEGV
	if (0 == cr2)
	{
		printf("\n SEG FAULT :Trying to access address 0x0:%d", cr2);
		while(1);
	}	
	re = (regs.err_code & 7);
	if (re == 7)
	{
		ppentry = ((uint64_t) ptov_map((uint64_t)cr2));
		if (0 == ppentry)
		{
			printf("\n VA DO NOT EXIST");
		}
		ccow = ((((uint64_t)ppentry) & COW) >> 51);
		printf("\n ccow:%d:ppentry%x",ccow, ppentry);
		if (ccow)
		{
			copyOnWritePageTables();
			 _set_k_ptable_cr3(running->cr3);
		}
	}	
	else if ((0 == re) || (2 == re) || (4 == re) || (6 == re))
     	{
     		self_refrence(cr2);
     	}

//     while(1);
}


/********************************************************************/
uint64_t isr_handler_80(GREG *regs)
{
	uint64_t ret = 0, str = 0, fd = 0, num = 0,exit_status;
	uint64_t num_bytes = 0, filename = 0, id = 0;
	uint64_t f = -1;
	int n = regs->rax,var;//,num;
	DIR *directory_struct;
	DIR *directory_init;
	dirent *dir;
	dirent *dir_init;
	uint64_t addr;

	switch(n){
      		case(1):
        		var =(int)regs->rbx;
			printf("%d",var);
        		break;
      		case(2):
        		addr = regs->rbx;
        		num = regs->rcx;
		        ret = write(1,(char *)addr,num);
		        regs->rax = ret;
		        break;
	         case(3):
        		f = doFork();
        		regs->rax = f;
        		break;
		  case(4):
        		num_bytes = regs->rbx;
        		addr =(uint64_t)p_malloc(num_bytes);
        		regs->rax = addr;
        		break;
     		  case(5):
        		addr = regs->rdx;
        		str = regs->rcx;
        		fd = regs->rbx;
        		if(fd ==0)
			{
        			ret =(uint64_t)scanf((char*)str,addr);
			}	
        		else
			{
          			ret =(uint64_t)read_file(fd,str,(char*)addr);
			}	
        		regs->rax = ret;
        		break;
      		   case(6):	
        		schedule1();
			break;
     		   case(7):
			filename = regs->rbx;
		        do_execve((char*)filename);
			break;
     
/***************************/

 		   case(8):
        		exit_status = regs->rbx;
        		do_exit(exit_status);
        		break;
      		   case(9):
        		id = regs->rbx;
        		ret = do_waitpid(id);
        		regs->rax = ret;
        		break;
      		   case(10):
        		ret = do_wait();
        		regs->rax = ret;
        		break;
     		    case(11):
        		directory_init =(DIR*)regs->rbx;
        		dir =(struct dirent*) regs->rcx;
        		dir_init = (struct dirent*)k_malloc(sizeof(struct dirent));
        		dir_init = readdir((DIR*)directory_init);
        
       	 	    	if(dir_init!=NULL)
        	    	{
          			regs->rax = 1;
          			strcpy(dir->d_name,dir_init->d_name);
          			dir->offset = dir_init->offset;
        		}
        		else
			{
          			regs->rax = 0;
			}	
        		break;
        		if(dir!=NULL)
			{
          			regs->rax = (uint64_t)dir;
        		}
			else
			{
          			regs->rax = 0;
			}	
        		break;
      		   case(12):
        		str = regs->rbx;
        		directory_init = (DIR*)regs->rcx;
        		directory_struct = opendir((char*)str);
        		if(directory_struct!=NULL)
        		{
          			regs->rax = 1;
          			strcpy(directory_init->dirname,directory_struct->dirname);
          			directory_init->current = directory_struct->current;
          			directory_init->dirent_filled = directory_struct->dirent_filled;
        		}

        		else
			{
          			regs->rax = 0;
			}	
        		break;

      		     case(13):
        		 directory_init = (DIR*)regs->rbx;
        		 ret = closedir(directory_init);
        		 regs->rax = ret;
        
		/***************************/
      		     case(14):
        		  addr = regs->rbx;
        		  num = regs->rcx;
        		  fd=2;
        		  ret = write(fd,(char *)addr,num);
        		  regs->rax = ret;
        		  break;
      		     case(15):
        		  str = regs->rbx;
        		  ret = open_file((char*)str);
        		  regs->rax = ret;
        		  break;
		     case(16):
        		   fd = regs->rbx;
	                   ret = close_file(fd);
		           regs->rax = ret;
			   break;
		     case(17):
		     	   do_Ps();
			   break;
		    case(18):
		    	ret = get_pid();
			regs->rax = ret;
		     default:
        		   return 0;
    		}
    	return ret;
		    
} 
