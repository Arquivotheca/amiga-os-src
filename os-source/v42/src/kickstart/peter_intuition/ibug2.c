;/*
sc RESOPT DBG=LINE NOSTKCHK NOICONS OPT ibug2
omd ibug2.o ibug2.c >ram:cout
quit
*/

/* Optimizer turns two move.l's into dbf loop of 8 move.b's why??? */

#include <exec/types.h>

struct IBox
{
    WORD Left;
    WORD Top;
    WORD Width;
    WORD Height;
};

struct Point
{
    WORD X;
    WORD Y;
};

#define cast(A) (p = *((struct Point *) (A)))
extern struct IBox	oldbox;

void foo( void )
{
    struct IBox	newbox;
    struct Point p;

    newbox = oldbox;

    bar( cast(&newbox) );
}
