/*
 * $Id: backfill.c,v 38.3 92/04/09 07:11:03 mks Exp $
 *
 * $Log:	backfill.c,v $
 * Revision 38.3  92/04/09  07:11:03  mks
 * Some quick cleanup...
 * 
 * Revision 38.2  92/01/07  13:58:22  mks
 * Uses the new backfill stuff.  Thus, this code is now very simple and
 * may well become "obsolete" soon!
 *
 * Revision 38.1  91/06/24  11:33:58  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#include "proto.h"
#include "string.h"
#include "exec/types.h"
#include "exec/memory.h"
#include "macros.h"
#include "intuition/intuition.h"
#include "graphics/gfxmacros.h"
#include "utility/hooks.h"
#include "workbench.h"
#include "workbenchbase.h"
#include "global.h"
#include "support.h"

struct Hook *PrepareBackFill(BOOL diskwin)
{
struct WorkbenchBase *wb = getWbBase();
struct Hook *hook;

	hook=&(wb->wb_WinHook);
	if (diskwin) hook=&(wb->wb_WBHook);

	return(hook);
}

void InstallBackFill(struct Window *w)
{
struct WorkbenchBase *wb = getWbBase();

	InstallLayerHook(w->WLayer, PrepareBackFill(w == wb->wb_BackWindow));
}
