/* graphics  kernel routines */
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/interrupts.h>
#include <hardware/intbits.h>
#include <graphics/graphint.h>


int ttskasm();

AddTOF(i,p,a)
register struct Isrvstr *i;
int (*p)();
int a;
{
	i->code = ttskasm;
	i->Iptr = i;
	i->ccode = p;
	i->Carg = a;
	AddIntServer(INTB_VERTB,i);
}

RemTOF(i)
struct Isrvstr *i;
{
	RemIntServer(INTB_VERTB,i);
}


waitbeam(b)
int b;
{
	while (b>VBeamPos());
}

