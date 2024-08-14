#include <graphics/gfx.h>

#define PASSRECT(x) x
/*#define PASSRECT(x) *((ULONG *) &x),*(((ULONG *) &x)+1) */

myfunc()
{
	struct Rectangle r;
	f1(PASSRECT(r));
}
