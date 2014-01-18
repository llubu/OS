#include <defs.h>
#include <stdio.h>
#include <sys/gdt.h>
#include <sys/idt.h>
#include <sys/isr.h>
#include <sys/irq.h>
#include <sys/pmem_manager.h>
#include <sys/kern_page_table.h>
#include <sys/task_management.h>
#include <sys/tarfs.h>
#include <sys/string.h>
#include <sys/errno.h>

int fd_used = 2;
FDT fd_table[150];

void init_fdtable()
{
	uint64_t i = 0;	
//	printf("\n IN FD_TABLE INIT");
	for (i = 0; i < 150; i++)
	{
		fd_table[i].seek = 0;
		fd_table[i].number = -1;
	}
}

int strcmp2(char* str1, char*str2)
{
	while(*str1 !='\0' || *str2!='\0' )
  	{

    		if(*str1 != *str2)
		{
      			return 0;
		}	
    		str1++;
    		str2++;
  	}
  	if(*str1=='\0'&&*str2=='\0')
  	{
    		return 1;
  	}
  	return 0;
}
char zero_size[12] = "000000000000";

int power(int x, int y)
{
	if( y == 0)
	{
     		return 1;
	}	
   	else if (y%2 == 0)
	{
     		return power(x, y/2)*power(x, y/2);
	}	
   	else
	{
     		return x*power(x, y/2)*power(x, y/2);
	}	
               
}

int size_to_int(char* p)
{
	int k = 0;
   	while (*p) 
	{
    		k = (k<<3)+(k<<1)+(*p)-'0';
    		p++;
  	}
  	int decimal=0, i=0, rem; 
  	while (k!=0) 
  	{ 
     		rem = k%10; 
     		k/=10; 
     		decimal += rem*power(8,i); 
     		++i;
  	}
      
      	return decimal;
}

 
int psize_to_int(char* size)
{
	int ret = 0;
  	int base = 1;
  	int i =7;
  	while(size[i] <= '0' || size[i] > '9'  )
    	i--;
  	for(i=i;  i >= 0; i--)
  	{
    		ret += base*(size[i] - '0');
    		base *= 8;
  	}
  	return ret;
}

VMA* read_pheader(char* addr, struct elf_header* elf_base)
{
	struct pheader* pbase = (struct pheader*)(addr);
   	uint64_t filesz,memsz,vaddr,offset;

   	VMA* vma = NULL;
   	filesz = pbase->p_filesz;
   	memsz = pbase->p_memsz;
   	vaddr = pbase->p_vaddr;
   	offset = pbase->p_offset;
   	uint64_t base = (uint64_t) elf_base;
   	uint64_t contents = base + offset;

   	if(memsz != 0)
   	{
     		uint32_t ret = m_map((uint64_t)vaddr,(uint64_t)contents, filesz, memsz);
     		if(ret == 0)
		{
   	   		vma = create_vma(vaddr,memsz);
		}	
   	}
  	return vma;
}
/*
* Gets the size of the elf, for the elf file specified in the argument
*
*/

uint64_t size_elf(char *size)
{
	uint64_t header_size = ((uint64_t)(*size))+256*((uint64_t)(*(size+1)));
  	return header_size;
}

int size_offset(char* size)
{
	uint64_t off = 0;
  	uint64_t base = 1;
  	for(int i = 0;i < 8 ; i++)
  	{
    		off+=base*((uint64_t)size[i]);
    		base*=8;
  	}
  	return off;
}

int get_num_segs(char* num)
{
	return 10*((num[1]) ) + (num[0]);
}

void readelf(char* addr, PCB *task)
{
	struct elf_header* elf_base = (struct elf_header*)(addr);
  	char* pheader;
  	pheader = (addr)+size_offset(elf_base->e_phoff);

  	int i = 0;
  	int num_entries = get_num_segs(elf_base->e_phnum);
  	task->rip = elf_base->e_entry;
  	VMA* newp = NULL;
  	VMA* ht = NULL;

  	for(i = 0; i < num_entries; i++ )
  	{
  		if (i == 0)
		{
			newp = read_pheader(pheader, elf_base);
			task->mm_st = newp; 
		 	pheader = pheader+size_elf(elf_base->e_phentsize);
		}	
		else 
		{
    			ht  = read_pheader(pheader, elf_base);
      			if(ht!=NULL)
      			{
  				newp->next = ht;
	  			newp = newp->next;
		   		pheader = pheader+size_elf(elf_base->e_phentsize);
      			}
    		}
  	}
}

