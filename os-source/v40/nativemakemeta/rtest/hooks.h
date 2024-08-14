#ifndef UTILITY_HOOKS_H
#define UTILITY_HOOKS_H TRUE
/*
**	$Id: hooks.h,v 36.1 90/07/12 15:48:54 rsbx Exp $
**
**	callback hooks
**
**	(C) Copyright 1989,1990 Commodore-Amiga Inc.
**		All Rights Reserved
*/

#ifndef EXEC_TYPES_H
#include "exec/types.h"
#endif

#ifndef EXEC_NODES_H
#include "exec/nodes.h"
#endif

/* new standard hook structure */
struct Hook	{
    struct MinNode	h_MinNode;
    ULONG		(*h_Entry)();	/* assembler entry point	*/
    ULONG		(*h_SubEntry)();/* often HLL entry point	*/
    VOID		*h_Data;	/* owner specific		*/
};

/*
 * Hook calling conventions:
 *	A0 - pointer to hook data structure itself
 *	A1 - pointer to parameter structure ("message") typically
 *	     beginning with a longword command code, which makes
 *	     sense in the context in which the hook is being used.
 *	A2 - Hook specific address data ("object," e.g, GadgetInfo)
 *
 * Control will be passed to the routine h_Entry.  For many
 * High-Level Languages (HLL), this will be an assembly language
 * stub which pushes registers on the stack, does other setup,
 * and then calls the function at h_SubEntry.
 *
 * The C standard receiving code is:
 * CDispatcher( hook, object, message )
 *     struct Hook	*hook;
 *     APTR		object;
 *     APTR		message;
 *
 * NOTE that register natural order differs from this convention
 * for C parameter order, which is A0,A2,A1.
 *
 * The assembly language stub for "vanilla" C parameter conventions
 * could be:

 _hookEntry:
 	move.l	a1,-(sp)		; push message packet pointer
 	move.l	a2,-(sp)		; push object pointer
 	move.l	a0,-(sp)		; push hook pointer
	move.l	h_SubEntry(a0),a0	; fetch C entry point ...
	jsr	(a0)			; ... and call it
	lea	12(sp),sp		; fix stack
	rts

 * with this function as your interface stub, you can write
 * a Hook setup function as:

 SetupHook( hook, c_function, userdata )
 struct Hook	*hook;
 ULONG		(*c_function)();
 VOID		*userdata;
 {
 	ULONG	(*hookEntry)();

	hook->h_Entry =		hookEntry;
	hook->h_SubEntry =	c_function;
	hook->h_Data = 		userdata;
 }

 * with Lattice C pragmas, you can put the C function in the
 * h_Entry field directly if you declare the function:

ULONG __saveds __asm
CDispatcher(	register __a0 struct Hook	*hook,
		register __a2 VOID		*object,
		register __a1 ULONG		*message );
 *
 ****/

#endif
