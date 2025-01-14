/* ---------------------------------------------------------------------------------
 * CONFIGINET.C    (CONFIGINET)
 *
 * $Locker:  $
 *
 * $Id: configinet.c,v 1.1 94/03/24 19:04:13 jesup Exp $
 *
 * $Revision: 1.1 $
 *
 * $Log:	configinet.c,v $
 * Revision 1.1  94/03/24  19:04:13  jesup
 * Initial revision
 * 
 *
 * $Header: Hog:Other/inet/src/c/configinet/RCS/configinet.c,v 1.1 94/03/24 19:04:13 jesup Exp $
 *
 *-----------------------------------------------------------------------------------
 */

 /* -------------------------------------------------------------------------------
  * CONFIGINET
  * 
  * This utility is used to with the as225/Envoy inet.library v5.0 and up.
  *                                                           ^^^^
  *
  * 1. It allows the user to set the gateway flag in the library. This enables
  *    (or disables) the forwardiing of IP packets. Default is -OFF-.
  * 2. Allows the user to query the status of the gateway flag using the
  *    "-Q=QUERY" argument.  At this time (1.0) this returns a single ascii
  *    character value ('1' or '0') stating the current state of the internal
  *    flag.
  * 3. It is possible to both set and query this flag in one action:
  *      "configinet gateway=1 query=g"
  *
  * Current options :
  *
  *   -G=GATEWAY - this takes an argument of either zero (0) or non-zero. 
  *                Zero turns off the gateway mode (the default) and non-zero
  *                turns it on.
  *
  *   -Q=QUERY   - this returns a character string representing the current 
  *                value of the requested variable. Under v1.0 the only option 
  *                for this flag is 'G' (for gateway.)  This will return an
  *                ascii '1' or '0' according to the current state of the 
  *                gateway flag.  Query returns may be redirected to files
  *                or environment variables. If redirected, the trailing 
  *                newline is suppressed.
  *
  *    Examples:
  *
  *        configinet -g 1         
  *        configinet -g=1         
  *        configinet gateway=1
  *        configinet gateway 1    These all turn ON gatway mode.
  *        
  *        configinet -g 0
  *        configinet -g=0
  *        configinet gateway=0
  *        configinet gateway 0    These all turn OFF gatway mode.
  *
  *
  *        configinet -q g
  *        configinet -Q=gateway  (only the 1st char in the string is seen)
  *        configinet query=g
  *        configinet query G     These all ask for the status of the 
  *                               internal gateway flag. This returns a '1'
  *                               or a '0' to represent that state.
  *
  * -------------------------------------------------------------------------------
  */
 
#include <exec/types.h>
#include <dos/dos.h>
#include <string.h>

#include <netinet/inetconfig.h>
#include <pragmas/inet_pragmas.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "configinet_rev.h"
STATIC BYTE *vers = VERSTAG ;

#define TEMPLATE "-G=GATEWAY/K,-Q=QUERY/K"

#define OPT_G      0
#define OPT_Q      1
#define OPT_COUNT  2

#define DOSLIB     "dos.library"
#define DOSVER      37L
#define INETLIB    "inet.library"
#define INETVER    5L

#define BUFFSIZE 256L

#ifdef NULL
#undef NULL
#endif
#define NULL ((VOID *)0L)

void ConfigureInetA( long * ) ;
void mysprintf( BYTE *, ... ) ;

int configinet(VOID)
{

	REGISTER struct DosLibrary *DOSBase ;
	REGISTER struct RDargs     *rdargs ;
	REGISTER struct Library    *InetBase ;
	REGISTER int retval        = RETURN_FAIL ;
	REGISTER int dotemplate    = FALSE ;
	struct InetQuery iq ;
	ULONG opts[OPT_COUNT] ;
	ULONG    tags[4] = {INET_Gateway,TRUE,TAG_DONE,0L} ;

	if(DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER)) 
	{
		if(InetBase = OpenLibrary(INETLIB,INETVER ))
		{
			memset((char *)&iq, 0, sizeof(iq)) ;
			memset((char *)opts,0,sizeof(opts)) ;

			if(rdargs = ReadArgs(TEMPLATE, opts, NULL))
			{
				if( opts[OPT_G] )
				{
					if( *(BYTE *)opts[OPT_G] == '0')
					{
						tags[1] = FALSE ;
					}
					ConfigureInetA(&tags[0]) ;
					retval = RETURN_OK ;
				}
				if( opts[OPT_Q] )
				{
					tags[0] = INET_Query ;
					tags[1] = (long)&iq ;
					ConfigureInetA(&tags[0]) ;
					switch( *(UBYTE *)opts[OPT_Q] )
					{
						case 'G' :
						case 'g' :
							PutStr(iq.iq_Gateway ? "1" : "0" ) ;
							if(IsInteractive(Output()))
							{
								PutStr("\n") ;
							}
							retval = RETURN_OK ;
							break ;
 						default:
							dotemplate = TRUE ;
							break ;
					}
				}
				if( dotemplate || ((! opts[OPT_G]) && (!opts[OPT_Q]) && (*GetArgStr() != '?')))
				{
					PutStr(TEMPLATE) ;
					PutStr("\n") ;
				}
			}
		}
		else
		{
			PutStr("configinet: cannot open inet.library.\n") ;
		}
		CloseLibrary((struct Library *)DOSBase) ;
	}
	return( retval ) ;
}
