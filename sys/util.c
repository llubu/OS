# include <stdio.h>

void clear_buf(uint64_t *pt, uint64_t size)
{
	uint64_t i = 0;

	if (pt == NULL)
	{
		printf("\n Error: NULL pointer passed");
		return;
	}

	while (i < size)
	{
		pt[i] = 0;
		++i;
	}
}

