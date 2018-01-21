# include <stdio.h>
# include <common.h>

# include <sys/idt.h>
# include <sys/isr.h>
# include <sys/irq.h>
# include <sys/task_switch.h>

extern void _set_k_ptable_cr3(uint64_t);
unsigned char second;
unsigned char minute;
unsigned char hour;
unsigned char day;
unsigned char month;
unsigned int year;
volatile unsigned char kbuf[128] = {0};
volatile int kbuf_index = -1;
volatile int newline = 0;

uint64_t tick = 0;
volatile uint64_t sec = 0;
void read_rtc();
enum
{
	cmos_address = 0x70,
	cmos_data    = 0x71
};

int get_update_in_progress_flag()
{
	outb(cmos_address, 0x0A);
	return (inb(cmos_data) & 0x80);
}

unsigned char get_RTC_register(int reg)
{
	outb(cmos_address, reg);
	return inb(cmos_data);
}

unsigned char keyboard_map[128] =
{
	0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
	'9', '0', '-', '=', '\b',	/* Backspace */
	'\t',			/* Tab */
	'q', 'w', 'e', 'r',	/* 19 */
	't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
	0,			/* 29   - Control */
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
	'\'', '`',   0,		/* Left shift */
	'\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
	'm', ',', '.', '/',   0,			/* Right shift */
	'*',
	0,	/* Alt */
	' ',	/* Space bar */
	0,	/* Caps lock */
	0,	/* 59 - F1 key ... > */
	0,   0,   0,   0,   0,   0,   0,   0, // 67
	0,	/* < ... F10 */
	0,	/* 69 - Num lock*/
	0,	/* Scroll Lock */
	0,	/* Home key */
	0,	/* Up Arrow */
	0,	/* Page Up */
	'-',
	0,	/* Left Arrow */
	0,
	0,	/* Right Arrow */
	'+',
	0,	/* 79 - End key*/
	0,	/* Down Arrow */
	0,	/* Page Down */
	0,	/* Insert Key */
	0,	/* Delete Key */
	0,   0,   0,
	0,	/* F11 Key */
	0,	/* F12 Key */
	0,	/* All other keys are undefined */ //88
	'!', '@', '#', '$', '%', '^', '&', '*',	/* 9 */
	'(', ')', '_', '+'
};

unsigned char keyboard_map_shift[128] =
{
	0,  27,
	'!', '@', '#', '$', '%', '^', '&', '*',	/* 9 */
	'(', ')', '_', '+',
	'\b',	/* Backspace */
	'\t',			/* Tab */
	'Q', 'W', 'E', 'R',	/* 19 */
	'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',	/* Enter key */
	0,			/* 29   - Control */
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',	/* 39 */
	'\"', '~',   0,		/* Left shift */
	'|', 'Z', 'X', 'C', 'V', 'B', 'N',			/* 49 */
	'M', '<', '>', '?',   0,				/* Right shift */
	'*',
	0,	/* Alt */
	' ',	/* Space bar */
	0,	/* Caps lock */
	0,	/* 59 - F1 key ... > */
	0,   0,   0,   0,   0,   0,   0,   0, // 67
	0,	/* < ... F10 */
	0,	/* 69 - Num lock*/
	0,	/* Scroll Lock */
	0,	/* Home key */
	0,	/* Up Arrow */
	0,	/* Page Up */
	'-',
	0,	/* Left Arrow */
	0,
	0,	/* Right Arrow */
	'+',
	0,	/* 79 - End key*/
	0,	/* Down Arrow */
	0,	/* Page Down */
	0,	/* Insert Key */
	0,	/* Delete Key */
	0,   0,   0,
	0,	/* F11 Key */
	0,	/* F12 Key */
	0,	/* All other keys are undefined */ //88
};

void PIC_sendEOI(unsigned char irq)
{
	if(irq >= 8)
	{
		outb(PIC2,PIC_EOI);
	}
	outb(PIC1,PIC_EOI);
}

//Remapping PICs
void init_pic()
{
	printf("PIC INIT \n");
	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	outb(0x21, 0x20);
	outb(0xA1, 0x28);
	outb(0x21, 0x04);
	outb(0xA1, 0x02);
	outb(0x21, 0x01);
	outb(0xA1, 0x01);
	outb(0x21, 0x0);
	outb(0xA1, 0x0);
}

void init_timer()
{
	uint16_t div;
	unsigned char low, high;

	// divisr = (1193180/500);
	div = 59659u;

	outb(0x43u, 0x36u);
	low = (unsigned char)(div & 0xffu);
	high = (unsigned char)( (div >> 8u) & 0xffu );

	outb(0x40u, low);
	outb(0x40u, high);
}

