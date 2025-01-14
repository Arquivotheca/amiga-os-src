/*
 * $Id: placeicon.c,v 38.2 92/02/13 14:22:50 mks Exp $
 *
 * $Log:	placeicon.c,v $
 * Revision 38.2  92/02/13  14:22:50  mks
 * Code cleanup
 * 
 * Revision 38.1  91/06/24  11:37:40  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#include "proto.h"
#include "string.h"
#include "exec/types.h"
#include "exec/memory.h"
#include "macros.h"
#include "intuition/intuition.h"
#include "workbench.h"
#include "workbenchbase.h"
#include "global.h"
#include "support.h"

#define MINIMUMY	4
#define COLUMNWIDTH	72	/* width (in pixels) of icon columns */
#define XSPACING	6	/* # of pixels between icon columns */
#define YSPACING	3	/* number of scan lines between icon rows */

int IconTextLength(char *text)
{
struct WorkbenchBase *wb = getWbBase();

	return((int)TextLength(&wb->wb_IconRast,text,strlen(text))+XSPACING);
}

void PlaceObj(struct WBObject *obj,struct WBObject *parent)
{
struct WorkbenchBase *wb = getWbBase();
struct NewDD *dd = parent->wo_DrawerData;
struct Window *win;
int i;
int maxy;

    win=dd->dd_DrawerWin;

    if (obj->wo_CurrentX == NO_ICON_POSITION)
    {
	obj->wo_CurrentX=0;
	obj->wo_CurrentY=0;
    }
    else if ((parent == wb->wb_RootObject)  && (win)) if (!PlaceCollision(obj)) win=NULL;

    if (win)
    {
        /* compute the maximum allowable Y object position */
        maxy = win->Height - win->BorderTop - win->BorderBottom + dd->dd_CurrentY;
        i = dd->dd_CurrentX+(XSPACING/2);


            /*
             * Check to see if the cached placement looks fine...
             */
        if (    (dd->dd_LastObjY < obj->wo_IOGadget.Height)
             && (dd->dd_LastMin == (dd->dd_MinX + dd->dd_MinY))
             && (dd->dd_LastMax == (dd->dd_MaxX + dd->dd_MaxY))
             && (dd->dd_LastUse == parent->wo_UseCount) )
        {
            i=dd->dd_LastColumn;
            if (!(parent->wo_UseCount & 31)) MinMaxDrawer(parent);
        }

        /* go in strips searching for a free place */
        while(!PlaceIconStrip(obj, parent, i, COLUMNWIDTH, dd->dd_CurrentY+MINIMUMY, maxy)) i += COLUMNWIDTH + XSPACING;

            /*
             * Store the data to determin cached placement status...
             */
        dd->dd_LastColumn=i;
        dd->dd_LastObjY=obj->wo_IOGadget.Height-4;
        dd->dd_LastMin=dd->dd_MinX + dd->dd_MinY;
        dd->dd_LastMax=dd->dd_MaxX + dd->dd_MaxY;
        dd->dd_LastUse=parent->wo_UseCount+1;
    }
}

InsertByY(struct WBObject *obj,struct WBObject *newobj)
{
struct WorkbenchBase *wb = getWbBase();

    if( newobj->wo_CurrentY >= obj->wo_CurrentY )
    {
	Insert( &wb->wb_UtilityList, &newobj->wo_UtilityNode,&obj->wo_UtilityNode );
	return( 1 );
    }
    return( 0 );
}

FindColumn(struct WBObject *obj,int min,int max)
{
struct WorkbenchBase *wb = getWbBase();
int objmin, objmax, i;
int GadgetWidth = obj->wo_Gadget.Width + EMBOSEWIDTH + XSPACING;

    objmin = obj->wo_CurrentX;

    i = IconTextLength(obj->wo_Name);

    if (i > GadgetWidth)
    {
	objmin -= (i - GadgetWidth + 1) / 2;
	GadgetWidth=i;
    }
    objmax = objmin + GadgetWidth;

    if (objmax >= min && objmin < max )
    {
	if (!UtilityRevSearch( InsertByY, obj ))
	{
	    ADDHEAD( &wb->wb_UtilityList, &obj->wo_UtilityNode );
	}
    }
    return( 0 );
}

