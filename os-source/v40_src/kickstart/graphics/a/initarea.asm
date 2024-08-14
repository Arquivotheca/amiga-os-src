*******************************************************************************
*
*	$Id: InitArea.asm,v 39.0 91/08/21 17:25:55 chrisg Exp $
*
*******************************************************************************

	include 'exec/types.i'      * Required data type definitions
	include 'graphics/rastport.i'       * RastPort/AreaInfo structures

	section	graphics
	xdef    InitArea,_InitArea          * Define entry as public symbol

******* graphics.library/InitArea ********************************************
* 
*   NAME   
* 
*	InitArea -- Initialize vector collection matrix
* 
*   SYNOPSIS
* 
*   	InitArea( areainfo, buffer, maxvectors )
*		    a0          a1      d0
*
*	void InitArea(struct AreaInfo *, void *, SHORT);
*	
*   FUNCTION
*	This function provides initialization for the vector collection matrix
*	such that it has a size of (max vectors ).  The size of the region
*	pointed to by buffer (short pointer) should be five (5) times as large
*	as maxvectors. This size is in bytes.  Areafills done by using AreaMove,
*	AreaDraw, and AreaEnd must have enough space allocated in this table to
*	store all the points of the largest fill. AreaEllipse takes up two
*	vectors for every call. If AreaMove/Draw/Ellipse detect too many
*	vectors going into the buffer they will return -1.
*
*   INPUTS
*	areainfo - pointer to AreaInfo structure
*	buffer - pointer to chunk of memory to collect vertices
*	maxvectors - max number of vectors this buffer can hold
*
*   RESULT
*	Pointers are set up to begin storage of vectors done by
*	AreaMove, AreaDraw, and AreaEllipse.
* 
*   BUGS
*
*   SEE ALSO
*	AreaEnd() AreaMove() AreaDraw() AreaEllipse() graphics/rastport.h
* 
******************************************************************************
*                                       INITIALIZE VECTOR COLLECTION MATRIX
InitArea:
_InitArea:
	    move.l  a1,ai_VctrPtr(a0)           * ai->VctrPtr = v;
	    move.l  a1,ai_VctrTbl(a0)           * ai->VctrTbl = v;
	    move    d0,ai_MaxCount(a0)          * ai->MaxCount = nv;
	    asl.l   #2,d0                       * nv = nv + nv (v is short)
	    adda    d0,a1                       * A1 = &v[nv+nv]
	    move.l  a1,ai_FlagPtr(a0)           * ai->FlagPtr = &v[nv+nv];
	    move.l  a1,ai_FlagTbl(a0)           * ai->FlagTbl = &v[nv+nv];
	    clr     ai_Count(a0)                * ai->Count = 0;
	    rts                                 * Exit to caller

	    end                                 * End of _InitArea
