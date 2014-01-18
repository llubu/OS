# include <stdio.h>
# include <sys/task_management.h>
# include <sys/v_mem_manager.h>
# include <sys/task_switch.h>
# include <sys/irq.h>

/*
* Removes a process with a given pid from a given ProcessQ
* returns the removed Process PCB 
*/

PCB *removefromQ(PLIST *list, uint64_t pid)
{
	PLIST *tmp = NULL;
	PLIST *prv = NULL;
	
	prv = list;
	if (list->next)
	{
		tmp = list->next;
	}	
	
	while(tmp)
	{
		if (tmp->pcb_li->pid == pid)
		{
			prv->next = tmp->next;
			return (tmp->pcb_li);
		}
		tmp = tmp->next;
		prv = tmp;
	}
	return NULL;
}




/*
* Looks for a process with id in process Q
* returns pid of found process on sucess and -1 if not found
*/
int lookintoQ(PLIST *list, uint64_t id)
{
	PLIST *tmp = NULL;
	int ret = -1;
	tmp = list;

	while(tmp->next)
	{
		if (tmp->pcb_li->pid == id)
		{
			return id; 		
		}			
	}
	
	return ret;
}



/*
* Looks for a process with ppid i == running process in process Q
* returns 0 on sucess and -1 if not found
*/
int lookintoQChild(PLIST *list, uint64_t id)
{
	PLIST *tmp = NULL;
	int ret = -1;
	tmp = list;

	while(tmp->next)
	{
		if (tmp->pcb_li->ppid == id)
		{
			ret = 0;		
		}			
	}
	
	return ret;
}


/*
* exit sys call 
*
*/

void do_exit(int status)
{
	PCB *tmp = NULL;
	uint64_t pid = -1;

	if ((pid = lookintoQ(runableQ, running->ppid)))
	{	
		tmp = removefromQ(waitQ, pid);
		if (NULL == tmp)
		{
			printf("\n Error given process not found");
		}
		//move running to waitQ and schedule another
		add_toQ(runableQ, tmp);
//		free(running);
		schedule1();
	}
}


/*
* If a process exist with pid in runbaleQ then current proccess
*
*/
int do_waitpid(uint64_t pid)
{
	if (0 == (lookintoQ(runableQ, pid)))
	{
		//move running to waitQ and schedule another
		add_toQ(waitQ, running);
		schedule1();
		return 0;
	}
	else
	{
		return -1;
	}	
}

/*
* Current waits if any of the child is in runableQ,second half in exit
* 
*/

int do_wait()
{
	if (0 == (lookintoQChild(runableQ, running->pid)))
	{
		//move running to waitQ and schedule another
		add_toQ(waitQ, running);
		schedule1();
		return 0;
	}
	else
	{
		return -1;
	}	
}


/*
* ls syscall
*
*/

void do_Ps()
{
	PLIST *tmp = NULL;

	tmp = runableQ;
	printf("\nps OUTPUT:\nRunable");
	printf("\nPID\tTime\tName");

	while (tmp->next)
	{
		printf("%d,\t,%d,\t%s", tmp->pcb_li->pid, tmp->pcb_li->load_time, tmp->pcb_li->name);

	}
	tmp = waitQ;
	printf("\nps OUTPUT:\nWaiting");
	printf("\nPID\tTime\tName");

	while (tmp->next)
	{
		
		printf("%d,\t,%d,\t%s", tmp->pcb_li->pid, tmp->pcb_li->load_time, tmp->pcb_li->name);

	}
}




