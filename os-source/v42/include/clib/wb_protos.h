#ifndef  CLIB_WB_PROTOS_H
#define  CLIB_WB_PROTOS_H
/*
**	$Id: wb_protos.h,v 38.4 92/05/31 16:34:03 mks Exp $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
/*--- functions in V36 or higher (Release 2.0) ---*/
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  DOS_DOS_H
#include <dos/dos.h>
#endif
#ifndef  WORKBENCH_WORKBENCH_H
#include <workbench/workbench.h>
#endif
#ifndef  INTUITION_INTUITION_H
#include <intuition/intuition.h>
#endif
#ifndef  UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif

struct AppWindow *AddAppWindowA( unsigned long id, unsigned long userdata,
	struct Window *window, struct MsgPort *msgport,
	struct TagItem *taglist );
struct AppWindow *AddAppWindow( unsigned long id, unsigned long userdata,
	struct Window *window, struct MsgPort *msgport, Tag tag1, ... );

BOOL RemoveAppWindow( struct AppWindow *appWindow );

struct AppIcon *AddAppIconA( unsigned long id, unsigned long userdata,
	UBYTE *text, struct MsgPort *msgport, struct FileLock *lock,
	struct DiskObject *diskobj, struct TagItem *taglist );
struct AppIcon *AddAppIcon( unsigned long id, unsigned long userdata,
	UBYTE *text, struct MsgPort *msgport, struct FileLock *lock,
	struct DiskObject *diskobj, Tag tag1, ... );

BOOL RemoveAppIcon( struct AppIcon *appIcon );

struct AppMenuItem *AddAppMenuItemA( unsigned long id, unsigned long userdata,
	UBYTE *text, struct MsgPort *msgport, struct TagItem *taglist );
struct AppMenuItem *AddAppMenuItem( unsigned long id, unsigned long userdata,
	UBYTE *text, struct MsgPort *msgport, Tag tag1, ... );

BOOL RemoveAppMenuItem( struct AppMenuItem *appMenuItem );

/*--- functions in V39 or higher (Release 3) ---*/


void WBInfo( BPTR lock, STRPTR name, struct Screen *screen );


#endif   /* CLIB_WB_PROTOS_H */
