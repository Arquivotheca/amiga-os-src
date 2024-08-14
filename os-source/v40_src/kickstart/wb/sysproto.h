/*
 * $Id: sysproto.h,v 38.3 92/02/27 12:01:30 mks Exp $
 *
 * $Log:	sysproto.h,v $
 * Revision 38.3  92/02/27  12:01:30  mks
 * Removed the locale stuff (all handled in magic assembly)
 * 
 * Revision 38.2  91/09/12  10:01:26  mks
 * Added locale support to prototypes for system calls.
 *
 * Revision 38.1  91/06/24  11:39:02  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#include "dos.h"
#include "workbenchbase.h"

#define getWbBase()	((struct WorkbenchBase *) getreg(REG_A6))
#define WbBASE		getWbBase()

#define SysBase WbBASE->wb_SysBase
#define GfxBase ((struct GfxBase *) WbBASE->wb_GfxBase)
#define IntuitionBase ((struct IntuitionBase *) WbBASE->wb_IntuitionBase)
#define IconBase ((struct Library *) WbBASE->wb_IconBase)
#define DOSBase ((struct DosLibrary *) WbBASE->wb_DOSBase)
#define LayersBase ((struct LayersBase *) WbBASE->wb_LayersBase)
#define TimerBase ((struct Library *) WbBASE->wb_TimerBase)
#define DiskfontBase ((struct Library *) WbBASE->wb_DiskfontBase)
#define GadToolsBase WbBASE->wb_GadToolsBase

#include <exec/types.h>
#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/layers_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/timer_protos.h>
#include <clib/icon_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/layers_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/diskfont_pragmas.h>
#include <pragmas/timer_pragmas.h>
#include <pragmas/icon_pragmas.h>
