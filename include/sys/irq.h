#include "isr.h"
# include <sys/task_switch.h>
#define PIC_EOI   0x20    /* End-of-interrupt command code */
#define PIC1      0x20
#define PIC2      0xA0

void init_pic();
//void irq_handler_0(registers_t regs);
void irq_handler_0(GREG regs);
void PIC_sendEOI(unsigned char irq);
void init_timer();
void printtime(unsigned char hour, unsigned char minute, unsigned char seconds);
extern volatile unsigned char kbuf[128];
extern volatile int kbuf_index;
extern volatile int newline;
extern volatile uint64_t sec ;
