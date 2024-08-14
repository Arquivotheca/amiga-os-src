*******************************************************************************
*
*	Source Control
*	--------------
*	$Id: sortvsprite.asm,v 37.0 91/01/07 15:28:30 spence Exp $
*
*	$Locker:  $
*
*	$Log:	sortvsprite.asm,v $
*   Revision 37.0  91/01/07  15:28:30  spence
*   initial switchover from V36
*   
*   Revision 33.1  90/07/27  16:39:21  bart
*   id
*   
*   Revision 33.0  86/05/17  15:23:14  bart
*   added to rcs for updating
*   
*
*******************************************************************************

	CODE
	XDEF	_sortVSprite
_sortVSprite:
.L14:
	MOVE.L	A2,-(SP)
.L15:
	MOVE.L	8(A7),A1
.L13:
*						*LEA	22(A1),A0
	MOVE.L	22(A1),D0			*MOVE.L	(A0),D0

	MOVE.L	4(A1),A2			*MOVE.L	4(A1),A0
*						*LEA	22(A0),A0
	CMP.L	22(A2),D0			*CMP.L	(A0),D0
	BGE.S	.L12
.L6:
*						*MOVE.L	4(A1),A2
.L5:
	MOVE.L	4(A2),A2
.L4:
*						*LEA	22(A2),A0
	CMP.L	22(A2),D0			*CMP.L	(A0),D0
	BLT	.L5
.L3:
	MOVE.L	(A1),A0
	
	MOVE.L	4(A1),D0
	MOVE.L	D0,4(A0)			* MOVE.L	4(A1),4(A0)
	MOVE.L	D0,A0				* MOVE.L	4(A1),A0

	MOVE.L	(A1),(A0)

	MOVE.L	(A2),A0
	MOVE.L	A1,4(A0)
	MOVE.L	A2,4(A1)
	MOVE.L	(A2),(A1)
	MOVE.L	A1,(A2)
	BRA.S	.L1
.L12:
	MOVE.L	(A1),A2				*MOVE.L	(A1),A0
*						*LEA	22(A0),A0
	CMP.L	22(A2),D0			*CMP.L	(A0),D0
	BLE.S	.L1
.L11
*						*MOVE.L	(A1),A2
.L10:
	MOVE.L	(A2),A2
.L9:
*						*LEA	22(A2),A0
	CMP.L	22(A2),D0			*CMP.L	(A0),D0
	BGT	.L10
.L8:
	MOVE.L	4(A1),A0
	
	MOVE.L	(A1),D0
	MOVE.L	D0,(A0)				*MOVE.L	(A1),(A0)
	MOVE.L	D0,A0				*MOVE.L	(A1),A0

	MOVE.L	4(A1),4(A0)

	MOVE.L	4(A2),D0
	MOVE.L	D0,A0				*MOVE.L	4(A2),A0
	MOVE.L	A1,(A0)
	MOVE.L	A2,(A1)
	MOVE.L	D0,4(A1)			*MOVE.L	4(A2),4(A1)
	MOVE.L	A1,4(A2)
.L1:
	MOVE.L	(SP)+,A2
	RTS	
	DATA
* ALLOCATIONS FOR _sortVSprite
*	A2	_t
*	D0	_q
*	8(FP)	_s
	CODE
	DATA
* ALLOCATIONS FOR MODULE
	CODE
	END
