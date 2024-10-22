#ifndef RS485_H
#include "rs485.h"
#endif

#include "rs485.device_rev.h"

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

struct InitTable __far _LibInitTab =  {
        (long *)(&RESLEN+sizeof(struct rs485Device)),
        _LibFuncTab,
        NULL,                        /* will initialize my own data */
        _LibInit,
};
extern long far _Libmergeddata;    /* Need this to determine start of MERGED DATA */


__saveds __stdargs __asm
ULONG _LibInit( register __a0 APTR seglist,
        register __d0 struct rs485Device *libbase )
{
	long *sdata, *reloc;
	char *ddata;
	long nrelocs;

        libbase->ml_SegList = (ULONG) seglist;

        /* init. library structure (since I don't do automatic data init.) */
        libbase->ml_Lib.lib_Node.ln_Type = NT_DEVICE;
        libbase->ml_Lib.lib_Node.ln_Name =  _LibName;
        libbase->ml_Lib.lib_Flags = LIBF_SUMUSED | LIBF_CHANGED;
        libbase->ml_Lib.lib_Version = VERSION;
        libbase->ml_Lib.lib_Revision = REVISION;
        libbase->ml_Lib.lib_IdString = (APTR)VSTRING;

	ddata = (char *)(libbase + 1);
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

        /* ----- Do library specific public initialization here ----- */

        return ( (ULONG) libbase );
}

LONG __saveds __stdargs __asm
_LibOpen( register __a6 struct rs485Device *libbase,
	  register __a1 struct IOSana2Req *iob,
	  register __d0 long unit,
	  register __d1 long flags )
{
	rs485Open(libbase, iob, unit, flags);
	if(iob->ios2_Req.io_Error == S2ERR_NO_ERROR){
	        libbase->ml_Lib.lib_OpenCnt++;
	        libbase->ml_Lib.lib_Flags &= ~LIBF_DELEXP;
	}
        return ( (LONG) libbase );
}

ULONG __saveds __stdargs __asm
_LibClose( register __a6 struct rs485Device *libbase,
	   register __a1 struct IOSana2Req *iob )
{
        ULONG retval = 0;

	--libbase->ml_Lib.lib_OpenCnt;

	rs485Close(libbase, iob);

        if(libbase->ml_Lib.lib_OpenCnt==0 && (libbase->ml_Lib.lib_Flags&LIBF_DELEXP)){
                retval = _LibExpunge( libbase ); /* return segment list        */
        }

        return (retval);
}

ULONG __saveds __stdargs __asm
_LibExpunge( register __a6 struct rs485Device *libbase )
{
        ULONG seglist = 0;
        LONG  libsize;

        libbase->ml_Lib.lib_Flags |= LIBF_DELEXP;
        if(libbase->ml_Lib.lib_OpenCnt == 0)
        {
		rs485Expunge(libbase);
                seglist = libbase->ml_SegList;
                Remove((struct Node *)libbase);
                libsize = libbase->ml_Lib.lib_NegSize + libbase->ml_Lib.lib_PosSize;
                FreeMem((char *)libbase - libbase->ml_Lib.lib_NegSize,(LONG) libsize );
        }

        return((ULONG)seglist);
}

