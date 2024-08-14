*	   ************************************************
*	   **						 **
*	   **	   --------------------------------	 **
*	   **	    ROM OPERATING SYSTEM EXECUTIVE	 **
*	   **	   --------------------------------	 **
*	   **						 **
*	   **		List and Queue Support		 **
*	   **						 **
*	   ************************************************

*************************************************************************
*									*
*   Copyright 1984,85,88,89,91 Commodore-Amiga, Inc.			*
*   All rights reserved.  No part of this program may be reproduced,	*
*   transmitted, transcribed, stored in retrieval system, or		*
*   translated into any language or computer language, in any form	*
*   or by any means, electronic, mechanical, magnetic, optical, 	*
*   chemical, manual or otherwise, without the prior written		*
*   permission of Commodore-Amiga, Incorporated.			*
*									*
*************************************************************************

**********************************************************************
*
*   Source Control:
*
*	$Id: lists.asm,v 39.2 92/05/28 19:04:24 mks Exp $
*
*	$Log:	lists.asm,v $
* Revision 39.2  92/05/28  19:04:24  mks
* Added an exported NewList function that is a replacement for the
* NEWLIST a0 macro.  (Used in initialization code... saved ROM space)
* 
* Revision 39.1  92/03/16  13:30:42  mks
* Code optimizations...
*
* Revision 39.0  91/10/15  08:27:54  mks
* V39 Exec initial checkin
*
**********************************************************************


;****** Included Files ***********************************************

    NOLIST
    INCLUDE	"assembly.i"
    INCLUDE	"types.i"
    INCLUDE	"nodes.i"
    INCLUDE	"lists.i"
    INCLUDE	"ables.i"
    INCLUDE	"calls.i"
    LIST

;****** Exported Functions *******************************************

    XDEF	Insert
    XDEF	AddHead
    XDEF	AddTail
    XDEF	Remove
    XDEF	RemHead
    XDEF	RemTail
    XDEF	AddNode 	;Protected Enqueue
    XDEF	RemNode 	;Protected REMOVE
    XDEF	FindNode	;Protected FindName
    XDEF	Enqueue
    XDEF	Exqueue
    XDEF	FindName
    XDEF	NewList		; For ROM Space... ;^)
;;  XDEF	FindNameNC

;****** Imported Globals *********************************************

    TASK_ABLES

;****** Imported Functions *******************************************




*****o* exec.library/NewList ************************************************
*
*   NAME
*	NewList - Initialize a list (minlist) header...
*
*   SYNOPSIS
*	NewList(list)
*	        A0
*
**********************************************************************
NewList:	NEWLIST	a0
		rts

*****o* exec.library/Insert ************************************************
*
*   NAME
*	Insert -- insert a node into a list
*
*   SYNOPSIS
*	Insert(list, node, listNode)
*	       A0    A1    A2
*
**********************************************************************

Insert:
	    MOVE.L  A2,D0		;68020: TST.L A2
	    BEQ.S   AddHead
	    MOVE.L  (A2),D0
	    BEQ.S   in_tail
	    MOVE.L  D0,A0
	    MOVEM.L D0/A2,(A1)          ; LN_SUCC,LN_PRED
	    MOVE.L  A1,LN_PRED(A0)
	    MOVE.L  A1,(A2)
	    RTS
in_tail:
	    MOVE.L  A2,(A1)
	    MOVE.L  LN_PRED(A2),A0
	    MOVE.L  A0,LN_PRED(A1)
	    MOVE.L  A1,LN_PRED(A2)
	    MOVE.L  A1,(A0)
	    RTS


*****o* exec.library/AddHead ***********************************************
*
*   NAME
*	AddHead -- insert node at the head of a list
*
*   SYNOPSIS
*	AddHead(list, node)
*		A0    A1
*
**********************************************************************

AddHead:
	    ADDHEAD
	    RTS


*****o* exec.library/AddTail ***********************************************
*
*   NAME
*	AddTail -- append node to tail of a list
*
*   SYNOPSIS
*	AddTail(list, node)
*		A0    A1
*
**********************************************************************

AddTail:
	    ADDTAIL
	    RTS



*****o* exec.library/Exqueue ***********************************************
*
*   NAME
*	Exqueue -- remove a node from a system queue
*
*   SYNOPSIS
*	Exqueue(list, node)
*		A0    A1
*
****************************************************************************

*****o* exec.library/Remove ************************************************
*
*   NAME
*	Remove -- remove a node from a list
*
*   SYNOPSIS
*	Remove(node)
*	       A1
*
**********************************************************************

;Protected REMOVE
RemPort:	xdef	RemPort
RemResource:	xdef	RemResource
RemSemaphore:	xdef	RemSemaphore
RemNode:
	    FORBID
	    REMOVE
	    JMP_PERMIT

Remove:
Exqueue:
	    REMOVE
	    RTS


*****o* exec.library/RemHead ***********************************************
*
*   NAME
*	RemHead -- remove the head node from a list
*
*   SYNOPSIS
*	node = RemHead(list)
*	D0	       A0
*
**********************************************************************

RemHead:
	    REMHEAD
	    RTS


*****o* exec.library/RemTail ***********************************************
*
*   NAME
*	RemTail -- remove the tail node from a list
*
*   SYNOPSIS
*	node = RemTail(list)
*	D0	       A0
*
**********************************************************************

