/* -----------------------------------------------------------------------
 * capture.c          capture buffer code for rlogin
 *
 * $Locker:$
 *
 * $Id:$
 *
 * $Revision:$
 *
 * $Log:$
 *
 * $Header:$
 *
 *------------------------------------------------------------------------
 */

#define DEBUG 1

#define CAPTURE 1

#include <exec/types.h>
#include <exec/libraries.h>
#include <libraries/dos.h>
#include <utility/tagitem.h>
#include <clib/asl_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/utility_protos.h>
#include <pragmas/asl_pragmas.h>
#include <string.h>

#include "rlglobal.h"

BPTR openit(struct RLGlobal *g, STRPTR d, STRPTR f ) ;
BOOL exists(char *fname) ;

BOOL
OpenBuffer( struct RLGlobal *g )
{
	APTR requester ;
	BOOL success = FALSE ;
	struct Library *AslBase ;
	STRPTR file = NULL, drawer = NULL ;
	BPTR fh ;

	if(AslBase = (struct Library *)OpenLibrary("asl.library", 37L ))
	{
		asltags[0].ti_Data = (LONG)g->rl_Window ;
		if(requester = AllocAslRequest(ASL_FileRequest, &asltags[0] ))
		{
			PutStr("allocaslrequest success\n") ;
			AslRequest(requester,NULL) ;

			drawer = ((struct FileRequester *)requester)->fr_Drawer ;
			file   = ((struct FileRequester *)requester)->fr_File ;
			printf("drawer= %s\n", drawer ) ;
			printf("file  = %s\n", file) ;
			if((fh = openit(g, drawer, file )) != NULL)
			{
				g->rl_capture_fh = fh ;
				g->rl_capture = TRUE ;
				success = TRUE ;
			}
			FreeAslRequest(requester) ;
		}
		else
		{
			PutStr("AllocAslRequest failed\n") ;
		}
		CloseLibrary(AslBase) ;
	}
	return(success) ;
}

/*************************************************************/

BPTR openit(struct RLGlobal *g, STRPTR d, STRPTR f )
{
	BPTR fh ;
	UBYTE buffer[1024] ;
	BOOL retval = FALSE ;
	
	strcpy(buffer, d) ;
	if(AddPart(buffer, f, 1024L ))
	{
		if(exists(buffer))
		{
			if(fh = Open(buffer, MODE_OLDFILE))
			{
				if(g->rl_append)
				{
					Seek(fh, 0L, OFFSET_END) ;
					if(IoErr())
					{
						printf("seek failed\n") ;
						Close(fh) ;
						fh = NULL ;
					}
				}
			}
		}
		else
		{
			fh = Open(buffer, MODE_NEWFILE) ;
		}
	}
return(fh) ;
}

/***************************************************************/

BOOL exists(char *fname)
{
	BPTR fh ;
	
	if( fh = Open(fname, MODE_OLDFILE))
	{
		Close(fh) ;
		return(TRUE) ;
	}
	return(FALSE) ;
}