PlaceVertically( obj, iconheight, currentyp, textheight )
struct WBObject *obj;
int iconheight;
int *currentyp;
int textheight;
{
    struct WorkbenchBase *wb = getWbBase();
    int currenty = *currentyp;

    /* see if the object will fit here */
    if( currenty + iconheight <= obj->wo_CurrentY )
    {
	/* it fits! */
	return( 1 );
    }

    /* update currenty */
    *currentyp = max( currenty,	obj->wo_CurrentY + obj->wo_Gadget.Height + EMBOSEHEIGHT + textheight );

    return( 0 );
}

PlaceIconStrip( icon, parent, base, width, currenty, maxy )
struct WBObject *icon;
struct WBObject *parent;
int base, width;
int currenty, maxy;
{
struct WorkbenchBase *wb = getWbBase();
LONG fontheight = wb->wb_IconFontHeight +YSPACING;
struct NewDD *dd = parent->wo_DrawerData;
int iconheight, cmpheight;
int GadgetWidth = icon->wo_Gadget.Width + EMBOSEWIDTH;


    /* zero out the utility list */
    NewList( &wb->wb_UtilityList );

    /* Make sure we have enough of a width... */
    FindColumnWidth(icon,&width);

    /* find all icons that fall in the column, and sort by Y */
    SiblingPredSearch( dd->dd_Children.lh_TailPred, FindColumn, base, base + width );

    iconheight = icon->wo_Gadget.Height + EMBOSEHEIGHT + fontheight;

    cmpheight = currenty + iconheight;
    if (maxy < cmpheight) maxy = cmpheight;

    /* now try and find an unused box */
    UtilitySearch( PlaceVertically, iconheight, &currenty, fontheight );

    if( currenty + iconheight <= maxy )
    {
	/* center icon/text inside column */
	icon->wo_CurrentY = currenty;
	icon->wo_CurrentX = base + (width - GadgetWidth + 1) / 2;

	return( 1 );
    }
    return( 0 );
}

CollideVertically(obj, min, max, ignoreobj)
struct WBObject *obj, *ignoreobj;
LONG min, max;
{
struct WorkbenchBase *wb = getWbBase();
int cury;

	cury = obj->wo_CurrentY;
	cury = !((obj == ignoreobj) || (cury >= max) || (cury + obj->wo_Gadget.Height + EMBOSEHEIGHT + wb->wb_IconFontHeight <= min));
	return(cury);
}

PlaceCollision( obj )
struct WBObject *obj;
{
struct WorkbenchBase *wb = getWbBase();
int cur, i, j;
int GadgetWidth = obj->wo_Gadget.Width + EMBOSEWIDTH;

    /* zero out the utility list */
    NewList( &wb->wb_UtilityList );

    /* find all icons that fall in the column, and sort by Y */
    cur = obj->wo_CurrentX;
    j = IconTextLength(obj->wo_Name)-XSPACING;
    i = max(j, GadgetWidth) + cur - 1;
    DP(("PC:  %ld = max(%ld, %ld)+%ld\n", i, j, GadgetWidth, cur));

    SiblingPredSearch(wb->wb_RootObject->wo_DrawerData->dd_Children.lh_TailPred, FindColumn,cur,i);

    /* now see if we have a collistion in the Y direction */
    cur = obj->wo_CurrentY;
    i = (int)( UtilitySearch( CollideVertically, cur, cur + obj->wo_Gadget.Height + EMBOSEHEIGHT + wb->wb_IconFontHeight, obj ) );
    return(i);
}

CleanupObj( obj, drawer )
struct WBObject *obj;
struct WBObject *drawer;
{
    PlaceObj( obj, drawer );
    AddIcon( obj, drawer );
    return( 0 );
}

