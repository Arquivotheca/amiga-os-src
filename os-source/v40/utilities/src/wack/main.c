
/***********************************************************************
*
*			   G R A N D W A C K
*
************************************************************************
*
*	Copyright (C) 1985, Commodore-Amiga. All rights reserved.
*
************************************************************************
*
*   Source Control:
*
*	$Id: main.c,v 1.12 92/08/27 12:08:43 peter Exp $
*
*	$Locker:  $
*
*	$Log:	main.c,v $
 * Revision 1.12  92/08/27  12:08:43  peter
 * Noted new shortcuts "task" and "proc".
 * 
 * Revision 1.11  92/08/20  17:57:06  peter
 * Bumped version banner.
 * 
 * Revision 1.10  92/05/26  14:49:12  peter
 * Bumped version to 1.91.
 * 
 * Revision 1.9  91/05/29  08:47:44  peter
 * Bumped version number to 1.9.
 * 
 * Revision 1.8  91/04/24  22:25:13  peter
 * Added "!" to help list.
 * 
 * Revision 1.7  91/04/24  20:49:47  peter
 * Bumped rev to 1.8.
 * Updated help to be complete.
 * Added LastWindow and SpareAddr pointers.
 * No longer exits if billboard not found.
 * 
 * Revision 1.6  90/05/30  11:08:35  kodiak
 * change where function to $ key and make @ alphanumeric
 * 
 * Revision 1.5  90/02/21  11:46:50  kodiak
 * 1.6: rename dbase to dosbase, and add dos device listing to it
 * 
 * Revision 1.4  89/11/22  16:23:59  kodiak
 * 1.5: Forbid()/Permit() using BGACK hold; "faster" symbol loading option.
 * 
 * Revision 1.3  88/01/21  18:47:56  jimm
 * changed the title/version
 * 
 * Revision 1.2  88/01/21  13:37:17  root
 * kodiak's copy of jimm's version, snapshot jan 21
 * 
*
***********************************************************************/

#define title "\n  Wack ( Sun Version 1.93 (Kickstart V39.100+) )\n"
#define EXECREV 144

typedef unsigned short uword;

#include "std.h"
#include <sys/file.h>
#include <stdio.h>
#include <sgtty.h>
#include "symbols.h"

#define DMA_DEV "/dev/bb0"


#define     min(x,y)    ((x) < (y) ? (x) : (y))
#define     max(x,y)    ((x) > (y) ? (x) : (y))


char *program;
FILE  *SymFile;

short remote = 1;
short dd = -1;
short td = 0;

extern int errno;

struct sgttyb newtty;
struct sgttyb oldtty;
extern struct Symbol *BindValue1();

short Faster;
short Backup;
short LastToken;
long Accum;
long holdRTS;
long LastNum;
long CurrAddr=0;
/* Swappable address ptr.  Also, some commands set this up. "!" to swap */
long SpareAddr=0;
long DisasmAddr=0;
long HoldNum=0;
long FrameSize=16;
char *InputPtr;

char stream[100];
char *streamp;

short Memory[1000];
short flags;

long values[20];

long LastWindow = 0;

long LastTask = 0;

char NextChar()
{
    return(*streamp++);
}

char PeekChar()
{
    return(*streamp);
}

quit( reason ) 
    char *reason; 
{
    fprintf (stderr, "%s: terminated for %s\n", program, reason);
    if (dd != -1) {
	fprintf (stderr, "closed %s\n", DMA_DEV);
	close (dd);
    }

    ioctl (td, TIOCSETP, &oldtty);
    exit (1);
}

conclude()
{
    if (dd != -1) {
/*	fprintf (stderr, "closed %s\n", DMA_DEV); */
	close (dd);
    }
    ioctl (td, TIOCSETP, &oldtty);
    puts("");
    exit (0);
}

Conclude()
{
    conclude ();
}

benasty()
{
    puts ("\n   break the ESC habit, use ctrl-d to exit");
    flags = 0;
}

error (s)

    char *s;
{
    printf("\nerror: %s\n");
    exit(1);
}

errmsg(s,n)
    char *s;
    int   n;
{
    printf( s, n);
    getchar();
}

getstream()
{
    char *s = stream;

    while ((*s++ = getchar ()) != 10) ;
    *s = 0;
}


BOOL EOFGetLong (word)
ULONG *word;
{
    int     c;
    char    bytes[sizeof (ULONG)];
    ULONG * word_value = (ULONG *) bytes;
    SHORT i;

    for (i = 0; i < sizeof (LONG); i++) {
	if ((c = getc (SymFile)) == EOF) {
	    return TRUE;
	}
	bytes[i] = c;
    }
    *word = *word_value;
    return FALSE;
}

