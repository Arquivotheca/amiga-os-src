/* -----------------------------------------------------------------------
 * 
 *
 * $Locker:  $
 *
 * $Id: password.c,v 2.2 93/02/22 17:02:16 bj Exp $
 *
 * $Revision: 2.2 $
 *
 * $Log:	password.c,v $
 * Revision 2.2  93/02/22  17:02:16  bj
 * binary 2.9
 * 
 * Fixes a bug where hitting an EOF (control backslash)
 * at the password prompt would cause the program to go
 * into an infinite loop.
 * 
 * Revision 2.1  92/10/30  11:29:50  bj
 * Binary 2.3
 *  
 * No code changes. Added RCS header. Removed unused debug code.
 * 
 *
 * $Header: Hog:Other/inet/src/c/ftp/RCS/password.c,v 2.2 93/02/22 17:02:16 bj Exp $
 *
 *------------------------------------------------------------------------
 */
/****** socket/getpass  *************************************************
*
*	NAME
*		getpass - Get a password entry from the user 
*
*	SYNOPSIS
*		password = getpass( prompt )
*
*		char *getpass( char * )
*
*	FUNCTION
*		Gets a password string from the user. 
*
*
*	INPUTS
*		prompt - A string which will prompt the user.
*
*	RESULT
*		password - A pointer to the string.
*
*	NOTES
*		Control-X clears the line and starts over.
*
*	BUGS
*		(1) There are currently no checks on the length (max == 32) of the
*		    string entered by the user. 
*
********************************************************************
*
* Simple passwd system for the Amiga.  The password encoding scheme
* needs improvement, but due to US DOD export requirements and the
* AT&T patent on the Unix scheme the obvious choice isn't available.
*
*/

#include <stdarg.h>
#include <sys/param.h>
#include <pwd.h>
#include <stdio.h>
#include <signal.h>

#define OSIZE	12

int	_pw_stayopen;

/*
** Read a password from the User.
*/

void iomode(int,int);


char *
getpass(char *prompt)
{
	static char buf[32];
	char *pass = buf;
	char *cp;
	void (*oldintr)();

	if(prompt)
	{
		printf("%s",prompt);
	}
	/* we don't want anyone breaking when we are in raw mode */
	oldintr = signal(SIGINT,SIG_IGN);

	iomode(1,1);
	*pass = '\0'; cp = pass;

	do 
	{
		*cp = getchar();

		if (*cp == EOF) 
		{
			break ;
		} 
		else if (*cp == '\r') 
		{
			*cp = '\0';
			printf("\n");
			break;
		} 
		else if (*cp == '\030') 
		{
			*pass = '\0';
			cp = pass;
		} 
		else if (*cp == '\010') 
		{
				if (cp > pass)
				{
					cp--;
				}
		} 
		else 
		{
			cp++;
		}
	} 
	while (1);

	iomode(1,0);

	/* enable ^C */
	(void)signal(SIGINT,oldintr);

	return buf;
}


/*
** mode=1	RAW
** mode=0	CON
*/
#include <ios1.h>
#include <dos/dos.h>
#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>
extern struct DosLibrary *DOSBase;

extern struct UFB _ufbs[];

void iomode(int fd, int mode)
{
	SetMode(_ufbs[fd].ufbfh,(LONG)mode);
}

