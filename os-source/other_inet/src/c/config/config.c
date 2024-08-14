/* ---------------------------------------------------------------------------------
 * CONFIG.C    (CONFIG)
 *
 * $Locker:  $
 *
 * $Id: config.c,v 37.9 92/07/27 11:50:50 bj Exp $
 *
 * $Revision: 37.9 $
 *
 * $Log:	config.c,v $
 * Revision 37.9  92/07/27  11:50:50  bj
 * 1. error message cosmetics.
 * 2. ReadArgs now "proper" in it's error handling.
 * 3. Return values (for scripts) now correct.
 * AS225 R2.  
 * 
 * Revision 37.8  92/04/21  14:43:23  bj
 * New rev numbering system.  New user argument handling.
 * New error value returns and error messages.
 * New arg options ('-u=user','-h=hostname','-r=reconfig')
 * as well as compatibility with the old arguments (-m, -v)
 * Now gives an error message if socket.library can't be opened
 * for some reason.
 * 
 * Revision 1.7  91/10/02  13:04:44  bj
 * Fixed extra linefeed problem.
 * 
 * Revision 1.5  91/09/10  16:08:12  bj
 * Additional adjustments to make the new config backwards compatible
 * with the old way (pre shared socket library) of doing things.
 * 
 * Revision 1.3  91/09/09  16:54:50  bj
 * Added a forgotten FreeArgs() call.
 * 
 * Revision 1.2  91/09/09  12:20:05  bj
 * Completely rewritten to use no startup code and the shared
 * socket library. Uses ReadArgs as well. Code dropped from
 * 17K -> 3K -> 628 bytes.
 * 
 *
 * $Header: Hog:Other/inet/src/c/config/RCS/config.c,v 37.9 92/07/27 11:50:50 bj Exp $
 *
 *-----------------------------------------------------------------------------------
 */

 
#include <exec/types.h>
#include <ss/socket.h>
#include <sys/param.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <libraries/dosextens.h>
#include <config.h>
#include <string.h>
#include <pwd.h>
#include <stdio.h>
#include <fcntl.h>


#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>

#include "config_rev.h"
STATIC BYTE *vers = VERSTAG ;

#define TEMPLATE "-R=RECONFIG/S,-U=USER/S,-H=HOSTNAME/S,-M=RECONFIG/S,-V=VALUE/K"

#define OPT_R      0
#define OPT_U      1
#define OPT_H      2
#define OPT_M      3
#define OPT_V      4
#define OPT_COUNT  5

#define DOSLIB     "dos.library"
#define DOSVER      37L
#define SOCKLIB    "inet:libs/socket.library"
#define SOCKVER     3L
#define UTILLIB    "utility.library"
#define UTILVER     0L

#define BUFFSIZE 256L

#ifdef NULL
#undef NULL
#endif
#define NULL ((VOID *)0L)

/* whichval defs */
#define NOVAL   0
#define USERVAL 1
#define HOSTVAL 2

/* 'failure' values */
#define  NOFAIL 0
#define  BADARG 1
#define  NOARG  2
#define  UID    3
#define  HOST   4

void mysprintf(char *, ...) ;  


int config(VOID)
{
	REGISTER struct Library    *SockBase ;
/*	REGISTER struct Library    *SysBase = (*((struct Library **) 4)); */
	REGISTER struct DosLibrary *DOSBase ;
	REGISTER struct Library    *UtilityBase ;
	REGISTER struct RDargs     *rdargs ;
	REGISTER int retval   = RETURN_FAIL ;
	REGISTER int whichval = NOVAL ;
	BYTE     hbuf[BUFFSIZE] ;
	BYTE     *value ;
	int      errno ;
	struct   passwd *pw ;
	struct   passwd *getpwuid();
	LONG     opts[OPT_COUNT];
	BYTE     *user = "user" ;
	BYTE     *host = "hostname" ;
	BOOL     interactive ;
	int      failure = NOFAIL ;
	STRPTR   argstring ;

	char *errors[] = 
	{   
		NULL,                                 /* 0 */
		"config - unknown argument : ",       /* 1    (no line feed!) */
		"config requires an argument.\n",     /* 2 */
		"config - error in UID settings\n",   /* 3 */
		"config - cannot get hostname\n"      /* 4 */
	} ;

	if(DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER)) 
	{
		if(UtilityBase = OpenLibrary(UTILLIB,UTILVER ))
		{                                   
			if(SockBase = OpenLibrary(SOCKLIB,SOCKVER ))
			{
				setup_sockets(1, &errno ) ;
				memset((char *)opts, 0, sizeof(opts));
				retval = RETURN_OK ;

				interactive = IsInteractive(Output()) ;
				argstring   = GetArgStr() ;

				if(rdargs = ReadArgs(TEMPLATE, opts, NULL))
				{
					if( opts[OPT_U] || opts[OPT_H] || opts[OPT_V] )
					{
						if( opts[OPT_U] )
						{
							whichval = USERVAL ;
						}
						else if( opts[OPT_H] )
						{
							whichval = HOSTVAL ;
						}
						else if(value = (BYTE *)opts[OPT_V]) 
						{
							if(Stricmp(value, host) == 0) 
							{
								whichval = HOSTVAL ;
							}
							else if(Stricmp(value, user) == 0)
							{
								whichval = USERVAL ;
							}
							else 
							{
								whichval = NOVAL ;
							}
						}

						switch( whichval )
						{
							case NOVAL :
								retval = RETURN_ERROR ;
								failure = BADARG ;
								break ;

							case USERVAL :
								if (pw = getpwuid(getuid()))
								{
									PutStr(pw->pw_name) ;
									if( interactive) PutStr("\n") ;
								}
								else
								{
									failure = UID ;
									retval = RETURN_ERROR ;
								}
								break ;
								
							case HOSTVAL :
								if (gethostname(hbuf, sizeof(hbuf)) >= 0)
								{
									PutStr(hbuf) ;
									if( interactive) PutStr("\n") ;
								}
								else
								{
									failure = HOST ;
									retval = RETURN_ERROR ;
								}
								break ;

							default :
								retval = RETURN_FAIL ;
								break ;
						}
					}

					else if((opts[OPT_M] || opts[OPT_R]) && reconfig() )
					{
						PutStr("configuration updated.\n") ;
					}
					else
					{
						failure = NOARG ;  /* no argument given */
					}
					
					FreeArgs(rdargs) ;
				}
				else
				{
					failure = BADARG ;
				}

				if( failure )
				{
					PutStr(errors[failure]) ;
					if( failure == BADARG )
					{
						PutStr(argstring) ;
					}
					retval = RETURN_FAIL ;
				}
				cleanup_sockets() ;
				CloseLibrary( SockBase ) ;
			}
			else
			{
				PutStr("config - no socket.library\n") ;
			}
			CloseLibrary(UtilityBase) ;
		}
		CloseLibrary((struct Library *)DOSBase) ;
	}
	return( retval ) ;
}
