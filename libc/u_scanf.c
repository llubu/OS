#include <stdarg.h>
#include <stdio.h>
#include <syscall.h>
#include<defs.h>
#include<libc/malloc.h>

char buf[128];

int get_int(int* ptr )
{
  int i = 0;
  int ret = 0;
  while(buf[i] != '\0')
  {
    if( buf[i]<='9' && buf[i] >= '0' )
    {
      ret =(buf[i]-'0')+(ret*10);
      i++;
    }
    else
    {
      return -1;
    }
  }
  *ptr=ret;
  return 1;
}

int get_hex(uint64_t* ptr )
{
  int i = 0;
  int ret = 0;
  while(buf[i] != '\0')
  {
    if( buf[i]<='9' && buf[i] >= '0' )
      ret =(buf[i]-'0')+(ret*16);
    else
    { 
      if( buf[i]<='F' && buf[i] >= 'A' )
         ret =(buf[i]-'A'+10)+(ret*16);
      else{
        if( buf[i]<='f' && buf[i] >= 'a' )
          ret =(buf[i]-'a'+10)+(ret*16);
        else
          return -1;
      }
    }

    i++;
  }
  *ptr=ret;
  return 1;
}

uint64_t u_scanf(const char* format, uint64_t ptr) 
{
  uint64_t ret = 0;
//  u_printf("\n Inside Scanf\n"); 
  while(1)
  {
    ret=__syscall3(SYSCALL_READ,0,(uint64_t)format,(uint64_t)buf);
    if(ret != 0)
      break;
  }
//  u_printf("\nout of scanf infinite loop with %s \n",buf ) ;
  if(format[0] == '%' && format[1]=='d')
    ret = get_int((int*)ptr);
  if(format[0] == '%' && format[1]=='x')
    ret = get_hex((uint64_t*)ptr);
  if(format[0] == '%' && format[1]=='s')
  {
    char* str = (char*)ptr;
    int i=0;
    while(buf[i] != '\0')
    {
      str[i] =buf[i];
      i++;
    }
    ret = 1;
  }
//  u_printf("\nout of scanf\n");
  return ret;
}
