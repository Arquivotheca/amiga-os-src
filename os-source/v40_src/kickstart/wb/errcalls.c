/*
 * $Id: errcalls.c,v 38.1 91/06/24 11:35:09 mks Exp $
 *
 * $Log:	errcalls.c,v $
 * Revision 38.1  91/06/24  11:35:09  mks
 * Initial V38 tree checkin - Log file stripped
 * 
 */

#include "proto.h"
#include "string.h"
#include "exec/types.h"
#include "exec/nodes.h"
#include "exec/lists.h"
#include "exec/memory.h"
#include "exec/alerts.h"
#include "macros.h"
#include "graphics/rastport.h"
#include "libraries/dos.h"
#include "libraries/dosextens.h"
#include "workbench.h"
#include "workbenchbase.h"
#include "global.h"
#include "support.h"
#include "quotes.h"

void NoMem(void)
{
    DP(("NoMem: out of memory\n"));
    ErrorTitle(Quote(Q_OUT_OF_MEM));
}

void *ObjAllocChip(struct WBObject *obj,ULONG len)
{
	return( WBFreeAlloc(&obj->wo_FreeList,len,MEMF_PUBLIC|MEMF_CHIP|MEMF_CLEAR));
}

void *ObjAllocNorm(struct WBObject *obj,ULONG len)
{
	return(WBFreeAlloc(&obj->wo_FreeList,len,MEMF_PUBLIC|MEMF_CLEAR));
}

char *
objscopy( obj, string )
struct WBObject *obj;
char *string;
{
    char *new;

    new = WBFreeAlloc( &obj->wo_FreeList, strlen(string)+1, MEMF_PUBLIC );
    if( new ) {
	strcpy(new,string);
    }
    return( new );
}

/* Does NOT refresh drawer object is in, needed for 'Show Only Icons' */
RemoveObjectCommon(obj)
struct WBObject *obj;
{
#ifdef	DEBUGGING
    struct WorkbenchBase *wb = getWbBase();
#endif
    struct WBObject *parent;
    struct NewDD *dd;

#ifdef	DEBUGGING
    if (obj == wb->wb_RootObject) {
	DP(("RemoveObjectCommon: ERROR!  Tried to remove RootObject!\n"));
	Debug(0L);
	return(1);
    }
#endif

    DP(("RemoveObjectCommon: removing obj %lx, '%s'\n", obj, obj->wo_Name));

    obj->wo_UseCount = 0;
    if (dd = obj->wo_DrawerData) {
	if (dd->dd_DrawerWin) {
	    DP(("ROC: calling CloseDrawer on $%lx (%s)\n",
		obj, obj->wo_Name));
	    CloseDrawer(obj);
	}
	UNLOCK(dd->dd_Lock);
	dd->dd_Lock = 0;
    }

    if (obj->wo_Selected) {
	DP(("ROC: removing $%lx (%s) from the select list\n",
	    obj, obj->wo_Name))
	REMOVE(&obj->wo_SelectNode);
#ifdef DEBUGGING
	DP(("SelectList:\n"));
	SelectSearch(PrintObj);
	DP(("\tEND\n"));
#endif DEBUGGING
    }

    if (parent = obj->wo_Parent) {
	--parent->wo_UseCount;
	DP(("ROC: removing $%lx (%s) siblings\n",obj, obj->wo_Name))
	REMOVE(&obj->wo_Siblings);
    }

    UNLOCK(obj->wo_Lock);
    obj->wo_Lock = 0;

    WBFreeWBObject(obj);
    return(0);
}

void RemoveObject(obj)
struct WBObject *obj;
{
    struct WBObject *parent = obj->wo_Parent; /* cache parent ptr */
    int selected = obj->wo_Selected; /* cache select state */

    /* remove object */
    if (RemoveObjectCommon(obj) == NULL) {
	if (selected) { /* if object was selected, need to rethink menus */
	    RethinkMenus();
	}
	if (parent) { /* if object had a parent, need to refresh drawer */
	    RefreshDrawer(parent);
	}
    }
}

void ErrorDos(index, objname)
long index;
char *objname;
{
    char errbuf[80], dosbuf[80];
    int err = IOERR();

    sprintf(errbuf, Quote(Q_ERROR_WHILE_BASE + (UWORD)index), objname);
    Fault(err, NULL, dosbuf, sizeof(dosbuf));
    ErrorTitle(Quote(Q_ERROR_WHILE), errbuf, err, dosbuf);
}

void OutputMessage(string, a0, a1, a2, a3, Flash)
char *string;
int a0, a1, a2, a3;
BOOL Flash;
{
    struct WorkbenchBase *wb = getWbBase();
    char *buf;

    DP(("OutputMessage: enter, string=$%lx (%s)\n", string, string));
#ifdef MEMDEBUG
    PrintAvailMem("OutputMessage", MEMF_CHIP);
#endif MEMDEBUG
    if (string != wb->wb_ScreenTitle) {

	/* don't step on an existing error */
	if (wb->wb_ErrorDisplayed) {
	    return;
	}

	wb->wb_ErrorDisplayed = 1;

	buf = wb->wb_CurrentError;

	if (buf != string) {
	    if (Flash) {
		DisplayBeep(wb->wb_Screen);
	    }
	    sprintf(buf, string, a0, a1, a2, a3);
	}


    }
    else {
	/* make sure there is an error displayed */
	if (!wb->wb_ErrorDisplayed) return;

	/* special case: we are refreshing the normal screen title */
	wb->wb_ErrorDisplayed = 0;
	buf = string;
    }

    MasterSearch(WindowOperate, SetWindowTitles, -1, buf);
    DP(("OutputMessage: exit\n"));
#ifdef MEMDEBUG
    PrintAvailMem("OutputMessage", MEMF_CHIP);
#endif MEMDEBUG
}

void ErrorTitle(string, a0, a1, a2, a3)
char *string;
int a0, a1, a2, a3;
{
    OutputMessage(string, a0, a1, a2, a3, TRUE);
}

void MessageTitle(string, a0, a1, a2, a3)
char *string;
int a0, a1, a2, a3;
{
    OutputMessage(string, a0, a1, a2, a3, FALSE);
}

struct NewDD *
AllocDrawer( obj )
struct WBObject *obj;
{
    struct NewDD *dd;

    if (dd = ObjAllocNorm(obj, sizeof(struct NewDD)))
    {
	NewList( &dd->dd_Children );
	obj->wo_DrawerData = dd;
    }
    return(dd);
}
