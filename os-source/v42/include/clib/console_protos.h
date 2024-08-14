#ifndef  CLIB_CONSOLE_PROTOS_H
#define  CLIB_CONSOLE_PROTOS_H
/*
**	$Id: console_protos.h,v 36.6 90/11/07 15:33:36 darren Exp $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
#ifndef  EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif
#ifndef  DEVICES_INPUTEVENT_H
#include <devices/inputevent.h>
#endif
#ifndef  DEVICES_KEYMAP_H
#include <devices/keymap.h>
#endif
struct InputEvent *CDInputHandler( struct InputEvent *events,
	struct Library *consoleDevice );
LONG RawKeyConvert( struct InputEvent *events, STRPTR buffer, long length,
	struct KeyMap *keyMap );
/*--- functions in V36 or higher (Release 2.0) ---*/
#endif   /* CLIB_CONSOLE_PROTOS_H */