FindColumnWidth(struct WBObject *obj,int *width)
{
struct WorkbenchBase *wb = getWbBase();
int GadgetWidth = obj->wo_Gadget.Width + EMBOSEWIDTH + XSPACING;

    if (GadgetWidth > *width) *width = GadgetWidth;
    GadgetWidth=IconTextLength(obj->wo_Name);
    if (GadgetWidth > *width) *width = GadgetWidth;

    return(0);
}

SetXPosition(struct WBObject *obj,struct WBObject *drawer,int base)
{
struct WorkbenchBase *wb = getWbBase();
int GadgetWidth = obj->wo_Gadget.Width + EMBOSEWIDTH;

    obj->wo_CurrentX = base - ((GadgetWidth+1)/2);
    return(AddIcon( obj, drawer ));
}


BuildColumn(obj, lh, maxY, currentyp)
struct WBObject *obj;
struct List *lh;
int maxY, *currentyp;
{
struct WorkbenchBase *wb = getWbBase();

	/* if icon is not too tall for this column OR first icon in column */
	if (*currentyp + obj->wo_Gadget.Height  + EMBOSEHEIGHT + wb->wb_IconFontHeight < maxY || *currentyp == MINIMUMY)
	{
		obj->wo_CurrentY = *currentyp;
		*currentyp += obj->wo_Gadget.Height + EMBOSEHEIGHT + wb->wb_IconFontHeight + YSPACING;
		REMOVE(&obj->wo_UtilityNode);
		ADDTAIL(lh, &obj->wo_UtilityNode);
		return(0);
	}
	return(1);
}

Cleanup( drawer ) /* CleanupByColumns */
struct WBObject *drawer;
{
struct WorkbenchBase *wb = getWbBase();
struct List list, columnlist;
int currentx, currenty, base, width, maxY, keepgoing;
struct NewDD *dd = drawer->wo_DrawerData;
struct WBObject *obj;
struct Node *ln;
struct Window *win;

    if (dd) if (dd->dd_DrawerWin) if (dd->dd_ViewModes <= DDVM_BYICON)
    {
        NewList( &list );

        /* order the icons by strips */
        for( currentx = dd->dd_MinX ; currentx <= dd->dd_MaxX; currentx += (COLUMNWIDTH/2) )
        {
            NewList( &wb->wb_UtilityList );
            SiblingPredSearch( dd->dd_Children.lh_TailPred, FindColumn, currentx, currentx + (COLUMNWIDTH/2));

            /* transfer this column to the master list */
            while( ln = (struct Node *) RemHead( &wb->wb_UtilityList ) )
            {
                obj = NodeToObj( ln );
                drawer->wo_UseCount--;
                obj->wo_UseCount--;
                REMOVE( &obj->wo_Siblings );
                ADDTAIL( &list, ln );
            }
        }

        /* reset the window origin */
        dd->dd_CurrentX = dd->dd_CurrentY = 0;
        RefreshDrawer( drawer );

        win = dd->dd_DrawerWin;
        maxY = win->Height - win->BorderTop - win->BorderBottom + dd->dd_CurrentY;
        base = 0;
        do
        {
            currenty = MINIMUMY;
            NewList(&columnlist); /* clear column list */
            /* build column list (assigns y position) */
            keepgoing = (int)SiblingSuccSearch(list.lh_Head, BuildColumn, &columnlist, maxY, &currenty);
            width = 0; /* clear column width */
            /* find column width */
            SiblingSuccSearch(columnlist.lh_Head, FindColumnWidth, &width);
            /* assign x position */
            SiblingSuccSearch(columnlist.lh_Head, SetXPosition, drawer, base + ((width+1)>>1));

            base += (width+1);
        } while (keepgoing);

        MinMaxDrawer( drawer );
    }

MP(("Cleanup: exit\n"));
    return( 0 );
}

struct WBObject *
insertByName( refobj, newobj )
struct WBObject *refobj;
struct WBObject *newobj;
{
    if (stricmp(refobj->wo_Name, newobj->wo_Name) > 0) {
	return(refobj);
    }
    else return(NULL);
}

