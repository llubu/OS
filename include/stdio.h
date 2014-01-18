#ifndef _STDIO_H
#define _STDIO_H

# include <defs.h>

int printf(const char *format, ...);
int scanf(const char *format, uint64_t pt);

int putch(char);			// Writes a single character- end point of all other print functions
void itoa(uint64_t num, char *bu);
void clear_screen();
int putstr(char * str);			// prints string
/************************************/

uint64_t sys_putint(int a);

/************************************/

//int user_putint(int value, char* buffer, int total);
//int user_long2hex(unsigned long value, char* buffer, int total);
//int user_putlong(unsigned long value, char* buffer, int total);
//int user_putchar(char a, char* buffer , int total);
//int user_int2hex(int value, char* buffer, int total);
//int user_puts(char* str, char* buffer, int total);
uint64_t u_printf(const char *format, ...);
uint64_t u_scanf(const char *format, uint64_t ptr);
uint64_t stder(const char* fmt, ...) ;
int write(int fd, volatile char* str, int n);



/************************************/




extern uint64_t video;
#endif












