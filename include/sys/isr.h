#ifndef _ISR_H
#define _ISR_H

typedef struct registers
{
     uint64_t r15, r14, r13, r12, r11, r10, r9, r8, rbp, rdi, rsi, rdx, rcx, rbx, rax; // Pushed by pushq i.e. all general purpose registers
     unsigned char err_code;    // Interrupt number and error code (if applicable)
     //uint64_t ds;                  // Data segment selector
     //uint64_t rip, cs, eflags, usersp; // Pushed by the processor automatically.
} registers_t; 


struct regis
{
	uint64_t r15;
	uint64_t r14;
	uint64_t r13;
	uint64_t r12;
	uint64_t r11;
	uint64_t r10;
	uint64_t r9;
	uint64_t r8;
//	uint64_t rsp;
	uint64_t rbp;
	uint64_t rdi;
	uint64_t rsi;
	uint64_t rdx;
	uint64_t rcx;	
	uint64_t rbx;	
	uint64_t rax;
};

typedef struct regis streg;



#endif
