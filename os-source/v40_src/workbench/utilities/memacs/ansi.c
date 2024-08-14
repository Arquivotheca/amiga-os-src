/*
 * The routines in this file
 * provide support for ANSI style terminals
 * over a serial line. The serial I/O services are
 * provided by routines in "termio.c". It compiles
 * into nothing if not an ANSI device.
 */
#include	"ed.h"

#if	ANSI

#if AMIGA
#define	NROW	50			/* Screen size. or 21  */
#define	NCOL	80			/* Edit if you want to */
#else
#define	NROW	24			/* Screen size. or 21  */
#define	NCOL	80			/* Edit if you want to */
#endif

#define	BEL	0x07			/* BEL character.		*/
#define	ESC	0x1B			/* ESC character.		*/

#if 0
extern	int	ttopen();		/* Forward references.		*/
extern	int	ttgetc();
extern	int	ttputc();
extern	int	ttflush();
extern	int	ttclose();
#endif

void ansimove(row, col)
int row,col;
{
	ttputc(ESC);
	ttputc('[');
	ansiparm(row+1);
	ttputc(';');
	ansiparm(col+1);
	ttputc('H');
}

void ansieeol()
{
	ttputc(ESC);
	ttputc('[');
	ttputc('K');
}

void ansieeop()
{
	ttputc(ESC);
	ttputc('[');
	ttputc('J');
}

void ansibeep()
{
	ttputc(BEL);
	ttflush();
}

void ansiparm(n)
 int	n;
{
	 int	q;

	q = n/10;
	if (q != 0)
		ansiparm(q);
	ttputc((n%10) + '0');
}

#endif

void ansiopen()
{
#if	V7
	 char *cp;
	char *getenv();

	if ((cp = getenv("TERM")) == NULL) {
		puts("Shell variable TERM not defined!");
		exit(1);
	}
	if (strcmp(cp, "vt100") != 0) {
		puts("Terminal type not 'vt100'!");
		exit(1);
	}
#endif
	ttopen();
}

/*
 * Standard terminal interface
 * dispatch table. Most of the fields
 * point into "termio" code.
*/

TERM	term	= {
	NROW-1,
	NCOL,
	(int)ansiopen,
	(int)ttclose,
	(int)ttgetc,
	(int)ttputc,
	(int)ttflush,
	(int)ansimove,
	(int)ansieeol,
	(int)ansieeop,
	(int)ansibeep
};

