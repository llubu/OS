#include <stdarg.h>
#include <stdio.h>
#include <syscall.h>
#include<defs.h>
#include<libc/malloc.h>


int user_putint(int value, char* buffer, int total)
{
  char ptr[30]={'\0'};
  char rc[30]={'\0'};
  int index = 0;
  int neg = 0;
  int j = 0;
  int i =0;
  char vals[10] = "0123456789";
  if ( value < 0 )
  {
    value*=-1;
    neg = 1;
  }
  do
  {
    // Modulo is negative for negative value. This trick makes abs() unnecessary.
    ptr[index] = vals[value%10];
    value /= 10;
    index++;
  } while ( value );
  if(neg)
    ptr[index]='-';
  else
    index--;
  while ( index >= 0 )
  {
    rc[j]=ptr[index];
    j++;
    index--;
  }
  for(i = 0 ; i < j ; i++)
  {
    buffer[total+i] = rc[i];
  }
  //__syscall2(SYSCALL_PUTS,(uint64_t)rc,i);
  return i;
}

int user_long2hex(unsigned long value, char* buffer, int total)
{
  char ptr[40]={'a'};
  char rc[40]={'a'};
  int i =0;
  int index =0, j= 0; 
  do
  {
    int temp = value & 0x0f;
    if(temp < 10)
        ptr[index] = temp + '0';
     else
        ptr[index] = (temp) - 10 + 'A';
     value = value >> 4;
     index++;

  } while ( value );
  
  ptr[index] = 'x';
  index++;
  ptr[index] = '0';
  while ( index >= 0 )
  {
    rc[j]=ptr[index];
    j++;
    index--;
  }
  for(i = 0 ; i < j ; i++)
  {
    buffer[total+i] = rc[i];
  }
  //__syscall2(SYSCALL_PUTS,(uint64_t)rc,i);
  return i;
}

int user_putlong(unsigned long value, char* buffer, int total)
{
  char ptr[30]={'a'};
  char rc[30]={'a'};
  int index = 0;
  int neg = 0;
  int j = 0, i = 0;
  char vals[10] = "0123456789";
  if ( value < 0 )
  {
    value*=-1;
    neg = 1;
  }
  do
  {
    // Modulo is negative for negative value. This trick makes abs() unnecessary.
    ptr[index] = vals[value%10];
    value /= 10;
    index++;
  } while ( value );
  if(neg)
    ptr[index]='-';
  else
    index--;
  while ( index >= 0 )
  {
    rc[j]=ptr[index];
    j++;
    index--;
  }
  for(i = 0 ; i < j ; i++)
  {
    buffer[total + i] = rc[i];
  }
  //__syscall2(SYSCALL_PUTS,(uint64_t)rc,i);
  return i;
}


int user_int2hex(int value, char* buffer, int  total)
{
  char ptr[32]={'a'} ;
  char rc[32]={'a'};
  int index =0, j= 0; 
  int i =0;
  do
  {
    int temp = value & 0x0f;
    if(temp < 10)
        ptr[index] = temp + '0';
     else
        ptr[index] = (temp) - 10 + 'A';
     value = value >> 4;
     index++;

  } while ( value );
  
  ptr[index] = 'x';
  index++;
  ptr[index] = '0';
  while ( index >= 0 )
  {
    rc[j]=ptr[index];
    j++;
    index--;
  }
  for(i = 0 ; i < j ; i++)
  {
    buffer[total+i] = rc[i];
  }

  //__syscall2(SYSCALL_PUTS,(uint64_t)rc,i);
  return i;
}

int user_puts(char* str,char* buffer, int total)
{
  int i=0;
  while(str[i] != '\0')
  {
    buffer[i+total]=str[i];
    i++;
  }
 // __syscall2(SYSCALL_PUTS,(uint64_t)str,i);
  return i;
}
int user_putchar(char a, char* buffer , int total)
{
  buffer[0+total]=a;
  return 1;
}
uint64_t stder(const char* fmt, ...) 
{
	va_list parameters;
  char buffer[100] = {'\0'}; //= (char*)malloc(512) ;
  uint64_t ret = 0;
  int total=0;
  const char* str = fmt;
  va_start(parameters, fmt);
  str = fmt;
  while(*str != '\0')
  {
    if(*str != '%')
    {
      buffer[total] = *str;
      str++;
      total++;

      continue;
    }
    str++;
    switch( *str )
    {
      case 'c': total+=user_putchar(va_arg(parameters, int), buffer,total);
                break;

      case 'd': total+=user_putint(va_arg(parameters, int),buffer,total);
                break;

      case 's': total+=user_puts(va_arg(parameters, char*), buffer,total);
                break;

      case 'x': total+=user_int2hex(va_arg(parameters, int), buffer,total);
                break;
      
      case 'l': if(*(++str)=='d')
                {
                  total+=user_putlong(va_arg(parameters, unsigned long), buffer,total);
                  break;
                }

      case 'p': total+=user_long2hex(va_arg(parameters, unsigned long), buffer,total);
                break;

      case '%': total+=user_putchar('%', buffer,total);
                break;
    }
    str++;
  }
  va_end(parameters);
  buffer[total] = '\0';
  total++;
  ret=__syscall3(SYSCALL_STDERR,(uint64_t)buffer,total,2);
  return ret;
}

uint64_t u_printf(const char* fmt, ...) 
{
	va_list parameters;
  char buffer[100] = {'\0'}; //= (char*)malloc(512) ;
  uint64_t ret = 0;
  int total=0;
  const char* str = fmt;
  va_start(parameters, fmt);
  str = fmt;
  while(*str != '\0')
  {
    if(*str != '%')
    {
      buffer[total] = *str;
      str++;
      total++;

      continue;
    }
    str++;
    switch( *str )
    {
      case 'c': total+=user_putchar(va_arg(parameters, int), buffer,total);
                break;

      case 'd': total+=user_putint(va_arg(parameters, int),buffer,total);
                break;

      case 's': total+=user_puts(va_arg(parameters, char*), buffer,total);
                break;

      case 'x': total+=user_int2hex(va_arg(parameters, int), buffer,total);
                break;
      
      case 'l': if(*(++str)=='d')
                {
                  total+=user_putlong(va_arg(parameters, unsigned long), buffer,total);
                  break;
                }

      case 'p': total+=user_long2hex(va_arg(parameters, unsigned long), buffer,total);
                break;

      case '%': total+=user_putchar('%', buffer,total);
                break;
    }
    str++;
  }
  va_end(parameters);
  buffer[total] = '\0';
  total++;
  ret=__syscall2(SYSCALL_PUTS,(uint64_t)buffer,total);
  return ret;
}
