/* -----------------------------------------------------------------------
 * lib_custom.c
 *
 * $Locker:  $
 *
 * $Id: lib_custom.c,v 1.3 92/07/21 15:41:33 bj Exp $
 *
 * $Revision: 1.3 $
 *
 * $Log:	lib_custom.c,v $
 * Revision 1.3  92/07/21  15:41:33  bj
 * Now requires inet.library >= 5.0.
 * NULLs out library bases upon closure.
 * This was used in socket.library 4.0 and greater. AS225 R2.
 * 
 *
 * $Header: AS225:src/lib/ss/RCS/lib_custom.c,v 1.3 92/07/21 15:41:33 bj Exp $
 *
 *------------------------------------------------------------------------
 */
/*
 * Library startup & init
 *
 * July 24, 1992    upped requested inet.library to versions >= 5.0
 */

#include "sslib.h"

BOOL __saveds __asm CustomLibOpen(register __a6 struct SocketLibrary *libbase);
void __saveds __asm CustomLibClose(register __a6 struct SocketLibrary *libbase);

struct ExecBase *SysBase = NULL;
struct DOSBase  *DOSBase = NULL;
struct InetNode *InetBase = NULL;
struct SocketLibrary *SockBase = NULL;
struct Library *UtilityBase = NULL;

extern struct config *init_config(int) ;

BOOL __saveds __asm CustomLibOpen(register __a6 struct SocketLibrary *lib)
{

	SockBase = lib;

	SysBase = (struct ExecBase *)(*((ULONG *)4));

		/* need this for 68040 */
    if(SysBase->LibNode.lib_Version >= 37) 
	{
		CacheClearU();
	}

	if(DOSBase = (struct DOSBase *)OpenLibrary("dos.library", 37L)) 
	{
		if(UtilityBase = (struct Library *)OpenLibrary("utility.library", 0L))
		{
			if(InetBase = (struct InetNode *)OpenLibrary("inet:libs/inet.library", 5L))
			{
				if(lib->ml_origbase->ml_config != NULL )
				{
					return(TRUE) ;
				}
				else
				{
					GETCONFIG(SockBase->ml_origbase->ml_config) ; /* this ultimately goes out! */
					if( SockBase->ml_origbase->ml_config == NULL) 
					{
						SockBase->ml_origbase->ml_config = init_config( 0 ) ;
						if(SockBase->ml_origbase->ml_config)
						{
							return(TRUE) ; 
						}
					}
				}             
				CloseLibrary((struct Library *)InetBase);
				InetBase = NULL;
			}
			CloseLibrary(UtilityBase);
			UtilityBase = NULL;
		}
		CloseLibrary(DOSBase);
		DOSBase = NULL;
	}
	return (FALSE);
}


void __saveds __asm CustomLibClose(register __a6 struct SocketLibrary *lib)
{
	/* close inet.library.  Inet.library will not currently expunge */

	if (InetBase)
		CloseLibrary((struct Library *)InetBase);

	if (UtilityBase)
		CloseLibrary((struct Library *)UtilityBase);

	if(DOSBase)
		CloseLibrary((struct Library *)DOSBase);
}
