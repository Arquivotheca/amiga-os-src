/** chunk.c -- suck program gadget definitions and dispatch  **/

#include <exec/types.h>
#include <intuition/intuition.h>
#include "suck.h"

ULONG AllocMem();

#define MAX(A, B)	(((A)>(B))?(A):(B))

UBYTE *chunkReport(cs)
struct ChunkState *cs;
{
    static UBYTE b[SUCKMSGLEN];

    sprintf(b, "%ld chunks at %ld", cs->cs_count, cs->cs_size);
    return (b);
}

doGadg(g)
struct Gadget *g;
{
    ULONG	chunkfree = ckfreesi.LongInt;

    s.cs_size = MAX( 8, cksizesi.LongInt);

    switch (g->GadgetID)
    {
    case SUCKG:
	Forbid();
	chunkSuck(&s);
	chunkFree(&s, chunkfree);
	Permit();
	availMsg();
	suckMsg(chunkReport(&s));
	break;

    case FREEG:
	chunkFree(&s, FREEALL);
	availMsg();
	suckMsg("No chunks sucked");
	break;

    case AVAILG:
	availMsg();
	break;

    case CKSIZEG:
    case CKFREEG:
	break;

    default:
	suckMsg("weird gadget problem");
    }

}

chunkSuck(cs)
struct ChunkState *cs;
{
    struct ChunkHunk *node;

    /* allocate until no more, keep them on a list */
    while (node = (struct ChunkHunk *) AllocMem(cs->cs_size, 0))
    {
	/* stash size of node in ln_Name */
	cs->cs_count++;
	node->ch_size =  cs->cs_size;
	node->ch_next = cs->cs_next;
	cs->cs_next = node;
    }
}

chunkFree(cs, num)
struct ChunkState *cs;
ULONG	num;
{
    struct ChunkHunk *node;

    while (num-- && (node = cs->cs_next) )
    {
	cs->cs_count--;
	cs->cs_next = node->ch_next;
	FreeMem(node, node->ch_size);
    }
}

strncpy(to, from, n)
UBYTE *to;
UBYTE *from;
WORD	n;
{
    while ((n-- > 0) && (*to++ = *from++));
}

/* print suck status message */
suckMsg(str)
UBYTE *str;
{
    strncpy(suckmsgbuf, str, SUCKMSGLEN);
    spaceFill(suckmsgbuf, SUCKMSGLEN);
    PrintIText(window->RPort, &suckmsgt, LEFTBORD, ROW(5));
}

/* print out current memory availability */
availMsg()
{
    sprintf(availmsgbuf, "%ld", AvailMem(0));
    spaceFill(availmsgbuf, AVAILMSGLEN);
    PrintIText(window->RPort, &availmsgt, MIDLEFTBORD, ROW(2));
}

spaceFill(buf, buflen)
UBYTE	*buf;
WORD	buflen;
{
    WORD curpos = 0;

    while (curpos < (buflen - 1) && buf[curpos]) curpos++;
    /* buf[curpos] is either NULL or last byte */
    while (curpos < buflen) buf[curpos++] = ' ';
    buf[buflen - 1] = '\0';
}
