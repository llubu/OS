#ifndef _TARFS_H
#define _TARFS_H
#define BLOCKSIZE 512
#include<sys/task_management.h>
#include<sys/v_mem_manager.h>
#include<sys/dirent.h>

extern char _binary_tarfs_start;
extern char _binary_tarfs_end;
extern int fd_used;
int size_to_int(char* size);
struct posix_header_ustar {
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char checksum[8];
	char typeflag[1];
	char linkname[100];
	char magic[6];
	char version[2];
	char uname[32];
	char gname[32];
	char devmajor[8];
	char devminor[8];
	char prefix[155];
	char pad[12];
};
struct elf_header
{
  unsigned char e_ident[16]; /* ELF identification */
  char e_type[2]; /* Object file type */
  char e_machine[2]; /* Machine type */
  char e_version[4]; /* Object file version */
  uint64_t e_entry; /* Entry point address */
  char e_phoff[8]; /* Program header offset */
  char e_shoff[8]; /* Section header offset */
  char e_flags[4]; /* Processor-specific flags */
  char e_ehsize[2]; /* ELF header size */
  char e_phentsize[2]; /* Size of program header entry */
  char e_phnum[2]; /* Number of program header entries */
  char e_shentsize[2]; /* Size of section header entry */
  char e_shnum[2]; /* Number of section header entries */
  char e_shstrndx[2]; /* Section name string table index */
};

struct pheader
{
  char p_type[4]; /* Type of segment */
  char p_flags[4]; /* Segment attributes */
  uint64_t p_offset; /* Offset in file */
  uint64_t p_vaddr; /* Virtual address in memory */
  char p_paddr[8]; /* Reserved */
  uint64_t p_filesz; /* Size of segment in file */
  uint64_t p_memsz; /* Size of segment in memory */
  char p_align[8]; /* Alignment of segment */
};

struct file_descriptor{
  int number;
  struct File* fp;
  uint64_t seek; 
};
typedef struct file_descriptor FDT;

//Size 532

extern FDT fd_table[150];
int read_tarfs(PCB* task, char* name);
VMA* read_pheader(char* addr, struct elf_header* elf_base);
void init_fdtable();
#endif
