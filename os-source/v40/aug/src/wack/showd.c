/*
 * Amiga Grand Wack
 *
 * showd.c -- Display DOS structures.
 *
 * $Id: showd.c,v 39.3 93/05/21 17:34:51 peter Exp $
 *
 * Code to display DOSBase.
 *
 */

#include "exec/types.h"
#include "exec/tasks.h"

#include "libraries/dosextens.h"

#include "symbols.h"
#include "special.h"

#include "show_proto.h"
#include "sys_proto.h"
#include "special_proto.h"
#include "link_proto.h"
#include "showd_proto.h"
#include "io_proto.h"

extern APTR CurrAddr;
extern APTR SpareAddr;

#define BSTRBUFLEN	256
char bstrbuf[ BSTRBUFLEN ];

void
PrintBSTR( BSTR *bstr )
{
    char *s;

    s = (char *) (ReadLong( bstr ) << 2);
    GetBSTR( s, bstrbuf, BSTRBUFLEN );
    PPrintf("\"%s\"", bstrbuf );
}

STRPTR
ShowDBase( void )
{
    struct DosLibrary DBase;
    APTR dbase;

    if (dbase = FindBase ("dos.library"))
    {
	SpareAddr = dbase;
	ReadBlock(dbase, &DBase, sizeof DBase);

	ShowAddress ("DOSBase", dbase);
	ShowAddress ("BCPL global vector", DBase.dl_GV);
	PPrintf ("Private A2 %08lx,  A5 %08lx,  A6 %08lx\n",
		DBase.dl_A2, DBase.dl_A5, DBase.dl_A6);
	ShowAddress ("RootNode", DBase.dl_Root);
    }
    else
    {
	PPrintf("Couldn't find dos.library.\n");
    }

    return( NULL );
}

STRPTR
ShowDOSList( void )
{
    struct DosLibrary DBase;
    struct RootNode *rn;
    struct DosInfo *di;
    struct DosList *dol;
    APTR dbase;
    long s;

    if (dbase = FindBase ("dos.library"))
    {
	ReadBlock(dbase, &DBase, sizeof DBase);

	rn = (struct RootNode *) DBase.dl_Root;
	ShowAddress ("RootNode", rn);

	di = (struct DosInfo *) ((long) ReadLong(&rn->rn_Info) << 2);
	ShowAddress ("DosInfo", di);
	dol = (struct DosList *) ((long) ReadLong(&di->di_DevInfo) << 2);
        ShowAddress ("DevInfo", dol);
	while (dol) {
	    PPrintf("{%8lx} ", ((long) dol) >> 2);
	    switch ((int)ReadLong(&dol->dol_Type)) {
	      case DLT_PRIVATE:
		PPrintf("PRIVATE");
		break;
	      case DLT_DEVICE:
		PPrintf("DEVICE ");
		break;
	      case DLT_DIRECTORY:
		PPrintf("ASSIGN ");
		break;
	      case DLT_VOLUME:
		PPrintf("VOLUME ");
		break;
	      case DLT_LATE:
		PPrintf("LATE A ");
		break;
	      case DLT_NONBINDING:
		PPrintf("NBIND A");
		break;
	      default:
		PPrintf("??? %3ld", ReadLong(&dol->dol_Type));
	    }
	    if (ReadLong(&dol->dol_Task)) {
		PPrintf("  %8lx  %8lx  ", ((long) dol) >> 2,
			((long) ReadLong(&dol->dol_Task)) - sizeof(struct Task),
			ReadLong(&dol->dol_Lock));
	    }
	    else {
		PPrintf("       0  %8lx  ", ReadLong(&dol->dol_Lock));
	    }
	    PrintBSTR(&dol->dol_Name);
	    switch ((int)ReadLong(&dol->dol_Type)) {
	      case DLT_DEVICE:
		s = (long) ReadLong(&dol->dol_misc.dol_handler.dol_Handler);
		if (s) {
		    PPrintf("  H: ");
		    PrintBSTR(&dol->dol_misc.dol_handler.dol_Handler);
		}
		PPrintf("  SS: #%ld  P: #%ld  S: %lx  SL: %lx  GV: #%ld",
			ReadLong(&dol->dol_misc.dol_handler.dol_StackSize),
			ReadLong(&dol->dol_misc.dol_handler.dol_Priority),
			ReadLong(&dol->dol_misc.dol_handler.dol_Startup),
			ReadLong(&dol->dol_misc.dol_handler.dol_SegList),
			ReadLong(&dol->dol_misc.dol_handler.dol_GlobVec));
		break;
	      case DLT_VOLUME:
		PPrintf("  LL: %lx  DT: %lx",
			ReadLong(&dol->dol_misc.dol_volume.dol_LockList),
			ReadLong(&dol->dol_misc.dol_volume.dol_DiskType));
		break;
	      default:;
	    }
	    NewLine();
	    dol = (struct DosList *) (((long) ReadLong(&dol->dol_Next)) << 2);
	}
    }
    else
    {
	PPrintf("Couldn't find dos.library.\n");
    }

    return( NULL );
}
