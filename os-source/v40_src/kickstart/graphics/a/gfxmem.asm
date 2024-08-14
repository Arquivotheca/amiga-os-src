*******************************************************************************
*
*	$Id: gfxmem.asm,v 39.4 92/09/03 15:36:12 spence Exp $
*
*******************************************************************************


	include 'exec/types.i'

******* graphics.library/GfxNew **********************************************
*
*   NAME
*       GfxNew -- allocate a graphics extended data structure (V36)
*
*   SYNOPSIS
* 	result = GfxNew( node_type );    
*	d0		 d0
*
*	struct ExtendedNode *GfxNew( ULONG);
*
*   FUNCTION
*	Allocate a special graphics extended data structure (each of which
*	begins with an ExtendedNode structure).  The type of structure to
*	be allocated is specified by the node_type identifier.
*
*   INPUTS
*	node_type = which type of graphics extended data structure to allocate. 
*		    (see gfxnodes.h for identifier definitions.)
*
*   RESULT
*	result = a pointer to the allocated graphics node or NULL if the 
*		 allocation failed.
*
*   BUGS
*
*   SEE ALSO
*	graphics/gfxnodes.h GfxFree() GfxAssociate() GfxLookUp()
*
******************************************************************************

******* graphics.library/GfxFree *********************************************
*
*   NAME
*       GfxFree -- free a graphics extended data structure (V36)
*
*   SYNOPSIS
*       GfxFree( node );
*       	      a0
*	
*	void GfxFree(struct ExtendedNode *);
*
*   FUNCTION
*	Free a special graphics extended data structure (each of which
*	begins with an ExtendedNode structure).
*
*   INPUTS
*	node = pointer to a graphics extended data structure obtained via
*	       GfxNew().
*
*   RESULT
*	the node is deallocated from memory. graphics will disassociate
*	this special graphics extended node from any associated data
*	structures, if necessary, before freeing it (see GfxAssociate()).
*
*   BUGS
*	an Alert() will be called if you attempt to free any structure 
*	other than a graphics extended data structure obtained via GfxFree().
*
*   SEE ALSO
*	graphics/gfxnodes.h GfxNew() GfxAssociate() GfxLookUp()
*
******************************************************************************

******* graphics.library/GfxLookUP *******************************************
*
*   NAME
*    	GfxLookUp -- find a graphics extended node associated with a 
*		     given pointer (V36)
*
*   SYNOPSIS
*       result = GfxLookUp( pointer );
*       d0		    a0
*
*	struct ExtendedNode *GfxLookUp( void *);
*
*   FUNCTION
*	Finds a special graphics extended data structure (if any) associated
*	with the pointer to a data structure (eg: ViewExtra associated with
*	a View structure).
*
*   INPUTS
*	pointer = a pointer to a data structure which may have an 
*		  ExtendedNode associated with it (typically a View ).
*
*   RESULT
*	result = a pointer to the ExtendedNode that has previously been
*		 associated with the pointer.
*		
*   BUGS
*
*   SEE ALSO
*	graphics/gfxnodes.h GfxNew() GfxFree() GfxAssociate()
*
******************************************************************************

******* graphics.library/GfxAssociate ****************************************
*
*   NAME
*	GfxAssociate -- associate a graphics extended node with a given pointer
*	                (V36)
*
*   SYNOPSIS
*       GfxAssociate(pointer, node);
*                    A0       A1
*
*	void GfxAssociate(VOID *, struct ExtendedNode *);
*
*   FUNCTION
*	Associate a special graphics extended data structure (each of which
*	begins with an ExtendedNode structure)  with another structure via
*	the other structure's pointer. Later, when you call GfxLookUp()
*	with the other structure's pointer you may retrieve a pointer
*	to this special graphics extended data structure, if it is
*	available.
*
*   INPUTS
*	pointer = a pointer to a data structure.
*	node = an ExtendedNode structure to associate with the pointer
*	
*   RESULT
*	an association is created between the pointer and the node such
*	that given the pointer the node can be retrieved via GfxLookUp().
*
*   BUGS
*
*   SEE ALSO
*	graphics/gfxnodes.h GfxNew() GfxFree() GfxLookUp()
*
******************************************************************************

* The hashing algorithm is just the sum of the 4 bytes in the address.
; ULONG __asm hashit(register __d0 ULONG l);

NUMENTRIES	equ	8

	xdef	_hashit

_hashit:
	move.b	d0,d1
	swap	d0
	add.b	d0,d1
	ror.l	#8,d0
	add.b	d0,d1
	swap	d0
	add.b	d0,d1
	move.b	d1,d0
	moveq	#(NUMENTRIES-1),d1
	and.l	d1,d0
	rts

	end
