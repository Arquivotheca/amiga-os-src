 
#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <libraries/dosextens.h>
#include <dos/rdargs.h>
#include <string.h>
#include <stdio.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#define TEMPLATE "PERMISSIONS/A,FILES/A/M"

#define OPT_P      0
#define OPT_FILES  1
#define OPT_COUNT  2

#define DOSLIB     "dos.library"
#define DOSVER      37L

#define BUFFSIZE 256L

void mysprintf(char *, ...) ;  

STRPTR GetArgStr(VOID) ;

int test(VOID)
{

/*	REGISTER struct Library    *SysBase = (*((struct Library **) 4)); */
	REGISTER struct DosLibrary *DOSBase ;
	REGISTER struct RDargs     *rdargs ;
	REGISTER int retval   = RETURN_FAIL ;
	BYTE     hbuf[BUFFSIZE] ;
	LONG     opts[OPT_COUNT];
	STRPTR   argstring ;
	BPTR console ;
	char last, first ;
	struct RDArgs ra ;

	if(DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER)) 
	{
		memset((char *)opts, 0, sizeof(opts));
		memset((char *)&ra, 0, sizeof(ra));

		argstring   = GetArgStr() ;
		first = *argstring ;		
		
		if( first == '?')
		{
			PutStr(TEMPLATE) ;
			PutStr(": ") ;
			Flush(Output()) ;
			if(console = Open("CONSOLE:", MODE_NEWFILE))
			{
				FGets(console, hbuf, BUFFSIZE) ;
				if( *hbuf == '?' )
				{				
					PutStr("       Who options : ugoa\n") ;
					PutStr("  Operator options : -+=\n") ;
					PutStr("Permission options : rwxst (xst are Unix only)\n") ;
					PutStr("Example : 'chmod g-w,o+x myfile'\n") ;
					goto getout ;
				}
				else
				{
					ra.RDA_Source.CS_Buffer = hbuf ;
					ra.RDA_Source.CS_Length = (LONG)strlen(hbuf) ;
				}
			}					
		}
		else
		{
			ra.RDA_Source.CS_Buffer = (UBYTE *)argstring ;
			ra.RDA_Source.CS_Length = (LONG)strlen(argstring) ;
		}
		ra.RDA_Source.CS_CurChr = 0L ;
		ra.RDA_ExtHelp = "test help" ;
		ra.RDA_Flags |= RDAF_NOPROMPT ;
		
		if(rdargs = ReadArgs(TEMPLATE, opts, &ra))
		{
			PutStr("ReadArgs succeeded\n") ;
			FreeArgs(rdargs) ;
		}                     
		else
		{
			PutStr("readargs failed!\n") ;
		}
getout:		
		CloseLibrary((struct Library *)DOSBase) ;
	}
	return( retval ) ;
}
