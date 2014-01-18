/*
 * Implement dirent interface as static functions so that the user does not
 * need to change his project in any way to use dirent function.  With this
 * it is sufficient to include this very header from source modules using
 * dirent functions and the functions will be pulled in automatically.
 */
#include <stdio.h>
#include <defs.h>
#include <stdlib.h>
#include <sys/tarfs.h>
#include <sys/string.h>
#include <sys/errno.h>
#include <sys/tarfs.h>
#include <sys/dirent.h>
#include <sys/pmem_manager.h>
#include <sys/v_mem_manager.h>
#include<sys/kern_page_table.h>

#define BLOCKSIZE 512


char *strchr(const char *s, int c);
int strcmp(const char *s1, const char *s2);
char* strcpy(char *s1, const char *s2);
char* strncpy(char *s1, const char *s2, int n);
int getSize(char *p);
int initdir (DIR *p);
const char *_getdirname (const struct dirent *dp);
void _setdirname (struct DIR *dirp);
int findfirst(char* dirname, char spec , struct dirent* data );
DIR * opendir(const char *dirname);


extern char _binary_tarfs_start;
int errno;

/*
 * <function name="opendir">
 * <intro>open directory stream for reading
 * <syntax>DIR *opendir (const char *dirname);
 *
 * <desc>Open named directory stream for read and return pointer to the
 * internal working area that is used for retrieving individual directory
 * entries.  The internal working area has no fields of your interest.
 *
 * <ret>Returns a pointer to the internal working area or NULL in case the 
 * directory stream could not be opened.  Global `errno' variable will set
 * in case of error as follows:
 *
 * <table>
 * [EACESS  |Permission denied.
 * [EMFILE  |Too many open files used by the process.
 * [ENFILE  |Too many open files in system.
 * [ENOENT  |Directory does not exist.
 * [ENOMEM  |Insufficient memory.
 * [ENOTDIR |dirname does not refer to directory.  This value is not
 *           reliable on MS-DOS and MS-Windows platforms.  Many
 *           implementations return ENOENT even when the name refers to a
 *           file.]
 * </table>
 * </function>
 */
 DIR * opendir(const char *dirname)
{
  DIR *dirp;
  if(dirname != NULL){
	dirp = (DIR*)k_malloc(sizeof(DIR));
	if (dirp != NULL) {
    char *p;
    strncpy (dirp->dirname, dirname, NAMEMAX);
    p = strchr (dirp->dirname, '\0'); //Return substring starting from character c
    if (dirp->dirname < p  &&
        *(p - 1) != '\\'  &&  *(p - 1) != '/'  &&  *(p - 1) != ':')
    {
      strcpy (p++, "/");
    }
	
    // open stream 
    if (initdir (dirp) == 0) {//Returns 1 if directory is found.
      // initialization failed 
     // unFreePage(GetMappedPhysicalAddress((uint64_t)dirp));
	  errno = ENOENT;
      return NULL;
	}
	//printf("found : %s",dirname);
	}
  return dirp;
  }
  errno = ENOTDIR;
  return NULL;
}

/*
 * <function name="readdir">
 * <intro>read a directory entry
 * <syntax>struct dirent *readdir (DIR *dirp);
 *
 * <desc>Read individual directory entry and return pointer to a structure
 * containing the name of the entry.  Individual directory entries returned
 * include normal files, sub-directories, pseudo-directories "." and ".."
 * and also volume labels, hidden files and system files in MS-DOS and
 * MS-Windows.   You might want to use stat(2) function to determinate which
 * one are you dealing with.  Many dirent implementations already contain
 * equivalent information in dirent structure but you cannot depend on
 * this.
 *
 * The dirent structure contains several system dependent fields that
 * generally have no interest to you.  The only interesting one is char
 * d_name[] that is also portable across different systems.  The d_name
 * field contains the name of the directory entry without leading path.
 * While d_name is portable across different systems the actual storage
 * capacity of d_name varies from system to system and there is no portable
 * way to find out it at compile time as different systems define the
 * capacity of d_name with different macros and some systems do not define
 * capacity at all (besides actual declaration of the field). If you really
 * need to find out storage capacity of d_name then you might want to try
 * NAME_MAX macro. The NAME_MAX is defined in POSIX standard althought
 * there are many MS-DOS and MS-Windows implementations those do not define
 * it.  There are also systems that declare d_name as "char d_name[1]" and
 * then allocate suitable amount of memory at run-time.  Thanks to Alain
 * Decamps (Alain.Decamps@advalvas.be) for pointing it out to me.
 *
 * This all leads to the fact that it is difficult to allocate space
 * for the directory names when the very same program is being compiled on
 * number of operating systems.  Therefore I suggest that you always
 * allocate space for directory names dynamically.
 *
 * <ret>
 * Returns a pointer to a structure containing name of the directory entry
 * in `d_name' field or NULL if there was an error.  In case of an error the
 * global `errno' variable will set as follows:
 *
 * <table>
 * [EBADF  |dir parameter refers to an invalid directory stream.  This value
 *          is not set reliably on all implementations.]
 * </table>
 * </function>
 */
