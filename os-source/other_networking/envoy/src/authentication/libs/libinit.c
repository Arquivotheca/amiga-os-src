#include "authlib_internal.h"
#include <libraries/dos.h>
#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>
#include <string.h>
#include "authentication.library_rev.h"

typedef LONG (*PFL)();   /* pointer to function returning 32-bit int        */

/* library initialization table, used for AUTOINIT libraries                */
struct InitTable {
        ULONG        *it_DataSize;        /* library data space size         */
        PFL          *it_FuncTable;      /* table of entry points           */
        APTR         it_DataInit;        /* table of data initializers      */
        PFL          it_InitFunc;        /* initialization function to run  */
};

extern PFL _LibFuncTab[];      /* my function table (Generated by Blink) */

extern char __far RESLEN;
extern long __far NEWDATAL; /* Generated by BLINK */
#define DATAWORDS ((long)&NEWDATAL)

ULONG __asm _LibInit(register __a0 APTR seglist,
                     register __d0 struct AuthenticationBase *AuthenticationBase );

struct InitTable __far _LibInitTab =  {
        (long *)(&RESLEN+sizeof(struct AuthenticationBase)),
        _LibFuncTab,
        NULL,                        /* will initialize my own data */
        _LibInit,
};
extern long far _Libmergeddata;    /* Need this to determine start of MERGED DATA */


/* These two are defined by blink */
extern char __far _LibID[];
extern char __far _LibName[];

ULONG __asm _LibInit(register __a0 APTR seglist,
		     register __d0 struct AuthenticationBase *AuthenticationBase )
{
long *sdata, *reloc;
char *ddata;
long nrelocs;

        AuthenticationBase->ml_SegList = (ULONG) seglist;
/*
**  init. library structure (since I don't do automatic data init.)
*/
	AuthenticationBase->ml_Lib.lib_Node.ln_Type = NT_LIBRARY;
        AuthenticationBase->ml_Lib.lib_Node.ln_Name =  _LibName;
        AuthenticationBase->ml_Lib.lib_Flags = LIBF_SUMUSED | LIBF_CHANGED;
        AuthenticationBase->ml_Lib.lib_Version = VERSION;
        AuthenticationBase->ml_Lib.lib_Revision = REVISION;
        AuthenticationBase->ml_Lib.lib_IdString = (APTR) _LibID;
	ddata = (char *)&AuthenticationBase->ml_Data+4;   /* The +4 is a wasted long word,
						  where _Libmergeddata is. */
	sdata = (long *)&_Libmergeddata;
	memcpy(ddata, (char *)sdata, DATAWORDS*4);
	sdata = sdata + DATAWORDS;
	nrelocs = *sdata++;
	while (nrelocs > 0)
	{
		reloc = (long *)((long)ddata + *sdata++);
		*reloc += (long)ddata;
		nrelocs--;
	}
	OnOff = FALSE;
        return ( (ULONG) AuthenticationBase );
}

ULONG __saveds __asm _LibExpunge( register __a6 struct AuthenticationBase *AuthenticationBase )
{
ULONG seglist = 0;
LONG  libsize;

        AuthenticationBase->ml_Lib.lib_Flags |= LIBF_DELEXP;
        if ( AuthenticationBase->ml_Lib.lib_OpenCnt == 0 )
        {
	/*
	**  really expunge: remove AuthenticationBase and freemem
	*/
                seglist = AuthenticationBase->ml_SegList;
                Remove( (struct Node *) AuthenticationBase);
                libsize = AuthenticationBase->ml_Lib.lib_NegSize + AuthenticationBase->ml_Lib.lib_PosSize;
                FreeMem( (char *) AuthenticationBase - AuthenticationBase->ml_Lib.lib_NegSize,(LONG) libsize );
        }
/*
**  return NULL or real seglist
*/
        return ( (ULONG) seglist );
}

LONG __asm _LibOpen( register __a6 struct AuthenticationBase *AuthenticationBase )
{
/*
**  if not previously opened, do init.
*/
	if(AuthenticationBase->ml_Lib.lib_OpenCnt == 0)
	{
	BUG(("About to CustomLibOpen()...  "));
		if(!CustomLibOpen(AuthenticationBase))
		{
		BUG(("failed CustomLibOpen().\n"));
       	 		return (NULL);
       		}
       	BUG(("did CustomLibOpen().\n"));
       	}
/*
**  mark us as having another customer
*/
        AuthenticationBase->ml_Lib.lib_OpenCnt++;
/*
**  clear delayed expunges (standard procedure)
*/
        AuthenticationBase->ml_Lib.lib_Flags &= ~LIBF_DELEXP;
       	return ( (LONG) AuthenticationBase );
}

ULONG __asm _LibClose( register __a6 struct AuthenticationBase *AuthenticationBase )
{
ULONG retval = 0;

        if(--AuthenticationBase->ml_Lib.lib_OpenCnt == 0)
        {
		CustomLibClose(AuthenticationBase);
		if(AuthenticationBase->ml_Lib.lib_Flags & LIBF_DELEXP)
       		{
       		/*
        	**  no more people have me open,
        	**  and I have a delayed expunge pending
        	*/
        	        retval = _LibExpunge( AuthenticationBase ); /* return segment list */
        	}
        }
        return retval;
}
