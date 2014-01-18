/**
* Implements the printf in kernel
* Final Project Code
* @author Abhiroop Dabral
**/

# include <stdarg.h>
# include <defs.h>
# include <common.h>	// for outb 
# include <stdio.h>

# define BUFLEN 1024			

char buf[BUFLEN]; 			// buffer to hold print arguments
int color = 0x07;
int x_axis = 0, y_axis = 0;
uint64_t video = 0xB8000;		// global variable pointing to video memory

void scroll();				// Scrolls the Video Buffer on Overflow
void update_cursor();			// Updates the screen
int putch(char);			// Writes a single character- end point of all other print functions
void reverse(char *buf, int len);
void d2hex(char *buf, uint64_t num);

int printf(const char *format, ...)
{
	va_list args;
	char pt, ch;
	char *hexstr = "0x";
	char *str = NULL;
	int i = 0, counter = 0;
	uint64_t num;
	
	va_start(args, format);
	pt = *format;

	for (pt = *format; pt != '\0'; i++, pt = *(format + i))
	{
		if (pt != '%')
		{
			putch(pt);
			counter++;
	
		}
		else if (pt == '%') {
			++i;
			pt = *(format + i);						// takes the format specifier after the % sign
			switch(pt)
			{
				case 'c':
					ch = (char)va_arg(args, int);
					putch(ch);
					counter++;
					break;
				
				case 'd':
					num = (uint64_t)va_arg(args, int);
					itoa(num, buf);
					counter += putstr(buf);
					break;
				
				case 's':
					str = (char *)va_arg(args, char *);
					counter += putstr(str);
					break;
					
				case 'p':
					num = (uint64_t)va_arg(args, int);
					d2hex(buf, num);
					putstr(hexstr);
					counter += putstr(buf);
					counter += 2;
					break;
				case 'x':
					num = (uint64_t)va_arg(args, long int);
					d2hex(buf, num);
					putstr(hexstr);
					counter += 2;
					counter += putstr(buf);
					break;
				default:
					break;
					/* TOODO */		
			}
		  }
	}
	va_end(args);
	return counter;
}					

/* This function implements putchar() */
int putch(char ch)
{
	 uint64_t cur = ((y_axis * 80) + x_axis)*2 + video;
	 char *doit = (char *)cur;		// Video Memory

	if (ch == '\n')
	{
		x_axis = 0;
		y_axis++;
//		return 0;
//		cur = ((y_axis * 80) + x_axis)* 2 + video;
		if (y_axis >= 25)
		{
			scroll();
		}
		update_cursor();
		return 0;

	}
	else if (ch == '\t')
	{
		if ((x_axis + 4) < 79)
		{
			x_axis += 4;
		}
		else {
			x_axis = 0;
			y_axis++;
		}
	}
	else {
		if (x_axis < 79)
		{
			x_axis++;
		}
		else {
			x_axis = 0;
			y_axis++;
		}
  	}

	if (y_axis >= 25)
	{
		scroll();
	}
	
	cur = ((y_axis * 80) + x_axis)* 2 + video;
	doit = (char *)cur;

	*doit++ = ch;
	*doit++ = color;
	update_cursor();
	return 0;
}


/* This function implements puts() */			
int putstr(char *st)
{
	int count = 0;	
	while (*st != '\0')
	{
		putch(*st);
		st++;
		count++;
	}
	return count;
}


/* This function implements itoa() */
void itoa(uint64_t num, char * bu)
{
	int i = 0;						// Check for signed no. If ON num is negative
	if (num < 0)
	{
//		flag = 1;
		bu[0] = '-';
		i++;
	}
	if (num == 0)
	{
		bu[0] = '0';
	}
	while (num > 0)
	{
		bu[i++] = '0' + num % 10;
		num /= 10;
	}

	buf[i] = '\0';
	reverse(buf, i);
}


/* This function reverses the number in buf */
void reverse(char * buf, int len)
{
	char tmp;
	int i = 0, j = len - 1;
	
	while( buf[i] != '\0' && i < j)
	{
		if (buf[i] == '-')
		{
			i++;
//			j++;
			continue;
		}
		
		tmp = buf[i];
		buf[i] = buf[j];
		buf[j] = tmp;
		i++;
		j--;
	}
}

/* TO convert decimal to HEX string */
void d2hex(char *buf, uint64_t num)
{
	const char *hex = "0123456789ABCDEF";
	int i = 0;
	while (num > 0)
	{
		buf[i++]  = hex[(num & 0xF)];
		num >>= 4;
	}
	buf[i] = '\0';				//EOS
	reverse(buf, i);
}
	
/* Clear the screen */
void clear_screen()
{
	int i;
	int len_vid = (25*80);
	char * doit = (char *) video;
	for ( i = 0; i < len_vid; i++)
	{
		*doit++ = ' ';
		*doit++ = color;
	}
	x_axis = 0, y_axis = 0;
	update_cursor();
}	
/***********Write Call****************/
int write(int fd,volatile char* str, int n)
{
	int i = 0;
	if(fd == 1 || fd==2)
      	{
        	for(i =0 ; i < n && str[i] != '\0' ; i++)
	      	{
	        	putch(str[i]);
		}
	}
		update_cursor();
		return i+1;
}

/* This function updates the cursor on the screen  */
/* Refrenced from http://wiki.osdev.org/Text_Mode_Cursor */
void update_cursor()
{
	unsigned short cursor = (y_axis * 80) + x_axis;
	outb(0x3D4, 0x0F);	// Cursor LOW port to VGA index reg
	outb(0x3D5, (unsigned char)(cursor & 0xFF));
	outb(0x3D4, 0x0E);	// Cursor HIGH port to VGA Index reg
	outb(0x3D5, (unsigned char)((cursor>>8) & 0xFF));
}

void scroll()
{
	int i = 0;
	volatile char *doit = (volatile char*)video;

	for (i = 80; i < (25 * 80); i++)
	{
		*(doit) = *(doit + 160);
		doit++;
		*doit = color;
		*doit++;
	}

	for (i = 0; i < 80; i++)
	{
		*(doit) = ' ';
		doit++;
		*doit = color;
		*doit++;
	}	

	x_axis = 0;
	y_axis = 24;
//	update_cursor();
}
