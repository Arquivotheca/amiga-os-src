******************************************************************************
*
*	$Id: a2024vb.i,v 39.0 92/10/26 19:38:58 spence Exp $
*
******************************************************************************/

	IFND EXEC_TYPES_H
	include 'exec/types.i'
	ENDC

     STRUCTURE A2024VB,0
	APTR gbhc
	APTR poke
	APTR tobplptrs
	STRUCT colours,2*4*6
	STRUCT offset,4*4*6		; 4 bitplane pointers, 6 frames
	UWORD maxcount
     LABEL A2024VBSIZEOF
