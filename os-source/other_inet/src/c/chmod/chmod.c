/* -----------------------------------------------------------------------
 * chmod.c    AS225 release 2
 *
 * $Locker:  $
 *
 * $Id: chmod.c,v 1.3 92/11/24 15:51:49 bj Exp $
 *
 * $Revision: 1.3 $
 *
 * $Log:	chmod.c,v $
 * Revision 1.3  92/11/24  15:51:49  bj
 * binary 37.2
 * 
 * Added wildcard support ala Unix (ie, it won't recurse into
 * directories.)  
 * 
 * Moved global defines into a new chmod.h file.
 * 
 * removed all references to c.o and the like. Size goes from
 * 4000 to 2000 bytes.
 * 
 * Using MatchFirst/Next now. Full ReadArgs support to come.
 * 
 *
 * $Header: Hog:Other/inet/src/c/chmod/RCS/chmod.c,v 1.3 92/11/24 15:51:49 bj Exp $
 *
 *------------------------------------------------------------------------
 */

// #define DEBUG 1


#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <string.h>
#include "chmod.h"
#include "chmod_rev.h"

#ifdef DEBUG
int main(void) ;
#endif
int chmod(void) ;
void report( struct AnchorPath *a, struct glob *g ) ;

static char *version = VERSTAG ;

#ifdef DEBUG
int main(void)
#else
int chmod(void)
#endif
{
	struct AnchorPath *ap ;
	ULONG err ;
	struct DosLibrary *DOSBase ;
	struct glob glob ;
	struct RDArgs *ra, *r2 ;
	struct RDArgs ra2 ;
	long opts[OPT_COUNT] ;
	long opts2[OPT_COUNT2] ;
	char *p ;
	char *src, *dest ;
	char perms[128] ;
	char *files ;
	
	memset((char *)&glob, 0, sizeof(struct glob)) ;
	memset((char *)opts, 0, sizeof(opts)) ;
	memset((char *)opts2, 0, sizeof(opts2)) ;
	memset((char *)&ra2	, 0, sizeof(struct RDArgs)) ;
	 
	if(DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 37L))
	{
		glob.g_DOSBase = DOSBase ;
		
		if(ra = (struct RDargs *)ReadArgs(TEMPLATE, opts, NULL))
		{
#ifdef DODIRS		
			if( opts[OPT_A] )
			{
				glob.g_all = TRUE ;
			}
#endif			
			if( opts[OPT_V] )
			{
				glob.g_verbose = TRUE ;
			}

			dest = perms ;
			src = GetArgStr() ;

			while((*src) && (*src != ' ') && (*src != '\t') )
			{
				*dest++ = *src++ ;
			}
			*dest = '\0' ;
			
			while(*src && *src == ' ')
			{
				src++ ;
			}
			files = src ;

			ra2.RDA_Source.CS_Buffer = files ;
			ra2.RDA_Source.CS_Length = (LONG)strlen(files) ;
			ra2.RDA_Source.CS_CurChr = 0L ;

			if(r2 = (struct RDArgs *)ReadArgs(T2,opts2,&ra2))
			{
				if( opts2[OPT_F2] )
				{
					glob.g_files = (char **)opts2[OPT_F2] ;

					p = glob.g_permissions = perms ;
					if( *p >= '0' && *p <= '7')
					{
						glob.g_mask_is_numeric = TRUE ;
					}

					if (ap = (struct AnchorPath *)AllocMem(sizeof(struct AnchorPath) + 255, MEMF_CLEAR))
					{
						ap->ap_Strlen = 255 ;
						ap->ap_Flags  = APF_DOWILD ;
						ap->ap_BreakBits = SIGBREAKF_CTRL_C ;
		
						do
						{
							if(err = MatchFirst((UBYTE *)*glob.g_files, ap ))
							{
								if((err = IoErr()) != ERROR_NO_MORE_ENTRIES ) 
								{
									PrintFault(err, "chmod" ) ;
								}
							}
							else   /* ok */
							{
								do 
								{
									report(ap, &glob) ;
								}
								while( !MatchNext(ap) ) ;
							}
							glob.g_files++ ;
						}
						while( *(glob.g_files) ) ;
						MatchEnd(ap) ;
						FreeMem(ap, sizeof(struct AnchorPath) + 255) ;
					}
				}
				FreeArgs(r2) ;
			}
			else
			{
				PrintFault(IoErr(), "chmod (internal)") ;
			}
			FreeArgs(ra) ;
		}
		else
		{
			PrintFault(IoErr(), "chmod") ;
		}
		CloseLibrary((struct Library *)DOSBase) ;
	}
	return(0) ;
}
		
