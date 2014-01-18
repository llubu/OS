#ifndef _IDT_H
#define _IDT_H

#include <defs.h>

/* adapted from Chris Stones, shovelos */

#define IDT_IST           (0x0000)  /*** code segment descriptor ***/
#define IDT_RESERVED      (0x0000)  /*** data segment descriptor ***/
#define IDT_TYPE          (0x0E00)  /*** conforming ***/
#define IDT_ZERO          (0x0000)  /*** conforming ***/
#define IDT_DPL0          (0x0000)  /*** descriptor privilege level 0 ***/
#define IDT_DPL1          (0x2000)  /*** descriptor privilege level 1 ***/
#define IDT_DPL2          (0x4000)  /*** descriptor privilege level 2 ***/
#define IDT_DPL3          (0x6000)  /*** descriptor privilege level 3 ***/
#define IDT_P             (0x8000)  /*** present ***/

void reload_idt();

#endif
