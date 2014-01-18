/* This header file contains the definations of some common functions used in the kernel code */
#ifndef _COMMON_H
#define _COMMON_H

/* outb and inb both sourced from http://wiki.osdev.org/Inline_Assembly/Examples#I.2FO_access */

inline void outb(unsigned short port, unsigned char val)
{
	__asm volatile("outb %0, %1"
			: : "a"(val), "Nd"(port) );
}

inline unsigned char inb(unsigned short port)
{
	unsigned char ret;
	__asm volatile( "inb %1, %0"
			: "=a"(ret) : "Nd"(port) );
	return ret;
}

#endif


