/* -----------------------------------------------------------------------
 * passwd.c
 *
 * $Locker:  $
 *
 * $Id: passwd.c,v 37.4 92/10/06 13:58:44 bj Exp $
 *
 * $Revision: 37.4 $
 *
 * $Header: AS225:src/c/passwd/RCS/passwd.c,v 37.4 92/10/06 13:58:44 bj Exp $
 *
 * $Log:	passwd.c,v $
 * Revision 37.4  92/10/06  13:58:44  bj
 * Major rewrite.  
 * 1. removed mystrlen() routine
 * 2. what you type is now not echoed to the display (sheesh)
 * 3. #2 required addiing a mini editor to getpass()
 * 4. Better error reporting - more Unix like messages.
 * 5. Won't allow control-C break which could leave files
 *    Locked.
 * 
 *
 *------------------------------------------------------------------------
 */


/*
 * Make a password for the given user.
 */
 
#include <exec/types.h>
#include <stdio.h>
#include <pwd.h>
#include <ss/socket.h>
#include <libraries/dosextens.h>
#include <config.h>
#include <string.h>
#include <stdio.h>

#include "passwd_rev.h"
static BYTE *vers = VERSTAG ;

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include <proto/exec.h>

#define TEMPLATE "USER/A"

#define OPT_USER   0
#define OPT_COUNT  1

#define DOSLIB     "dos.library"
#define DOSVER     36L
#define SOCKLIB    "inet:libs/socket.library"
#define SOCKVER     3L
#define UTILLIB    "utility.library"
#define UTILVER     0L

#define CTRL_C_EXIT 1
#define BUFFSIZE 256L
#define OSIZE  12
#define PWBUFF 128L

#ifdef NULL
#undef NULL
#endif

#define NULL ((BYTE *)0L)

struct glob {
	struct DosLibrary *g_DOSBase ;
	struct Library *g_UtilityBase ;
	struct Library *g_SockBase ;
	BYTE g_obuffer[OSIZE] ;
	BYTE g_pwbuff[PWBUFF] ;
	} ;


int    passwd(VOID) ;
VOID   mysprintf( BYTE *, ... ) ;
UBYTE  *crypt( REGISTER UBYTE *s, REGISTER UBYTE *uname, REGISTER struct glob *g ) ;
ULONG  mod( ULONG dividend, ULONG divisor, struct Library *UtilityBase) ;
UBYTE  *getpass( REGISTER UBYTE *prompt, REGISTER struct glob *g ) ;
struct passwd *getpwuid() ;

/* ==========================================================================
 * main entry +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * ==========================================================================
 */

int passwd(VOID)
{
	REGISTER struct DosLibrary *DOSBase ;
	REGISTER struct Library    *UtilityBase ;
	REGISTER struct Library    *SockBase ;
	REGISTER struct passwd     *pwd;
	REGISTER struct RDargs     *rdargs;
	REGISTER BPTR fh ;
	REGISTER LONG len, ok;
	LONG opts[OPT_COUNT];
	int error = 20 ;
	int pwerr = 1 ;
	int incorrect = 0 ;
	struct glob glob ;
	BYTE newname[64], newpbuf[32], *oldp, *checkp, *newp, oldname[64] ;
	BYTE hbuf[BUFFSIZE] ;
	char *pwstr = (char *)"passwd - '%s' " ;

	memset((BYTE *)&glob,0,sizeof(struct glob)) ;
	opts[0] = ok = 0L ;

	if(UtilityBase = glob.g_UtilityBase = OpenLibrary(UTILLIB,UTILVER))
	{
		if (SockBase = glob.g_SockBase = OpenLibrary(SOCKLIB,SOCKVER))
		{
			if (DOSBase = glob.g_DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER))
			{
				if((rdargs = ReadArgs(TEMPLATE, opts, NULL)) != NULL)
				{
					mysprintf(oldname,"%s.old",PWFILE) ;
					DeleteFile(oldname) ;
					mysprintf(newname,"%s.new",PWFILE) ;
					DeleteFile(newname) ;
					if(fh = Open( newname, MODE_NEWFILE))
					{
						setpwent(1L) ;
						while(pwd = getpwent())
						{
							pwerr = 0 ;
							if(Stricmp((UBYTE *)pwd->pw_name, (UBYTE *)opts[OPT_USER]) == 0)
							{
								if((len = (LONG)(pwd->pw_passwd ? strlen(pwd->pw_passwd) : 0L )) != 0L)
								{
									oldp = getpass("Old password: ", &glob) ;
									checkp = crypt(oldp, pwd->pw_name, &glob);
									if(bcmp(checkp, pwd->pw_passwd, len) != 0)
									{
										PutStr("password incorrect.\n");
										incorrect = 1 ;
										DeleteFile(newname);
										goto getout1 ;
									}
								}
								if(newp = getpass("New password: ", &glob))
								{
									if( *newp == '\0' )
									{
										DeleteFile(newname);
										goto getout1 ;
									}
								}
								CopyMem(newp, newpbuf, 32L) ;
								if(newp = getpass("Retype new password: ", &glob))
								{
									if( *newp == '\0')
									{
										DeleteFile(newname);
										goto getout1 ;
									}
								}
								if(Stricmp(newp, newpbuf) != 0)
								{
									PutStr("password mismatch.\n");
									DeleteFile(newname);
									goto getout1 ;
								}
								pwd->pw_passwd = crypt(newpbuf, pwd->pw_name, &glob);
								ok++;
							}
							mysprintf(hbuf, "%s|%s|%ld|%ld|%s|%s|%s\n\x00", 
									pwd->pw_name, 	pwd->pw_passwd,
									(LONG)pwd->pw_uid, 	(LONG)pwd->pw_gid, 
									pwd->pw_gecos, 	pwd->pw_dir, 
									pwd->pw_shell);

							FPuts(fh, hbuf) ;
						}
						if( pwerr )
						{
							PrintFault(IoErr(), "passwd - 'inet:db/passwd' ") ;
						}
getout1:
						endpwent() ;
						Close(fh) ;
					}
					else
					{
						mysprintf(hbuf, pwstr, newname) ;
						PrintFault(IoErr(), hbuf) ;
					}

					FreeArgs(rdargs) ;

					if(ok)
					{
						if((Rename(PWFILE, oldname)) != 0)  /* copy orig passwd to inet:db/passwd.old */
						{
							if((Rename(newname, PWFILE)) != 0)  /* copy new passwd entry to iinet:db/passwd */
							{
								error = 0 ;
							}
							else
							{
								mysprintf(hbuf, pwstr, PWFILE) ;
								PrintFault(IoErr(), hbuf) ;
							}
						}
						else
						{
							mysprintf(hbuf, pwstr, oldname) ;
							PrintFault(IoErr(), hbuf) ;
						}
					}
                    
				}
				else 
				{
					PrintFault(IoErr(), "passwd") ;
				}
				if( error && incorrect == 0 )
				{
					PutStr("Password unchanged.\n") ;
				}
				DeleteFile(newname) ;
				CloseLibrary((struct Library *)DOSBase) ;
			}
			CloseLibrary(SockBase) ;
		}
		CloseLibrary(UtilityBase) ;
	}                             
	return(error) ;
}

 