struct dirent * readdir (DIR *dirp)
{
  if (dirp == NULL) {
    errno = EBADF;
    return NULL;
  }
  if (dirp->dirent_filled != 0) {
    /*
     * Directory entry has already been retrieved and there is no need to
     * retrieve a new one.  Directory entry will be retrieved in advance
     * when the user calls readdir function for the first time.  This is so
     * because real dirent has separate functions for opening and reading
     * the stream whereas Win32 and DOS dirents open the stream
     * automatically when we retrieve the first file.  Therefore, we have to
     * save the first file when opening the stream and later we have to
     * return the saved entry when the user tries to read the first entry.
     */
    dirp->dirent_filled = 0;
  } else if (findNext(&dirp->current) != 0) {
      /* findnext will set errno to ENOENT when no
       * more entries could be retrieved. */
       return NULL;
  }
  //printf("\n\nfound next : %s",dirp->current.d_name);
  return &dirp->current;
} 


/*
 * <function name="closedir">
 * <intro>close directory stream.
 * <syntax>int closedir (DIR *dirp);
 *
 * <desc>Close directory stream opened by the `opendir' function.  Close of
 * directory stream invalidates the DIR structure as well as previously read
 * dirent entry.
 *
 * <ret>The function typically returns 0 on success and -1 on failure but
 * the function may be declared to return void on same systems.  At least
 * Borland C/C++ and some UNIX implementations use void as a return type.
 * The dirent wrapper tries to define VOID_CLOSEDIR whenever closedir is
 * known to return nothing.  The very same definition is made by the GNU
 * autoconf if you happen to use it.
 *
 * The global `errno' variable will set to EBADF in case of error.
 * </function>
 */
 int closedir (DIR *dirp)
 {   
  int retcode = 0;
 // make sure that dirp points to legal structure 
  if (dirp == NULL) {
    errno = EBADF;
    return -1;
  }
  // clear dirp structure to make sure that it cannot be used anymore
//  unFreePage(GetMappedPhysicalAddress((uint64_t)dirp));
  return retcode;
}


/*
 * <function name="rewinddir">
 * <intro>rewind directory stream to the beginning
 * <syntax>void rewinddir (DIR *dirp);
 *
 * <desc>Rewind directory stream to the beginning so that the next call of
 * readdir() returns the very first directory entry again.  However, note
 * that next call of readdir() may not return the same directory entry as it
 * did in first time.  The directory stream may have been affected by newly
 * created files.
 *
 * Almost every dirent implementation ensure that rewinddir will update
 * the directory stream to reflect any changes made to the directory entries
 * since the previous ``opendir'' or ``rewinddir'' call.  Keep an eye on
 * this if your program depends on the feature.  I know at least one dirent
 * implementation where you are required to close and re-open the stream to
 * see the changes.
 *
 * <ret>Returns nothing.  If something went wrong while rewinding, you will
 * notice it later when you try to retrieve the first directory entry.
 */
 void rewinddir (DIR *dirp)
 {   
  // make sure that dirp is legal 
  if (dirp == NULL) {
    errno = EBADF;
    return;
  }
  
  // re-open previous stream 
  if (initdir (dirp) == 0) {
    /* initialization failed but we cannot deal with error.  User will notice
     * error later when she tries to retrieve first directory entry. */
    /*EMPTY*/;
	}
 } 


/*
 * Open native directory stream object and retrieve first file.
 * Be sure to close previous stream before opening new one.
 */
 int initdir (DIR *dirp)
{ 
  dirp->dirent_filled = 0;

  if (findfirst (dirp->dirname, _A_SUBDIR | _A_RDONLY | _A_ARCH | _A_SYSTEM | _A_HIDDEN, &dirp->current) != 0)
  {
    /* findfirst will set errno to ENOENT when no 
     * more entries could be retrieved. */
    return 0;
  }
  // initialize DIR and it's first entry 
  dirp->dirent_filled = 1;
  return 1;
}


