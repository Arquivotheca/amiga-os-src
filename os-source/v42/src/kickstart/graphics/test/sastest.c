#include <exec/types.h>

WORD a,b;

UWORD coolmul(void)
{
	return((UWORD) ((((LONG) a)*((LONG) b))>>15));
}