RemTail:
	    REMTAIL
	    RTS



*****o* exec.library/Enqueue ***********************************************
*
*   NAME
*	Enqueue -- insert or append node to a system queue
*
*   SYNOPSIS
*	Enqueue(list, node)
*		A0    A1
*
**********************************************************************
*
*	NOTE: Exec assumes A1 preserved by Enqueue!
*


*------ protected Enqueue ------
AddNode:
	    FORBID
	    BSR.S   Enqueue
	    JMP_PERMIT

Enqueue:
	    MOVE.B  LN_PRI(A1),D1
	    MOVE.L  (A0),D0
en_next:
	    MOVE.L  D0,A0
	    MOVE.L  (A0),D0             ; next node
	    BEQ.S   en_end
	    CMP.B   LN_PRI(A0),D1
	    BLE.S   en_next		; if we cannot insert yet
en_end:
	    MOVE.L  LN_PRED(A0),D0
	    MOVE.L  A1,LN_PRED(A0)
	    MOVE.L  A0,(A1)
	    MOVE.L  D0,LN_PRED(A1)
	    MOVE.L  D0,A0
	    MOVE.L  A1,(A0)
	    RTS

*****o* exec.library/FindName **********************************************
*
*   NAME
*	FindName -- find a system list node with a given name
*
*   SYNOPSIS
*	node = FindName(start, name)
*	D0		A0     A1
*
**********************************************************************
FindNode:	; Protected FindName
		FORBID
		bsr.s	FindName
		JMP_PERMIT
*
FindName:
		MOVE.L	A2,-(SP)
		MOVE.L	A0,A2		    ; Start to A2
		MOVE.L	A1,D1		    ; Name to D1
		MOVE.L	(A2),D0
		BEQ.S	fn_exit 	    ; list tail

fn_next:	MOVE.L	D0,A2		    ; current node
		MOVE.L	(A2),D0             ; next node
		BEQ.S	fn_exit 	    ; end of list
		;---- found a node
		TST.L	LN_NAME(A2)
		BEQ.S	fn_next		    ; ignore if no name
		MOVE.L	LN_NAME(A2),A0
		MOVE.L	D1,A1
fn_char:	CMP.B	(A0)+,(A1)+
		BNE.S	fn_next
		TST.B	-1(A0)
		BNE.S	fn_char
		MOVE.L	A2,D0		    ; result

fn_exit:	;[D0-return value]
		MOVE.L	D1,A1
		MOVE.L	(SP)+,A2
		RTS


		IFEQ	1
*****i* exec.library/FindNameNC *********************************************
*
*   NAME
*	FindNameNC -- find a list node with a case-insensitive name (V36)
*
*   SYNOPSIS
*	node = FindNameNC(start, name)
*	D0		  A0	 A1
*
*   FUNCTION
*	Traverse a system list until a node with the given case-insensitive
*	name is found.	To find multiple occurances of a string, this
*	function may be called with a node starting point.
*
*   INPUTS
*	start - a list header or a list node to start the search
*		(if node, this one is skipped)
*	name - a pointer to a name string terminated with null
*
*   RESULTS
*	node - a pointer to the node with the same name else
*	    zero to indicate that the string was not found.
*
*****************************************************************************

FindNameNC:
		MOVEM.L D2/A2,-(SP)
		MOVE.L	A0,A2		    ; Start to A2
		MOVE.L	A1,D1		    ; Name to D1
		MOVE.L	(A2),D2
		BEQ.S	fnc_exit	    ; list tail

fnc_next:	MOVE.L	D2,A2		    ; current node
		MOVE.L	(A2),D2             ; next node
		BEQ.S	fnc_exit	    ; end of list
		;---- found a node
		MOVE.L	LN_NAME(A2),A0
		MOVE.L	D1,A1

		;[A0-candidate A1-master D0-scratch]
fnc_char:	MOVE.B	(A0)+,D0            ; candidate character
		BEQ.S	fnc_endsource	    ; check end...
		CMP.B	(A1)+,D0            ; compare with master
		BEQ.S	fnc_char	    ; match...
		BCHG.B	#6,D0		    ; toggle case bit
		CMP.B	-1(A1),D0
		BNE.S	fnc_next	    ; mismatch...

		;Check that character is capsable.  Valid ranges are
		;A-Z a-z and all extended characters except multiply,
		;divide, sharp S and umlaut y.
		BCLR.B	#5,D0		    ; cut case tests by two
		CMP.B	#'A',D0
		BLO.S	fnc_next	    ; mismatch (below A / a)...
		CMP.B	#'[',D0
		BLO.S	fnc_char	    ; match (below [ / {)...
		CMP.B	#$C0,D0
		BLO.S	fnc_next	    ; mismatch (below special A)...
		CMP.B	#$D7,D0
		BEQ.S	fnc_next	    ; mismatch (multiply / divide)...
		CMP.B	#$DF,D0
		BEQ.S	fnc_next	    ; mismatch (sharp s / umlaut y)...
		BRA.S	fnc_char	    ; passed the gauntlet!!!!...


fnc_endsource:	TST.B	(A1)
		BNE.S	fnc_next
		MOVE.L	A2,D2		    ; result

fnc_exit:	;[D2-return value]
		MOVE.L	D1,A1
		MOVE.L	(SP)+,A2/D2
		RTS
		ENDC

	    END