/*
 * Return implementation dependent name of the current directory entry.
 */
 const char * _getdirname (const struct dirent *dp)
{
  return dp -> d_name;
}


void displayentry(struct posix_header_ustar* entry){
	printf("\nname - %s, size-%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s",entry ->name, entry->size, entry-> mode , entry-> uid, entry-> gid, entry-> mtime, entry-> checksum, entry-> typeflag, entry-> linkname, entry-> magic, entry-> version, entry-> uname, entry-> gname, entry-> devmajor, entry-> devminor , entry-> prefix  , entry-> pad);
}


int findNext(struct dirent* current)
{
	struct posix_header_ustar* entry = (struct posix_header_ustar*)&_binary_tarfs_start + current->offset;
	if(entry != NULL){
		int padding =0; int offset = current -> offset;
		char tempstr[NAMEMAX], dirname[NAMEMAX]; int index;
		strncpy(tempstr,entry->name,strlen(entry->name));
		if(strcmp(entry->typeflag,"5") == 0 || strcmp(entry->typeflag,"0") == 0){
			strncpy(tempstr,entry->name,strlen(entry->name)-1);
			tempstr[strlen(entry->name)-1] = 0;
			index = lastIndexOf (tempstr, "/");
			if(index == -1 || index == strlen(entry->name)-1)
			dirname[0] = '\0';
			else
			substring(dirname ,entry->name , 0, index);
		}
		
		do{		
			int size = size_to_int(entry->size);	
			//increasing by size of the structure + size of file
			entry = (struct posix_header_ustar *)((char*)entry + sizeof(struct posix_header_ustar) + size );
			offset += 1 + (size/BLOCKSIZE);
			if(size > 0){
				padding = BLOCKSIZE - size%BLOCKSIZE;
				//printf("value - %d & padding - %d",(char*)&_binary_tarfs_end - (char*)entry, padding);
				if((char*)&_binary_tarfs_end - (char*)entry >= BLOCKSIZE && padding > 0)
				{
					entry = (struct posix_header_ustar *)((char*)entry + padding);
					offset++;
				}
				//printf(" ,address:%p",entry);
				}	
			if(starts_with( entry-> name,dirname) && strcmp(entry->typeflag,"5")  == 0) //starts_with( base,prefix)
			{
				if(indexOf_shift(entry-> name, "/", strlen(dirname)+1) == strlen(entry->name)-1){
					//indexOf_shift(base, str, startIndex)
					current -> offset = offset;	
					strncpy(current -> d_name, entry->name, NAMEMAX-1);
					//printf("found - %s",current->d_name);
					return 0;
				}
			}
			if(starts_with( entry-> name,dirname) && strcmp(entry->typeflag,"0")  == 0 ){
				if((indexOf_shift(entry-> name, "/", strlen(dirname)+1))==-1){
					current -> offset = offset;	
					strncpy(current -> d_name, entry->name, NAMEMAX-1);
					//printf("found - %s",current->d_name);
					return 0;
				}
			}	
			}while((uint64_t)entry < (uint64_t)&_binary_tarfs_end);
		}
		errno = ENOENT;
		return errno;
 }