struct WBObject *
insertByDate( refobj, newobj )
struct WBObject *refobj;
struct WBObject *newobj;
{
    if (CompareDates(&refobj->wo_DateStamp, &newobj->wo_DateStamp) > 0) {
	return( refobj );
    }
    else return(NULL);

}

struct WBObject *
insertBySize( refobj, newobj )
struct WBObject *refobj;
struct WBObject *newobj;
{
    if (refobj->wo_FileSize > newobj->wo_FileSize) {
	return(refobj);
    }
    else return(NULL);
}

/*
	This routine is used by the ViewBy logic to temporarely move objects
	from the Children list to the Utility list.  They are then moved
	back on to the Children list (by InsertByText) in an order determined
	by the ViewBy mode.  Normally, when an object is moved off of the
	Children list, its UseCount and its drawer's UseCount would be
	decremented to reflect the fact that the object is no longer
	being referenced.  Since the move here is temporary, no decrementation
	is done.
*/
UtilityAdd( obj, drawer )
struct WBObject *obj;
struct WBObject *drawer;
{
    struct WorkbenchBase *wb = getWbBase();

    MP(( "UtilityAdd: adding obj %lx/%s drawer %lx/%s\n",
	obj, obj->wo_Name, drawer, drawer->wo_Name ));

    ADDTAIL( &wb->wb_UtilityList, &obj->wo_UtilityNode );

    return( NULL );
}

InsertByText( obj, drawer )
struct WBObject *drawer;
struct WBObject *obj;
{
    struct WBObject *(*func)();
    struct NewDD *dd = drawer->wo_DrawerData;
    struct WBObject *newobj;

    MP(( "InsertByText: drawer %lx/%s, obj %lx/%s, dd %lx\n",
	drawer, drawer->wo_Name, obj, obj->wo_Name, dd ));

    switch( dd->dd_ViewModes ) {
    case DDVM_BYNAME:	func = insertByName;	break;
    case DDVM_BYDATE:	func = insertByDate;	break;
    case DDVM_BYSIZE:	func = insertBySize;	break;
#ifdef DEBUGGING
    default:
	MP(( "InsertByText: unknown view mode %ld\n", dd->dd_ViewModes ));
	Debug(0L);
	return( 1 );
#endif DEBUGGING
    }

    newobj = SiblingPredSearch( dd->dd_Children.lh_TailPred, func, obj );

    MP(( "InsertByText: after insert: newobj %lx/%s, obj %lx/%s\n",
	newobj, newobj ? newobj->wo_Name : "<null>", obj, obj->wo_Name ));

    /* voodoo: if newobj is zero, we are at the head of the list */
    Insert( &dd->dd_Children, &obj->wo_Siblings,
	newobj ? &newobj->wo_Siblings : NULL );

    return( 0 );
}

void SortByText( drawer )
struct WBObject *drawer;
{
    struct WorkbenchBase *wb = getWbBase();
    struct NewDD *dd;

MP(("SortByText: enter\n"));
    ASSERT( Drawer_P( drawer ) );

    dd = drawer->wo_DrawerData;

    /* can you say "n-squared" algorithm?  We insert nodes into
    ** the utility list by the new ordering; then relink them into
    ** the sibling list.
    **
    ** While we're at it, we compute the max name length & max string length.
    */

    MP(( "SortByText( 0x%lx/%s )\n", drawer, drawer->wo_Name ));

    /* zero out the utility list */
    NewList( &wb->wb_UtilityList );

    /* transfer all the nodes to the utility list */
    SiblingSuccSearch( dd->dd_Children.lh_Head, UtilityAdd, drawer );

    /* clear the sibling list */
    NewList( &dd->dd_Children );

    /* and move them back */
    UtilitySearch( InsertByText, drawer );

    /* and refresh the window */
    RefreshDrawer( drawer );
MP(("SortByText: exit\n"));
}