/* ==================================================================
 * crypt()  encrypts the new password.
 * ==================================================================
 */

 
UBYTE *crypt( REGISTER UBYTE *s, REGISTER UBYTE *uname, REGISTER struct glob *g )
{
	REGISTER struct Library *UtilityBase = g->g_UtilityBase ;
	REGISTER int i ;
	REGISTER int k ;
	unsigned int buf[OSIZE];

	for(i = 0; i < OSIZE; i++)
	{
			buf[i] = 'A' + (*s? *s++:i) + (*uname? *uname++:i);
	}

	for(i = 0; i < OSIZE; i++)
	{
		for(k = 0; k < OSIZE; k++)
		{                           
			buf[i] += buf[OSIZE-k-1];
			buf[i]  = (unsigned int)mod( (LONG)buf[i], 53L, UtilityBase) ; 
		}
		g->g_obuffer[i] = buf[i] + 'A' ;
	}
	g->g_obuffer[OSIZE-1] = 0;
	return (g->g_obuffer) ;
}


/* ====================================================================
 * getpass()
 *
 * gets password entry from the user.  Maximum length user can enter
 * is PWBUFF.
 * ====================================================================
 */

#define BACKSPACE "\x08 \x08"

UBYTE *getpass( REGISTER UBYTE *prompt, REGISTER struct glob *g )
{
	REGISTER struct DosLibrary *DOSBase = g->g_DOSBase ;
	REGISTER BPTR fhp ;
	REGISTER int charcount = 0 ;
	REGISTER BYTE *p ;
	REGISTER LONG c ;

	if(fhp = Open("CONSOLE:", MODE_NEWFILE))
	{
		*(p = g->g_pwbuff) = '\0' ;
		SetMode(fhp, 1L) ;
		FPuts(fhp,prompt) ;
		Flush(fhp) ;

		while((c = (LONG)FGetC(fhp)) != -1L)
		{
			if( c >= ' ' && c <= '~' && charcount < PWBUFF )
			{
				*p++ = (BYTE)c ;
				charcount++ ;
				FPutC(fhp, (LONG)'+' ) ;
				Flush(fhp) ;
			}
			else
			{
				switch(c)
				{
					case 13:
						FPutC(fhp,10L) ;
						Flush(fhp) ;
						goto ret ;
						break ;
					case 8:
						if(charcount > 0)
						{
							FPuts(fhp, BACKSPACE) ;
							Flush(fhp) ;
							charcount-- ;
							*(--p) = '\0' ;
						}
					default :
						break ;
				}
			}
		}
ret:		
		*p = '\0' ;
		SetMode(fhp, 0L) ;
		Close(fhp) ;
		return(g->g_pwbuff) ;
	}
	return(NULL) ;
}