int findfirst(char* dirname, char spec , struct dirent* data )
{	
	struct posix_header_ustar * entry = (struct posix_header_ustar *)&_binary_tarfs_start;
	
	//specs of files: not considering for now, (_A_SUBDIR | _A_RDONLY | _A_ARCH | _A_SYSTEM | _A_HIDDEN)
	if(entry != NULL){
		int padding =0, offset = 0;
		while( (uint64_t)entry < (uint64_t)&_binary_tarfs_end){
			int size = size_to_int(entry->size);
			if(starts_with(entry-> name,dirname) && strcmp(entry->typeflag,"5")  == 0) //starts_with( base,prefix)
			{
				if(indexOf_shift(entry-> name, "/", strlen(dirname)+1) == strlen(entry->name)-1){
					//indexOf_shift(base, str, startIndex)
					strncpy(data -> d_name,entry -> name,NAMEMAX-1);
					data -> offset = offset;
					printf("found - %s",data->d_name);
					return 0;
				}
			}
			if(starts_with( entry-> name,dirname) && strcmp(entry->typeflag,"0")  == 0 ){
        printf("\ngetting the file\n");
				if((indexOf_shift(entry-> name, "/", strlen(dirname)+1))==-1){
					strncpy(data -> d_name,entry -> name,NAMEMAX-1);
					data -> offset = offset;
					printf("found - %s",data->d_name);
					return 0;
				}
			}
			//increasing by size of the structure + size of file
			entry = (struct posix_header_ustar *)((char*)entry + sizeof(struct posix_header_ustar) + size );
			offset += 1 + (size/BLOCKSIZE);
			if(size > 0){
				padding = BLOCKSIZE - size%BLOCKSIZE;
				printf("value - %d & padding - %d",(char*)&_binary_tarfs_end - (char*)entry, padding);
				if((char*)&_binary_tarfs_end - (char*)entry >= BLOCKSIZE && padding > 0)
				{
					entry = (struct posix_header_ustar *)((char*)entry + padding);
					offset += 1;
				}
				printf(" ,address:%p",entry);
				}	
			}
		}
		errno = ENOENT;
		return errno;
}


void list()
{
    struct posix_header_ustar * entry = (struct posix_header_ustar*)(&_binary_tarfs_start);
    
    printf("tarfs in [2q%p:%p]\n", &_binary_tarfs_start, &_binary_tarfs_end);
    int padding =0;
	//printf("\nname - %s, size-%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s",entry ->name, entry->size, entry-> mode , entry-> uid, entry-> gid, entry-> mtime, entry-> checksum, entry-> typeflag, entry-> linkname, entry-> magic, entry-> version, entry-> uname, entry-> gname, entry-> devmajor, entry-> devminor , entry-> prefix  , entry-> pad);
    //while(strcmp(fileName,entry ->name) != 0 && (uint64_t)entry < (uint64_t)&_binary_tarfs_end)
    while((uint64_t)entry < (uint64_t)&_binary_tarfs_end)
        {
    	   int size =size_to_int(entry->size);
           
		   
			//printf("\nname - %s, size-%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s",entry ->name, entry->size, entry-> mode , entry-> uid, entry-> gid, entry-> mtime, entry-> checksum, entry-> typeflag, entry-> linkname, entry-> magic, entry-> version, entry-> uname, entry-> gname, entry-> devmajor, entry-> devminor , entry-> prefix  , entry-> pad);
           if(strcmp(entry->typeflag,"5")  == 0){
				//printf("  Directory -- %s, size-%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s",entry ->name, entry->size, entry-> mode , entry-> uid, entry-> gid, entry-> mtime, entry-> checksum, entry-> typeflag, entry-> linkname, entry-> magic, entry-> version, entry-> uname, entry-> gname, entry-> devmajor, entry-> devminor , entry-> prefix  , entry-> pad);
				printf(" \n Directory -- %s, size-%s, type %s ",entry ->name, entry->size, entry-> typeflag);
    //while(1);
			}else if(strcmp(entry->typeflag,"0")==0){
				//printf("  file -- %s, size-%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s",entry ->name, entry->size, entry-> mode , entry-> uid, entry-> gid, entry-> mtime, entry-> checksum, entry-> typeflag, entry-> linkname, entry-> magic, entry-> version, entry-> uname, entry-> gname, entry-> devmajor, entry-> devminor , entry-> prefix  , entry-> pad);
				printf(" \n file -- %s, size-%s, type %s ",entry ->name, entry->size, entry-> typeflag);
			}else if(entry->typeflag == NULL){
				printf(" typeflag null");
			}else{
				printf(" entry address : %p ,entry->typeflag %s, name %s",entry,entry->typeflag,entry->name);
			}
           //increasing by size of the structure + size of file
           entry = (struct posix_header_ustar *)((char*)entry + sizeof(struct posix_header_ustar) + size );
		   if(size > 0){
				padding = BLOCKSIZE - size%BLOCKSIZE;
				//printf("value - %d & padding - %d",(char*)&_binary_tarfs_end - (char*)entry, padding);
				if((char*)&_binary_tarfs_end - (char*)entry >=BLOCKSIZE)
				{
					entry = (struct posix_header_ustar *)((char*)entry + padding);
					//break;
				}
			 //printf(" ,address:%p",entry);
			 //add padding
			}	
		} 

}
