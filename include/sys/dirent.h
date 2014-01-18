#ifndef DIRENT_H
#define DIRENT_H

#include<sys/tarfs.h>
#include<defs.h>
#define NAMEMAX 256
  /*
   * Substitute for real dirent structure.  Note that `d_name' field is a
   * true character array although we have it copied in the implementation
   * dependent data.  We could save some memory if we had declared `d_name'
   * as a pointer refering the name within implementation dependent data.
   * We have not done that since some code may rely on sizeof(d_name) to be
   * something other than four.  Besides, directory entries are typically so
   * small that it takes virtually no time to copy them from place to place.
   */
   
#define _A_NORMAL   0x00    /* Normal file.    */
#define _A_RDONLY   0x01    /* Read only file.  */
#define _A_HIDDEN   0x02    /* Hidden file.     */
#define _A_SYSTEM   0x04    /* System file.     */
#define _A_SUBDIR   0x10    /* Subdirectory.    */
#define _A_ARCH     0x20    /* Archive file.    */

   
  typedef struct dirent {
    char d_name[NAMEMAX];
    uint64_t offset;
  } dirent;

  /* DIR substitute structure containing directory name.  The name is
   * essential for the operation of ``rewinndir'' function. */
  typedef struct DIR {
    char          dirname[NAMEMAX];                    /* directory being scanned */
    dirent        current;                     /* current entry */
    int           dirent_filled;               /* is current un-processed? */
  } DIR;
  
  
struct File {
  char  path[256]; //256
  uint64_t offset; //8
  struct dirent parent; //264
};
  
  void list();
  int findNext(struct dirent* current);
  DIR * opendir(const char *dirname);
  struct dirent* readdir(DIR* dir);
  int findfirst(char* dirname, char spec , struct dirent* data );
  int closedir (DIR *dirp);
  uint64_t open_file(char* filename);
  uint64_t close_file(uint64_t fd);
  uint64_t read_file(uint64_t fd,uint64_t num, char* buffer);

  
#endif
