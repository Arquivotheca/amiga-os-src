*************************************************************************
*                                                                       *
*   callhook.asm -- should probably be a macro
*                                                                       *
*   Copyright (C) 1985, 1989 Commodore Amiga Inc.  All rights reserved. *
*                                                                       *
*   $Id: callhook.asm,v 36.3 90/11/05 18:55:07 peter Exp $
*************************************************************************

	include	'exec/types.i'
	include	'hooks.i'

	xdef	CallHook

******* utility.library/CallHookPkt *************************************
*
*    NAME
*	CallHookPkt	--  (New for V36) Invoke a Hook function callback.
*
*    SYNOPSIS
*	CallHookPkt( struct Hook *Hook, VOID *Object, VOID *ParamPkt )
*				  A0		A2		A1
*
*    FUNCTION
*	Performs the callback standard defined by a Hook structure.
*	This function is really very simple; it effectively performs
*	a JMP to Hook->h_Entry.
*
*	It is probably just as well to do this operation in an
*	assembly language function linked in to your program, possibly
*	from a compiler supplied library or a builtin function.
*
*	It is anticipated that C programs will often call a 'varargs'
*	variant of this function which will be named CallHook.  This
*	function must be provided in a compiler specific library, but
*	an example of use would be:
*	
*	returnval = CallHook( hook, dataobject, COMMAND_ID, param1, param2 );
*	
*	This function CallHook can be implemented in many C compilers
*	like this:
*	CallHook( hook, object, command, ... )
*	struct Hook	*hook;
*	VOID		*object;
*	ULONG		command;
*	{
*		return ( CallHookPkt( hook, object, (VOID *) &command ) );
*	}
*
*    INPUTS
*	Hook = pointer to Hook structure as defined in utility/hooks.h
*	Object = useful data structure in the particular context the
*		hook is being used for.
*	ParamPkt = pointer to a parameter packet (often built on the
*		stack); by convention this packet should start off
*		with a longword command code, and the remaining data
*		in the packet depends on that command.
*
*    RESULT
*	The meaning of the value returned in D0 depends on the context
*	in which the Hook is being used.
*
*    BUGS
*
*    SEE ALSO
*	utility/hooks.h
*
*****************************************************************************

; since the registers are all set up by the time it gets
; here, this function doesn't do much but jump to h_Entry.
CallHook:
	move.l	h_Entry(a0),-(sp)
	rts;
