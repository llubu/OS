#include <stdarg.h>
#include <stdio.h>
#include <defs.h>

#include<sys/irq.h>

int scanf(const char *format, uint64_t ptr)
{
	int i=0;
	if (newline)
	{
		char* str = (char*)ptr;
		while(kbuf[i] != '\n')
		{
			str[i]=kbuf[i];
			i++;
		}
		str[i] = '\0';
		i++;
		int index = i;
		for(i; i < kbuf_index; i++)
			kbuf[i-index] = kbuf[i];
		kbuf_index = kbuf_index-index;
		newline = 0;
		return 1;
	}
	return 0;
}

