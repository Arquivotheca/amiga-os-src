#include <intuition/intuition.h>
/* include "standard.h" */

extern	void	puts();
extern	struct	Window *Window;
extern	ULONG	IMClass;
extern	struct	IntuiMessage *message;
extern	int	stop_test;		/* Not 0 means <ctrl-c> typed */
extern	int	abort;			/* Not 0 means exit program */
extern	struct MsgPort  consoleMsgPort;
extern	ULONG    WakeUpBit, WaitMask;
int	inKey, kcnt, scnt;
char	*sbp, sbuf[80], ch;

extern	void	putint();		/* Forward definition */
extern	void	putlong();
extern	void	gets();
extern	char	getch();
	char	sup_cr=0;		/* If not zero, getch() doesn't */
					/*	echo carriage returns */

int getint(def)				/* input integer from console */
	int def;			/* value returned if empty string */
{
	char num_input[80];		/* work area for inputting numbers */
	int inp;

	sup_cr++;		/* Don't echo carriage return */
	gets(num_input);		/* Get input from keyboard */
	sup_cr = 0;
		/* If just carriage return enterred */
	if ((sscanf(num_input,"%d",&inp) != 1)) /* Put input value in inp */
	{
		inp = def;
		putint(inp);		/* Display default */
	}
	puts("\n");
	return(inp);		/*	and return it as value */
}
	
int getlong(def)			/* input integer from console */
	long def;			/* value returned if empty string */
{
	long inp;
	char num_input[80];		/* work area for inputting numbers */

	sup_cr++;		/* Don't echo carriage return */
	gets(num_input);		/* Get input from keyboard */
	sup_cr = 0;

		/* If just carriage return enterred */
	if ((sscanf(num_input,"%ld",&inp) != 1)) /* Put input value in inp */
	{
		inp = def;
		putlong(inp);		/* Display default */
	}
	puts("\n");
	return(inp);		/*	and return it as value */
}
	
/*
 * Output a string to the main screen
 */
void puts(str)
register char *str;
{
	register char c;
	register char *cp;
	char ostr[80];

	cp = ostr;
	while ((c = *str) != 0)	/* Replace any '\n's with '\n\r' */
	{
		*cp++ = *str++;
		if (c == '\n')
			*cp++ = '\r';
	}
	*cp = 0;	/* Mark end of string */
	CDPutStr(ostr);
}

/*
 * Output an integer
 */
void putint(i)
register int i;
{
	char ostr[20];
	
	sprintf(ostr,"%d",i);
	CDPutStr(ostr);
}

/*
 * Output a long
 */
void putlong(l)
register long l;
{
	char ostr[20];
	
	sprintf(ostr,"%ld",l);
	CDPutStr(ostr);
}

/*
 * Output a byte in hex
 */
void putbhex(c)
register int c;
{
	UBYTE ostr[2];
	
	sprintf(ostr,"%2x",c);
	CDPutStr(ostr);
}

/*
 * Output a nibble in hex
 */
void puthhex(c)
register char c;
{
	char ostr[2];
	
	sprintf(ostr,"%1x",c);
	CDPutStr(ostr);
}

/*
 * Output an integer in hex
 */
void putihex(i)
register int i;
{
	char ostr[4];
	
	sprintf(ostr,"%4x",i);
	CDPutStr(ostr);
}

/*
 * Output a long in hex
 */
void putlhex(l)
register long l;
{
	char ostr[8];
	
	sprintf(ostr,"%8x",l);
	CDPutStr(ostr);
}

void gets(cp)
char *cp;
{
	register char c;
	register char *s;
	
	s = cp;
#ifdef junk
/* LCE */ puts("gets: abort = ");
	putint(abort);
	puts("\n");
#endif
	while (((c = getch()) != '\r') && (c != '\003') && !abort)
	{
		if (c == 8) {
			if (s <= cp)
				continue;
			s--;
			puts("\b \b");
			continue;
		}
		*s++ = c;
	}
	*s = '\0';
#ifdef LCE
/* LCE */ puts("gets: returns = ");
	puts(cp);
	puts("\n");
#endif
}

char getch()
{
	char c;
	if (!abort) do {
	    WakeUpBit = Wait(WaitMask);	/* Wait for char. typed */
	    TimeCheck(0);		/* Test for timeout event */
	    while( message = (struct IntuiMessage *)GetMsg(Window->UserPort)) {
		IMClass = message->Class;        /* Correspond to IDCMP flags */
		ReplyMsg(message);         /* reply when info is obtained */
		abort = abort || ((IMClass & CLOSEWINDOW) !=0);
	    }
	    if(WakeUpBit & (1<<consoleMsgPort.mp_SigBit)) /* If key typed */
	    {
		c = CDGetChar();	/* get the character */
		if ((c >= ' ') && ((c != '\r') || !sup_cr))
			CDPutChar(c);		/* Echo it to the screen */
		stop_test = stop_test || (c == '\003');
		return(c);		/* Return it to caller */
	    }
	}
	while (!stop_test && !abort);
	return ('\003');
}
