#ifndef  CLIB_GADTOOLS_PROTOS_H
#define  CLIB_GADTOOLS_PROTOS_H

/*
**	$Id: gadtools_protos.h,v 39.2 92/03/24 15:14:44 peter Exp $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  INTUITION_INTUITION_H
#include <intuition/intuition.h>
#endif
#ifndef  UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif
#ifndef  LIBRARIES_GADTOOLS_H
#include <libraries/gadtools.h>
#endif

/*--- functions in V36 or higher (Release 2.0) ---*/

/* Gadget Functions */

struct Gadget *CreateGadgetA( unsigned long kind, struct Gadget *gad,
	struct NewGadget *ng, struct TagItem *taglist );
struct Gadget *CreateGadget( unsigned long kind, struct Gadget *gad,
	struct NewGadget *ng, Tag tag1, ... );
void FreeGadgets( struct Gadget *gad );
void GT_SetGadgetAttrsA( struct Gadget *gad, struct Window *win,
	struct Requester *req, struct TagItem *taglist );
void GT_SetGadgetAttrs( struct Gadget *gad, struct Window *win,
	struct Requester *req, Tag tag1, ... );

/* Menu functions */

struct Menu *CreateMenusA( struct NewMenu *newmenu, struct TagItem *taglist );
struct Menu *CreateMenus( struct NewMenu *newmenu, Tag tag1, ... );
void FreeMenus( struct Menu *menu );
BOOL LayoutMenuItemsA( struct MenuItem *firstitem, APTR vi,
	struct TagItem *taglist );
BOOL LayoutMenuItems( struct MenuItem *firstitem, APTR vi, Tag tag1, ... );
BOOL LayoutMenusA( struct Menu *firstmenu, APTR vi, struct TagItem *taglist );
BOOL LayoutMenus( struct Menu *firstmenu, APTR vi, Tag tag1, ... );

/* Misc Event-Handling Functions */

struct IntuiMessage *GT_GetIMsg( struct MsgPort *iport );
void GT_ReplyIMsg( struct IntuiMessage *imsg );
void GT_RefreshWindow( struct Window *win, struct Requester *req );
void GT_BeginRefresh( struct Window *win );
void GT_EndRefresh( struct Window *win, long complete );
struct IntuiMessage *GT_FilterIMsg( struct IntuiMessage *imsg );
struct IntuiMessage *GT_PostFilterIMsg( struct IntuiMessage *imsg );
struct Gadget *CreateContext( struct Gadget **glistptr );

/* Rendering Functions */

void DrawBevelBoxA( struct RastPort *rport, long left, long top, long width,
	long height, struct TagItem *taglist );
void DrawBevelBox( struct RastPort *rport, long left, long top, long width,
	long height, Tag tag1, ... );

/* Visuals Functions */

APTR GetVisualInfoA( struct Screen *screen, struct TagItem *taglist );
APTR GetVisualInfo( struct Screen *screen, Tag tag1, ... );
void FreeVisualInfo( APTR vi );

/*--- functions in V39 or higher (Release 3) ---*/

LONG GT_GetGadgetAttrsA( struct Gadget *gad, struct Window *win,
	struct Requester *req, struct TagItem *taglist );
LONG GT_GetGadgetAttrs( struct Gadget *gad, struct Window *win,
	struct Requester *req, Tag tag1, ... );

#endif   /* CLIB_GADTOOLS_PROTOS_H */
