/* $Id: showd.c,v 1.5 91/04/24 20:50:28 peter Exp $	*/
/* showd.c : wack "show dos" functions.
 */

#include "exec/types.h"
#include "exec/tasks.h"

#include "libraries/dosextens.h"

#include "symbols.h"
#include "special.h"

extern APTR CurrAddr;
extern APTR SpareAddr;

PrintBSTR(bstr)
long bstr;
{
    long s;
    short i;

    Print("\"");
    s = ((long) GetMemL(bstr)) << 2;
    i = GetMemB(s++);
    while (i--)
	Print("%c", GetMemB(s++));
    Print("\"");
}

	    
ShowDBase()
{
    struct DosLibrary DBase;
    struct RootNode *rn;
    struct DosInfo *di;
    struct DosList *dol;
    long dbase;
    long s;
    short i;

    if (dbase = (long) FindBase ("dos.library"))
    {
	SpareAddr = (APTR) dbase;
	GetBlock(dbase, &DBase, sizeof DBase);

	ShowAddress ("", dbase);
	ShowAddress ("BCPL global vector", DBase.dl_GV);
	Print ("Private A2 %08lx,  A5 %08lx,  A6 %08lx\n",
		DBase.dl_A2, DBase.dl_A5, DBase.dl_A6);
	rn = (struct RootNode *) DBase.dl_Root;
	ShowAddress ("RootNode", rn);

	di = (struct DosInfo *) ((long) GetMemL(&rn->rn_Info) << 2);
	ShowAddress ("DosInfo", di);
	dol = (struct DosList *) ((long) GetMemL(&di->di_DevInfo) << 2);
        ShowAddress ("DevInfo", dol);
	while (dol) {
	    Print("{%6lx} ", ((long) dol) >> 2);
	    switch (GetMemL(&dol->dol_Type)) {
	      case DLT_PRIVATE:
		Print("PRIVATE");
		break;
	      case DLT_DEVICE:
		Print("DEVICE ");
		break;
	      case DLT_DIRECTORY:
		Print("ASSIGN ");
		break;
	      case DLT_VOLUME:
		Print("VOLUME ");
		break;
	      case DLT_LATE:
		Print("LATE A ");
		break;
	      case DLT_NONBINDING:
		Print("NBIND A");
		break;
	      default:
		Print("??? %3d", GetMemL(&dol->dol_Type));
	    }
	    if (GetMemL(&dol->dol_Task)) {
		Print("  %6lx  %6lx  ", ((long) dol) >> 2,
			((long) GetMemL(&dol->dol_Task)) - sizeof(struct Task),
			GetMemL(&dol->dol_Lock));
	    }
	    else {
		Print("       0  %6lx  ", GetMemL(&dol->dol_Lock));
	    }
	    PrintBSTR(&dol->dol_Name);
	    switch (GetMemL(&dol->dol_Type)) {
	      case DLT_DEVICE:
		s = (long) GetMemL(&dol->dol_misc.dol_handler.dol_Handler);
		if (s) {
		    Print("  H: ");
		    PrintBSTR(&dol->dol_misc.dol_handler.dol_Handler);
		}
		Print("  SS: #%ld  P: #%ld  S: %lx  SL: %lx  GV: #%ld",
			GetMemL(&dol->dol_misc.dol_handler.dol_StackSize),
			GetMemL(&dol->dol_misc.dol_handler.dol_Priority),
			GetMemL(&dol->dol_misc.dol_handler.dol_Startup),
			GetMemL(&dol->dol_misc.dol_handler.dol_SegList),
			GetMemL(&dol->dol_misc.dol_handler.dol_GlobVec));
		break;
	      case DLT_VOLUME:
		Print("  LL: %lx  DT: %lx",
			GetMemL(&dol->dol_misc.dol_volume.dol_LockList),
			GetMemL(&dol->dol_misc.dol_volume.dol_DiskType));
		break;
	      default:;
	    }
	    Print("\n");
	    dol = (struct DosList *) (((long) GetMemL(&dol->dol_Next)) << 2);
	}
    }
    else
    {
	puts("\ncouldn't find dos.library in exec library list.");
    }
}
