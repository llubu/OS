#include <libc/dir.h>

struct dirent* u_readdir(DIR* directory)
{
	uint64_t ret;
  struct dirent* dir = (struct dirent*)malloc(sizeof(dirent));
	ret =  __syscall2(SYSCALL_READDIR,(uint64_t)directory,(uint64_t)dir);
  if(ret == 0)
    return NULL;
	return dir;
}

DIR* u_opendir(char* str)
{
	uint64_t ret;
  DIR* directory = (DIR *)malloc(sizeof(DIR));
	ret =  __syscall2(SYSCALL_OPENDIR,(uint64_t)str,(uint64_t)directory);
  if(ret == 0)
    return NULL;

	return directory;
}

uint64_t u_closedir(DIR* directory)
{
	uint64_t ret;
	ret =  __syscall1(SYSCALL_CLOSEDIR,(uint64_t)directory);
  if(ret == 0)
    return NULL;

	return ret;
}
uint64_t open(char* str)
{
	uint64_t ret;
	ret =  __syscall1(SYSCALL_OPEN,(uint64_t)str);

	return ret;
}
uint64_t close(uint64_t str)
{
	uint64_t ret;
	ret =  __syscall1(SYSCALL_CLOSE,(uint64_t)str);

	return ret;
}
uint64_t read(uint64_t fd,uint64_t num, char* str )
{
	uint64_t ret;
	ret =  __syscall3(SYSCALL_READ,fd,num,(uint64_t)str);

	return ret;
}
