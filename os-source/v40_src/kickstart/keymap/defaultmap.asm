**
**	$Id: defaultmap.asm,v 36.8 92/09/17 15:15:23 darren Exp $
**
**      manipulate default keymap
**
**      (C) Copyright 1985,1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	keymap

**	Included Files

	INCLUDE	"kldata.i"


**	Exported Functions

	XDEF	AskKeyMapDefault
	XDEF	SetKeyMapDefault

	
******* keymap.library/AskKeyMapDefault ******************************
*
*   NAME
*	AskKeyMapDefault -- Ask for a pointer to the current default
*	                    keymap. (V36)
*
*   SYNOPSIS
*	keyMap = AskKeyMapDefault()
*
*	struct KeyMap *AskKeyMapDefault( VOID );
*
*   FUNCTION
*	Return a pointer to the keymap used by the keymap library for
*	MapRawKey and MapANSI when a keymap is not specified. 
*
*   RESULTS
*	keyMap - a pointer to a keyMap structure.  This key map is
*	    guaranteed to be permanently allocated: it will remain in
*	    memory till the machine is reset.
*
*   BUGS
*	The keymap.h include file should be in the libraries/ or perhaps
*	resources/ directory, but is in the devices/ directory for
*	compatibility reasons.
*
*   SEE ALSO
*	devices/keymap.h, keymap.library/SetKeyMapDefault(),
*	console.device ...KEYMAP functions
*
**********************************************************************
AskKeyMapDefault:
		move.l	kl_KMDefault(a6),d0
		rts


******* keymap.library/SetKeyMapDefault ******************************
*
*   NAME
*	SetKeyMapDefault -- Set the current default keymap. (V36)
*
*   SYNOPSIS
*	SetKeyMapDefault(keyMap)
*
*	void SetKeyMapDefault( struct KeyMap * );
*
*   FUNCTION
*	A pointer to key map specified is cached by the keymap library
*	for use by MapRawKey and MapANSI when a keymap is not specified. 
*
*   INPUTS
*	keyMap - a pointer to a keyMap structure.  This key map must be
*	    permanently allocated: it must remain in memory till the
*	    machine is reset.  It is appropriate that this keyMap be a
*	    node on the keymap.resource list.
*
*   BUGS
*	The keymap.h include file should be in the libraries/ or perhaps
*	resources/ directory, but is in the devices/ directory for
*	compatability reasons.
*
*   SEE ALSO
*	devices/keymap.h, keymap.library/AskKeyMapDefault(),
*	console.device ...KEYMAP functions
*
********************************************************************************
SetKeyMapDefault:
		move.l	a0,kl_KMDefault(a6)
		rts

	END