ULONG GetLong ()
{
    ULONG word;

    if (EOFGetLong (&word)) {
	quit ("Unexpected EOF!");
    }
    return word;
}



SuckHunks()
{
    long    hunk = 0;
    long    offset = 0;
    long    size = 0;
    long    i = 0;
    char    basename[20];

    while ((fscanf (SymFile, "%d %x %x\n", &hunk, &offset, &size))==3) {
	sprintf (basename, "`hunk_%lx", hunk);
/*	printf ("%s = %x\n", basename, offset); */
	BindValue1 (basename, ACT_BASE, offset);
    }
    puts (" ");
}

Help()
{
    puts ("\nKeystrokes:");
    puts ("    .   forward a frame         ,   backward a frame");
    puts ("    >   forward a word          <   backward a word");
    puts ("    +n  forward n bytes         -n  backward n bytes");
    puts ("    n   set position to n       :n  set frame size to n");
    puts ("    [   indirect thru current   ]   exdirect to previous");
    puts ("    {   indirect bptr thru current");
    puts ("    ^   set breakpoint (count)  #   toggle trace mode");
    puts ("    ;   disassemble frame       $   symbol name for current");
    puts ("    =   assign value");
    puts ("    !   swap current address pointer with spare");
    puts ("Command Symbols:");
    puts ("    quit -- return to shell");
    puts ("    nodes -- print list nodes");
    puts ("    tasks -- show all tasks");
    puts ("    showtask (=task) -- format current frame as a task");
    puts ("    showprocess (=proc) -- format current frame as a process");
    puts ("    attach -- attach symbolmap to task at current frame");
    puts ("    devices (=devs) -- show all devices");
    puts ("    libraries (=libs) -- show all libraries");
    puts ("    resources (=rsrcs) -- show all resources");
    puts ("    memory (=mem) -- show memory allocation lists");
    puts ("    regions -- show memory regions");
    puts ("    modules (=mods) -- show system modules");
    puts ("    ints -- show all interrupts");
    puts ("    ports -- show all public ports");
    puts ("    symbols -- show all symbols");
    puts ("    bindhunks -- attach symbolmap to current segment list");
    puts ("    findrom (=rom) -- show ROM-like addresses starting at current frame");
    NewLine();
    puts ("    keys -- show key bindings");
    puts ("    dec -- convert to decimal (eg. \"(dec abcd)\")");
    puts ("    hex -- convert to hexadecimal (eg. \"(hex 1234)\")");
    puts ("    bptr -- convert BPTR to hexadecimal (eg. \"(bptr abcd)\")");
    NewLine();
    puts ("    Also see \"newhelp\"");

    flags = 0;
}


Facts()
{
    puts ("  For Amiga Internal Use\n");
    /* jimm: */
    puts ("  type newhelp for intuition and graphics commands\n");
}


main(argc, argv)
    int argc;
    char *argv[];
{
    char    fileName[1024];
    int     tkn;
    short   reason;

    program = *argv++;

    printf (title);
    Facts ();

    if ((dd = open (DMA_DEV, O_RDWR, 0)) < 0) {
	fprintf (stderr, "%s: cannot open %s\n", program, DMA_DEV);
	dd = (-1);
	remote = 0;
	fprintf (stderr, "-- Billboard did not open! --\n");
#if 0	/* We're just like wacka now */
	exit (1);
#endif
    }

    ioctl (td, TIOCGETP, &oldtty);
    ioctl (td, TIOCGETP, &newtty);
    newtty.sg_flags |= CBREAK;
    newtty.sg_flags &= ~ECHO;

    if (ioctl (td, TIOCSETP, &newtty) != 0) {
	fprintf (stderr, "%s: cannot set CBREAK mode\n", program);
	perror (program);
	exit (1);
    }

    TryIt ();

    if (Faster = (argc > 3))
	printf("  faster\n");

    if (argc-- > 1) {
	sscanf (*argv++, "%s", fileName);
	if ((SymFile = fopen (fileName, "r")) == NULL) {
	    quit ("not opening symbol file");
	}
	SuckSymbols ();
    }

    if (argc-- > 1) {
	sscanf (*argv++, "%s", fileName);
	if ((SymFile = fopen (fileName, "r")) == NULL) {
	    quit ("not opening hunk file");
	}
	SuckHunks ();
	FixupTopMap ();
    }

    clearBPT ();

    puts ("  ready");
    Interp();
}

NewLine()
{
    puts (" ");
}
