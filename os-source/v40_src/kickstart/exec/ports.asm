*	   ************************************************
*	   **						 **
*	   **	   --------------------------------	 **
*	   **	    ROM OPERATING SYSTEM EXECUTIVE	 **
*	   **	   --------------------------------	 **
*	   **						 **
*	   **		     Message Ports		 **
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
*	$Id: ports.asm,v 39.2 92/05/28 19:04:27 mks Exp $
*
*	$Log:	ports.asm,v $
* Revision 39.2  92/05/28  19:04:27  mks
* Changed NEWLIST a0 to a bsr NewList to save ROM space
* 
* Revision 39.1  92/03/16  13:32:09  mks
* Fixed FindPort() and optimized the RemPort case
*
* Revision 39.0  91/10/15  08:28:17  mks
* V39 Exec initial checkin
*
*********************************************************************


;****** Included Files ***********************************************

    NOLIST
    INCLUDE	"assembly.i"

    INCLUDE	"types.i"
    INCLUDE	"nodes.i"
    INCLUDE	"lists.i"
    INCLUDE	"ports.i"
    INCLUDE	"execbase.i"
    INCLUDE	"ables.i"

    INCLUDE	"calls.i"
    LIST


;****** Imported Globals *********************************************

    INT_ABLES
    TASK_ABLES

    EXTERN_BASE PortList


;****** Imported Functions *******************************************

    EXTERN_CODE AddNode
    EXTERN_CODE	FindNode
    EXTERN_CODE AddTail
    EXTERN_CODE RemHead
    EXTERN_CODE Enqueue
    EXTERN_CODE Exqueue
    EXTERN_CODE NewList

    EXTERN_SYS	Signal
    EXTERN_SYS	Wait
    EXTERN_SYS	PutMsg
    EXTERN_SYS	Cause
    EXTERN_SYS	GetMsg
    EXTERN_SYS	ReplyMsg


;****** Exported Functions *******************************************

    XDEF	AddPort
;    XDEF	RemPort
    XDEF	PutMsg
    XDEF	GetMsg
    XDEF	ReplyMsg
    XDEF	WaitPort
    XDEF	FindPort


*****o* exec.library/AddPort ***********************************************
*
*   NAME
*	AddPort -- add a message port to the system
*
*   SYNOPSIS
*	AddPort(port)
*		A1
*
*   FUNCTION
*	This function attaches a message port structure to the
*	system's message port list.  The name and priority fields
*	of the port structure should be initialized prior to
*	calling this function.	If the user does not require the
*	name and priority fields, they should be initialized to
*	zero.  As with the name field in other system list items,
*	the name is useful when more than one task needs to
*	rendezvous on a port.
*
*	Once a port has been added to the naming list, you must
*	be careful to remove the port from the list (via
*	RemPort) before deallocating its memory.
*
*   INPUTS
*	port - pointer to a message port
*
*   SEE ALSO
*	RemPort, FindPort
*
**********************************************************************

AddPort:
	    LEA     MP_MSGLIST(A1),A0
	    bsr     NewList		; (a0)
	    LEA     PortList(A6),A0
	    BRA     AddNode


*****o* exec.library/RemPort ***********************************************
*
*   NAME
*	RemPort -- remove a message port from the system
*
*   SYNOPSIS
*	RemPort(port)
*		A1
*
*   FUNCTION
*	This function removes a message port structure from the
*	system's message port list.  Subsequent attempts to
*	rendezvous by name with this port will fail.
*
*   INPUTS
*	port - pointer to a message port
*
*   SEE ALSO
*	AddPort, FindPort
*
**********************************************************************

;RemPort:	equ	RemNode		; Just do RemNode


