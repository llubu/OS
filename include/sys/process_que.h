#ifndef _PROCESS_QUE_
#define _PROCESS_QUE_


int lookintoQ(PLIST *list, uint64_t id); //returns pid on sucess and -1 if not found
int lookintoQChild(PLIST *list, uint64_t id); //returns 0 on sucess and -1 if not found
PCB *removefromQ(PLIST *list, uint64_t pid);

void do_exit(int status);
uint64_t do_waitpid(uint64_t pid);
int do_wait(); 
void do_Ps();
#endif
