*******************************************************************************
*
*	$Id: gasm.asm,v 39.0 91/08/21 17:28:52 chrisg Exp $
*
*******************************************************************************

    
USE_GLUE_CODE	equ	0
	ifne	USE_GLUE_CODE
	xdef    _CBump
    xref    _ucbump
_CBump:
*               current routine calls a C subroutine to do the work
    move.l  a1,-(sp)    * push user copper list ptr on stack
    jsr _ucbump     * call C routine
    addq.l #4,sp        * remove args from stack
    rts

    xdef    _UCopperListInit
    xref    _ucinit
******* graphics.library/CINIT *************************************************
*
*   NAME
*	CINIT -- Initialize user copperlist to accept intermediate 
*		 user copper instructions.
*
*   SYNOPSIS
* 	cl = CINIT( ucl , n )
*
*	cl = UCopperListInit( ucl , n )
*			      a0    d0
*
*	struct CopList *UCopperListInit( struct UCopList *, UWORD );
*
*   FUNCTION
*	Allocates and/or initialize copperlist structures/buffers
*	internal to a UCopList structure.
*
*	This is a macro that calls UCopListInit. You must pass a 
*	(non-initialized) UCopList to CINIT (CINIT will NOT allocate 
*	a new UCopList if ucl==0 ). If (ucl != 0) it will initialize the
*	intermediate data buffers internal to a UCopList. 
*
*	The maximum number of intermediate copper list instructions
*	that these internal CopList data buffers contain is specified
*	as the parameter n.
*
*   INPUTS
*	ucl - pointer to UCopList structure
*	n - number of instructions buffer must be able to hold
*
*   RESULTS
*	cl- a pointer to a buffer which will accept n intermediate copper 
*	    instructions. 
*
*	NOTE: this is NOT a UCopList pointer, rather a pointer to the 
*	      UCopList's->FirstCopList sub-structure.
*		
*   BUGS
*	CINIT will not actually allocate a new UCopList if ucl==0.
*	Instead you must allocate a block MEMF_PUBLIC|MEMF_CLEAR, the 
*	sizeof(struct UCopList) and pass it to this function.  
*
*	The system's FreeVPortCopLists function will take care of 
*	deallocating it if they are called.
*
*	Prior to release V36 the  CINIT macro had { } braces surrounding
*	the definition, preventing the proper return of the result value.
*	These braces have been removed for the V36 include definitions.
*
*   SEE ALSO
* 	CINIT CMOVE CEND graphics/copper.h
*
*******************************************************************************

_UCopperListInit:
*               current routine calls a C subroutine to do the work
    move.l  d0,-(sp)	* num of copper instructions if a1==0
    move.l  a0,-(sp)    * push user copper list ptr on stack
    jsr _ucinit		* call C routine
    lea 8(sp),sp       	* remove args from stack
    rts

    xdef    _CMove
    xref    _ucmove
******* graphics.library/CMOVE *************************************************
*
*   NAME
*	CMOVE -- append copper move instruction to user copper list.
*
*   SYNOPSIS
*	CMOVE( c , a , v )
*
*	CMove( c , a , v )
*	      a1  d0  d1
*	CBump( c )
*	      a1
*
*	void CMove( struct UCopList *, void *, WORD );
*
*   FUNCTION
*	Add instruction to move value v to hardware register a.
*
*   INPUTS
*	c - pointer to UCopList structure
*	a - hardware register
*	v - 16 bit value to be written
*
*   RESULTS
*	This is actually a macro that calls CMove(c,&a,v)
*	and then calls CBump(c) to bump the local pointer
*	to the next instruction. Watch out for macro side affects.
*
*   BUGS
*	
*   SEE ALSO
*	CINIT CWAIT CEND graphics/copper.h
*
********************************************************************************
_CMove:
*               current routine calls a C subroutine to do the work
    move.l  d1,-(sp)
    move.l  d0,-(sp)
    move.l  a1,-(sp)    * push user copper list ptr on stack
    jsr _ucmove     * call C routine
    lea 12(sp),sp       * remove args from stack
    rts

    xdef    _CWait
    xref    _ucwait
******* graphics.library/CWAIT *************************************************
*
*   NAME
*	CWAIT -- Append copper wait instruction to user copper list.
*
*   SYNOPSIS
*	CWAIT( c , v , h )
*
*	CWait( c , v , h )
*	       a1  d0  d1
*	CBump( c )
*	      a1
*
*	void CWait( struct UCopList *, WORD, WORD)
*
*   FUNCTION
*	Add instruction to wait for vertical beam position v and
*	horizontal position h to this intermediate copper list.
*
*   INPUTS
*	c - pointer to UCopList structure
*	v - vertical beam position (relative to top of viewport)
*	h - horizontal beam position
*
*   RESULTS
*	this is actually a macro that calls CWait(c,v,h)
*	and then calls CBump(c) to bump the local pointer
*	to the next instruction.
*
*   BUGS
*	User waiting for horizontal values of greater than 222 decimal 
*	is illegal.
*
*   SEE ALSO
* 	CINIT CMOVE CEND graphics/copper.h
*
*******************************************************************
_CWait:
*               current routine calls a C subroutine to do the work
    move.l  d1,-(sp)
    move.l  d0,-(sp)
    move.l  a1,-(sp)    * push user copper list ptr on stack
    jsr _ucwait     * call C routine
    lea 12(sp),sp       * remove args from stack
    rts


******* graphics.library/CEND ***************************************************
*   NAME
*	CEND -- Terminate user copper list.
*
*   SYNOPSIS
*	CEND( c )
*
*	struct UCopList *c;
*
*   FUNCTION
*	Add instruction to terminate user copper list.
*
*   INPUTS
*	c - pointer to UCopList structure
*
*   RESULTS
*	This is actually a macro that calls the macro CWAIT(c,10000,255)
*	10000 is a magical number that the graphics.library uses.
*	I hope display technology doesn't catch up too fast!
*
*   BUGS
*
*   SEE ALSO
*	CINIT CWAIT CMOVE graphics/copper.h
********************************************************************************

******* graphics.library/CBump **************************************************
*   NAME
*	CBump -  increment user copper list pointer (bump to next position in list).
*
*   SYNOPSIS
*	CBump( c )
*	      a1
*
*	void CBump( struct UCopList * );
*
*   FUNCTION
*	Increment pointer to space for next instruction in user copper list.
*
*   INPUTS
*	c - pointer to UCopList structure
*
*   RESULTS
*	User copper list pointer is incremented to next position.  
*	Pointer is repositioned to next user copperlist instruction block 
*	if the current block is full.
*
*	    Note: CBump is usually invoked for the programmer as part of the
*	          macro definitions CWAIT or CMOVE.
*
*   BUGS
*
*   SEE ALSO
*	CINIT CWAIT CMOVE CEND graphics/copper.h
********************************************************************************

******* graphics.library/SetOPen ***********************************************
*
*   NAME
*	SetOPen -- Change the Area OutLine pen and turn on Outline
*			mode for areafills.
*
*   SYNOPSIS
*	SetOPen(rp, pen)
*
*	void SetOPen( struct RastPort *, UBYTE );
*
*   FUNCTION
*	This is implemented as a c-macro.
*	Pen is the pen number that will be used to draw a border
*	around an areafill during AreaEnd().
*
*   INPUTS
*	rp = pointer to RastPort structure
*	pen = number  between 0-255
*
*   BUGS
*
*   SEE ALSO
*	AreaEnd() graphics/gfxmacros.h graphics/rastport.h
*
********************************************************************************

	endc		* USE_GLUE_CODE
	end