void
report( struct AnchorPath *a, struct glob *g )
{
	struct DosLibrary *DOSBase = g->g_DOSBase ;

#ifdef DODIRS
	LONG entrytype = a->ap_Info.fib_DirEntryType ;

	if( a->ap_Flags & APF_DIDDIR )
	{
		a->ap_Flags &= ~APF_DIDDIR ;
		return ;
	}

	setperms(g,a) ;
	
	if(entrytype > 0)
	{
		if( g->g_all )
		{
			a->ap_Flags |= APF_DODIR ;
		}
	}

	else if( entrytype < 0 )
	{
		setperms(g,a) ;
	}

#else
	setperms(g,a) ;
#endif

	if( g->g_verbose )
	{
		PutStr(a->ap_Buf) ;
		PutStr("\n") ;
	}
}

/************************************************
 * test permissions
 ************************************************
 */


int
setperms(struct glob *g, struct AnchorPath *a )
{
	char *p = g->g_permissions ;
	char *fnam, op ;
    ULONG emask, mask, whomask ;
	struct DosLibrary *DOSBase = g->g_DOSBase ;
	LONG foo ;
	
	fnam = a->ap_Buf ;
	foo = (long)&fnam ;

	if( g->g_mask_is_numeric )
	{
//		whomask = 07777 ;
		for(mask = 0 ; *p && (*p >= '0' && *p <= '7') ;p++)
		{
			mask = (mask<<3) + *p - '0' ;
		}
		if( mask > 07777 || *p )
		{
			PutStr("chmod: invalid mode\n") ;
			return(0) ;
		}
	}
	
	
			  /* we're testing the permissions on each file as we go */
	else
	{
		if( !GetUnixProtect(fnam, &mask, g))
		{
			VPrintf("chmod: cannot access '%s', try numeric protections\n", (LONG *)&foo) ; /***/
			return(0) ;
		}
		
		while(*p)
		{
			emask = whomask = 0 ;
			while(*p && *p==',')
			{
				p++ ;
			}
			while(*p && ((*p=='u' && (whomask|=05700)) ||
						 (*p=='g' && (whomask|=03070)) ||
						 (*p=='o' && (whomask|=01007)) ||
						 (*p=='a' && (whomask|=07777))))
			{
				p++ ;
			}
			if(whomask == 0)
			{
				whomask = 07777 ;        /* assume meant a */
			}

			if(*p)
			{
				op = *p++ ;
			}
			else
			{
				return(0) ;
			}
			while(*p && ((*p=='r' && (emask|=00444)) ||
						(*p=='w' && (emask|=00222)) ||
						(*p=='x' && (emask|=00111)) ||
						(*p=='s' && (emask|=06000)) ||
						(*p=='t' && (emask|=01000)) 
						))
			{
				p++ ;
			}
			if(op == '=')
			{
				mask = (mask & ~whomask) | (whomask & emask) ;
			}
			else if(op == '-')
			{
				mask &= ~(whomask & emask) ;
			}
			else if(op == '+')
			{
				mask |= whomask & emask ;
			}
			else 
			{
				PutStr("chmod: invalid operator : ") ;
				return(0) ;
			}
		}
	}
	
	if(!SetUnixProtect(fnam, mask, g))
	{
		VPrintf("chmod: can't change %s\n", (LONG *)&fnam) ;
        return(0) ;
	}
	return(1) ;
}
