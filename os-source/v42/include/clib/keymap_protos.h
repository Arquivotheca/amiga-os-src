#ifndef  CLIB_KEYMAP_PROTOS_H
#define  CLIB_KEYMAP_PROTOS_H
/*
**	$Id: keymap_protos.h,v 36.4 90/07/19 16:05:16 darren Exp $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
#ifndef  DEVICES_INPUTEVENT_H
#include <devices/inputevent.h>
#endif
#ifndef  DEVICES_KEYMAP_H
#include <devices/keymap.h>
#endif
/*--- functions in V36 or higher (Release 2.0) ---*/
void SetKeyMapDefault( struct KeyMap *keyMap );
struct KeyMap *AskKeyMapDefault( void );
WORD MapRawKey( struct InputEvent *event, STRPTR buffer, long length,
	struct KeyMap *keyMap );
LONG MapANSI( STRPTR string, long count, STRPTR buffer, long length,
	struct KeyMap *keyMap );
#endif   /* CLIB_KEYMAP_PROTOS_H */