*****o* exec.library/ReplyMsg **********************************************
*
*   NAME
*	ReplyMsg -- put a message to its reply port
*
*   SYNOPSIS
*	ReplyMsg(message)
*		 A1
*
*   FUNCTION
*	This function sends a message to its reply port.  This is
*	usually done when the receiver of a message has finished
*	and wants to return it to the sender (so that it can be
*	re-used or deallocated, whatever).
*
*   INPUT
*	message - a pointer to the message
*
*   SEE ALSO
*	ReplyMsg
*
**********************************************************************
*
*	A historic Neil comment.  You bet it is a critical section!
*	A task switch after marking, but before adding could
*	cause WaitIO() to screw up.
*
*	    ;------ the marking as "reply" is a critical section (sigh).
*

ReplyMsg:   MOVEQ   #NT_REPLYMSG,D0	* prepare for putmsg

	    ;------ see if there is a reply port
	    MOVE.L  MN_REPLYPORT(A1),D1
	    MOVE.L  D1,A0
	    BNE.S   PutMsg1	; tail recursion + doesn't use PutMsg vector
				;[A0-port, A1-message, D1-port, D0-type]

	    MOVE.B  #NT_FREEMSG,LN_TYPE(A1)
	    RTS

*****o* exec.library/PutMsg ************************************************
*
*   NAME
*	PutMsg -- put a message to a message port
*
*   SYNOPSIS
*	PutMsg(port, message)
*	       A0    A1
*
*   FUNCTION
*	This function attaches a message to a given message port.
*	It provides a fast, non-copying message sending mechanism.
*
*	Messages can be attached to only one port at a time.  The
*	message body can be of any size or form.  Because messages
*	are not copied, cooperating tasks share the same message
*	memory.  The sender task should not recycle the message
*	until it has been replied by the receiver.  Of course this
*	depends on the message handling conventions setup by the
*	involved tasks.  If the ReplyPort field is non-zero, when
*	the message is replied by the receiver, it will be sent back
*	to that port.
*
*	Any one of the following actions can be set to occur
*	when a message is put:
*		1. no special action
*		2. signal a given task
*		3. cause a software interrupt
*
*	The action is selected depending on the value set in
*	PB_ACTION of MP_FLAGS.
*
*   INPUT
*	port - pointer to a message port
*	message - pointer to a message
*
*   SEE ALSO
*	GetMsg, ReplyMsg
*
**********************************************************************

PutMsg:     MOVEQ   #NT_MESSAGE,D0	* send type in D0
	    MOVE.L  A0,D1		* save port

PutMsg1:	;[A0-port, A1-message, D0-type, D1-port]

*	    ------- hook message to port:
	    LEA     MP_MSGLIST(A0),A0
	    DISABLE

	    MOVE.B  D0,LN_TYPE(A1)	* Setting type must be in DISABLE!
	    ADDTAIL			* does not affect D1

	    MOVE.L  D1,A1
	    MOVE.L  MP_SIGTASK(A1),D1
	    BEQ.S   pm_exit
	    MOVE.B  MP_FLAGS(A1),D0
	    AND.W   #PF_ACTION,D0	* mask off interesting bits
	    BEQ.S   pm_signal

	    CMP.B   #PA_SOFTINT,D0
	    BNE.S   pm_call
	    MOVE.L  D1,A1
	    JSRSELF Cause		* software interrupt
pm_exit:    ENABLE
	    RTS				; exit


pm_call:    CMP.B   #PA_IGNORE,D0
	    BEQ.S   pm_exit
	    MOVE.L  D1,A0
	    JSR     (A0)                ; direct call (dangerous!)
	    BRA.S   pm_exit		; there are requests to document A1


pm_signal:  MOVE.B  MP_SIGBIT(A1),D0
	    ;prevent task switches, then ENABLE to prevent MIDI problems
	    FORBID
	    ENABLE
	    MOVE.L  D1,A1
	    CLEAR   D1
	    BSET.L  D0,D1
	    MOVE.L  D1,D0
	    JSRSELF Signal		;(A1,D0)
	    JMP_PERMIT			;exit & take your task switch