volatile uint32_t schd_ON;

// timer_handler
void irq_handler_0(GREG regs)
{
	tick++;
	//	regs_sch = &regs;
	if (0 == (tick % 20u))
	{
		read_rtc();
		sec++;
		if(0 == (tick % 200u))
			schd_ON = 1;
	}
	outb(0x20u, 0x20u);

	if((schd_ON  && running))
	{
		schd_ON = 0;
		schedule1();
	}
}

int shift_ON = 0;

// keyboard_handler
void irq_handler_1(registers_t regs)
{
	unsigned char scancode;
	unsigned char status;

	status = inb(0x64);
	scancode = status;

	// Read from the keyboard's data buffer
	scancode = inb(0x60);

	if (scancode & 0x80)
	{
		if(scancode == 0x2A)
			shift_ON = 0;
	}
	else
	{
		if(scancode == 0x2A)
		{
			shift_ON = 1;
		}
		else
		{
			if(shift_ON)
			{
				putch(keyboard_map_shift[scancode]);
				kbuf_index++;
				kbuf[kbuf_index] = keyboard_map_shift[scancode];
				kbuf_index = kbuf_index%128;
				if(kbuf[kbuf_index] == '\n')
					newline  = 1;
				shift_ON = 0;
			}
			else
			{
				putch(keyboard_map[scancode]);
				kbuf_index++;
				kbuf[kbuf_index] = keyboard_map[scancode];
				kbuf_index =( kbuf_index % 128u);
				if(kbuf[kbuf_index] == '\n')
					newline = 1;
			}
		}
	}
	outb(0xA0, 0x20);
	outb(0x20, 0x20);
}

void read_rtc()
{
	unsigned char last_second;
	unsigned char last_minute;
	unsigned char last_hour;
	unsigned char last_day;
	unsigned char last_month;
	unsigned char last_year;
	unsigned char registerB;

	// Make sure an update isn't in progress
	while (get_update_in_progress_flag());
	second = get_RTC_register(0x00);
	minute = get_RTC_register(0x02);
	hour = get_RTC_register(0x04);
	day = get_RTC_register(0x07);
	month = get_RTC_register(0x08);
	year = get_RTC_register(0x09);

	do
	{
		last_second = second;
		last_minute = minute;
		last_hour = hour;
		last_day = day;
		last_month = month;
		last_year = year;

		while (get_update_in_progress_flag());
		second = get_RTC_register(0x00);
		minute = get_RTC_register(0x02);
		hour = get_RTC_register(0x04);
		day = get_RTC_register(0x07);
		month = get_RTC_register(0x08);
		year = get_RTC_register(0x09);
	}
	while((last_second != second) || (last_minute != minute)
			|| (last_hour != hour) || (last_day != day)
			|| (last_month != month) || (last_year != year));

	registerB = get_RTC_register(0x0B);

	// Convert BCD to binary values if necessary

	if (!(registerB & 0x04)) {
		second = (second & 0xf) + ((second / 16) * 10);
		minute = (minute & 0xf) + ((minute / 16) * 10);
		hour = ((hour & 0xf) + (((hour & 0x70) / 16) * 10))
			| (hour & 0x80);
		day = (day & 0xf) + ((day / 16) * 10);
		month = (month & 0xf) + ((month / 16) * 10);
		year = (year & 0xf) + ((year / 16) * 10);
	}
	printtime(hour,minute,second);
}

void printtime(unsigned char hour, unsigned char minute,
		unsigned char seconds)
{
	int color = 0x07;
	volatile char *doit = (volatile char*)(video + 0xf90);
	char a;

	if(hour >= 4)
		hour -= 4;
	else
		hour += 20;

	a = ((hour / 10) + '0');
	*(doit++) = a;
	*doit++ = color;
	a = ((hour % 10) + '0');
	*(doit++) = a;
	*doit++ = color;

	a = ':';
	*(doit++) = a;
	*doit++ = color;

	a = ((minute/10) + '0');
	*(doit++) = a;
	*doit++ = color;
	a = ((minute % 10) + '0');
	*(doit++) = a;
	*doit++ = color;

	a = ':';
	*(doit++) = a;
	*doit++ = color;

	a = ((seconds / 10) + '0');
	*(doit++) = a;
	*doit++ = color;
	a = ((seconds % 10) + '0');
	*(doit++) = a;
	*doit++ = color;
}