int read_tarfs(PCB* task, char* name)
{
	if(name == NULL)
	{
    		return 1;
	}	
 
 	int temp_size;
  	int step = 0,mod; 
  	char* elf_header = NULL;
  	char* prevname = NULL;
  	char* prevsize = NULL;
  	struct posix_header_ustar *header =  (struct posix_header_ustar*)(&_binary_tarfs_start);
  	struct posix_header_ustar *end =  (struct posix_header_ustar*)(&_binary_tarfs_end);
  
 	while(header < end)
 	{
    		temp_size = size_to_int(header->size);
    		if(temp_size > 0 && strcmp2(name,header->name))
    		{

        		elf_header =(char*)(header+1);
        		readelf((char*)elf_header, task);
        		return 0;
    		}
    
    		if(header->name[0] == '\0' && header->size[0]=='\0' && prevname == '\0' && prevsize == '\0')
    		{
      			return 2;
    		}

    		mod = temp_size%(sizeof(struct posix_header_ustar));
    		if(mod == 0)
		{
      			step = 1 +(int)( temp_size/(sizeof(struct posix_header_ustar)));
		}	
    		else
		{
      			step = 2 +(int)( temp_size/(sizeof(struct posix_header_ustar)));
		}	
    		prevname = header->name;
    		prevsize = header->size;
    		header = header+step;
 	}
 return 2;
}

struct File* getFile(const char *fileName)
{
      struct posix_header_ustar * entry = (struct posix_header_ustar*)(&_binary_tarfs_start);
      int padding =0;
      errno = 0;
      struct File* file = (struct File *)k_malloc(sizeof(struct File));
      int exitflag = 0;
      while((uint64_t)entry < (uint64_t)&_binary_tarfs_end)
      {
        	int size = size_to_int(entry->size);
        	if(strcmp(entry->typeflag,"5")  == 0 && strncmp(fileName,entry->name,lastIndexOf (fileName, "/")+1) == 0 &&  lastIndexOf (fileName, "/")+ 1 == strlen(entry->name))
		{
        		strncpy(file->parent.d_name,entry->name,NAMEMAX);
        		file->parent.offset = (char*)entry - (char*)&_binary_tarfs_start;
        		if(++exitflag >= 2)
			{
            			break;
			}	
         	}
        	if(strcmp(fileName,entry->name) == 0 && strcmp(entry->typeflag,"0")  == 0)
		{
        		strncpy(file->path, fileName, NAMEMAX);
        		file->offset = (char*)entry - (char*)&_binary_tarfs_start;
        		if(++exitflag >= 2)
			{
                    		break;
			}	
        	}
        	entry = (struct posix_header_ustar *)((char*)entry + sizeof(struct posix_header_ustar) + size );
        	if(size > 0)
		{
          		padding = BLOCKSIZE - size%BLOCKSIZE;
          		if((char*)&_binary_tarfs_end - (char*)entry >=BLOCKSIZE)
          		{
            			entry = (struct posix_header_ustar *)((char*)entry + padding);
          		}
        	}
      }

      if(strcmp(fileName,entry->name)!=0)
      {
		return NULL;
      }
      return file;
}

uint64_t read_file(uint64_t fd,uint64_t num, char* buffer)
{
	volatile uint64_t i = 0;
  	while(fd_table[i].number != fd && i <150)
  	{
      		i++;
  	}
  	if(i >=150)
	{
    		return NULL;
	}
// 	printf("\nFile has fd %d",fd);
  	FDT entry = fd_table[i];
  	char* base = (char*)&_binary_tarfs_start;
  	base = base + entry.fp->offset ;
  	struct posix_header_ustar* tarfs_struct = (struct posix_header_ustar*)base;
  	int size = size_to_int(tarfs_struct->size);
  	if(size == 0)
  	{
    		return NULL;
    	}
//printf("size of file being read  = %d with seek = %d\n",size,entry.seek);
  	base = (char*)(tarfs_struct+1);
  	base = base + entry.seek ;

  	int index = 0;
  	for(index=0; index < num ; index++ )
  	{
    		if(entry.seek<size)
    		{
      			buffer[index] = base[index]; 
      			entry.seek++;
    		}
  	}
  	fd_table[i].seek = entry.seek;
  	return i;
}

uint64_t close_file(uint64_t fd)
{
	uint64_t i = 0;
  	while(fd_table[i].number != fd && i<150)
  	{
      		i++;
  	}
  	if(i >=150)
	{
    		return 1;
	}	
  	fd_table[i].number=-1;
  	fd_table[i].fp=NULL;
 	fd_table[i].seek = 0;
  	return 0;
}

uint64_t open_file(char* filename)
{
	uint64_t i = 0;
  	while(fd_table[i].number != -1)
  	{
      		i++;
  	}
  	if(getFile(filename)==NULL)
	{
    		return -1;
	}	
  	fd_table[i].number=fd_used + 1;
  	fd_table[i].fp=getFile(filename) ;
  	if(fd_table[i].fp == NULL)
	{
    		return -1;
	}	
  	fd_table[i].seek = 0;
  	return fd_table[i].number;
}