*****o* exec.library/GetMsg ************************************************
*
*   NAME
*	GetMsg -- get next message from a message port
*
*   SYNOPSIS
*	message = GetMsg(port)
*	D0		 A0
*
*   FUNCTION
*	This function receives a message from a given message port.
*	It provides a fast, non-copying message receiving mechanism.
*
*	The received message is removed from the message port.
*
*	This function will not wait.  If a message is not present
*	this function will return zero.  If a program must wait for
*	a message, it can Wait on the signal specified for the port
*	or use the WaitPort function.  There can only be one task
*	waiting for any given port.
*
*	Getting the message does not imply that the message is now
*	free to be reused. When the receiver is finished with the
*	message, it may ReplyMsg it.
*
*   INPUT
*	port - a pointer to the receiver message port
*
*   RESULT
*	message - a pointer to the first message available.  If
*		there are no messages, return zero.
*
*   SEE ALSO
*	PutMsg, ReplyMsg, WaitPort
*
**********************************************************************

GetMsg:
	    LEA     MP_MSGLIST(A0),A0
	    DISABLE
	    REMHEAD
	    IFNE    TORTURE_TEST
		move.l	d0,a0
		tst.l	d0
		beq.s	tt_NoNode
		move.l	#$DEAD0011,LN_SUCC(a0)
		move.l	#$DEAD0021,LN_PRED(a0)
tt_NoNode:
	    ENDC
	    ENABLE
	    RTS


*****o* exec.library/WaitPort **********************************************
*
*   NAME
*	WaitPort -- wait for a given port to be non-empty
*
*   SYNOPSIS
*	message = WaitPort(port)
*	D0		   A0
*
*   FUNCTION
*	This function waits for the given port to become non-empty.
*	If necessary, the Wait function will be called to wait for
*	the port signal.  If a message is already present at the
*	port, this function will return immediately.  The return
*	value is always a pointer to the first message queued (but
*	it is not removed from the queue.
*
*   INPUT
*	port - a pointer to the message port
*
*   RETURN
*	message - a pointer to the first available message
*
*   SEE ALSO
*	GetMsg
*
**********************************************************************

WaitPort:
	    MOVE.L  MP_MSGLIST+LH_HEAD(A0),A1	;Note: non-atomic test
	    TST.L   (A1)                * SUCC
	    BNE.S   wp_exit

	    MOVE.B  MP_SIGBIT(A0),D1
	    LEA     MP_MSGLIST+LH_HEAD(A0),A0
	    CLEAR   D0
	    BSET    D1,D0
	    MOVE.L  A2,-(SP)
	    MOVE.L  A0,A2

wp_wait:    JSRSELF Wait			;(D0)
	    MOVE.L  (A2),A1			;Note: non-atomic test
	    TST.L   (A1)
	    BEQ.S   wp_wait
	    MOVE.L  (SP)+,A2

wp_exit:
	    MOVE.L  A1,D0
	    RTS


*****o* exec.library/FindPort **********************************************
*
*   NAME
*	FindPort -- find a given system message port
*
*   SYNOPSIS
*	port = FindPort(name)
*	D0		A1
*
*   FUNCTION
*	This function will search the system message port list for
*	a port with the given name.  The first port matching this
*	name will be returned.
*
*   INPUT
*	name - name of the port to find
*
*   RETURN
*	port - a pointer to the message port, or zero if
*		not found.
*
*   EXAMPLE
*
*	int
*	putToPort( msg, name )
*	struct Message *msg;
*	char *name;
*	{
*	    int retval;
*	    struct MsgPort *port;
*	    struct MsgPort *FindPort();
*
*	    Forbid();
*	    port = FindPort( name );
*	    if( port ) {
*		PutMsg( port, msg );
*		retval = 1;
*	    } else {
*		retval = 0;
*	    }
*	    Permit();
*
*	    return( retval );
*	}
*
*
**********************************************************************
*
FindPort:	LEA.L	PortList(A6),A0
		bra	FindNode	* (a0,a1)

	END
