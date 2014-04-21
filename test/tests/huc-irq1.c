#include "huc.h"

/* Checks if context is saved and restored properly when calling a user
   vsync handler. */

const int a[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
int *b;

void v(void) __irq
{
	(void)b[10];	/* uses __ptr */
}

int main()
{
	unsigned int i;
	b = a;	/* need to use a non-const pointer to force use of __ptr */
	irq_add_vsync_handler(v);
	irq_enable_user_irq(IRQ_VSYNC);
	for (i = 0; i < 60000; i++) {
		/* Unrolling increases the chance of an interrupt
		   occurring between writing __ptr and using it. */
		if (b[0] != 0) abort();
		if (b[1] != 1) abort();
		if (b[2] != 2) abort();
		if (b[3] != 3) abort();
		if (b[4] != 4) abort();
		if (b[5] != 5) abort();
		if (b[6] != 6) abort();
		if (b[7] != 7) abort();
		if (b[8] != 8) abort();
		if (b[9] != 9) abort();
	}
	return 0;
}
