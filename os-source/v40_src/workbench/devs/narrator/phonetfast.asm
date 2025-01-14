	TTL	'PHONETFAST.ASM'


*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************


          SECTION      speech


*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*
*		FAST PHONET ROUTINES
*
*	These routines are ASM versions of some of the more heavily
*	used PHONET routines that were originally coded in C.  Should
*	speed things up somewhat.
*
*	Currently FillBuffer and AdjustTrack are done here.
*
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


	PUBLIC	_FillBuffer


;void FillBuffer(Track)				C calling sequence
;struct	PhonetParms  *Track;

_FillBuffer:
	LINK	A5,#0				;Keep it in C style


*	;------ Get Track->Cumdur, Track->Segdur, and Track->Target

	MOVE.L	8(A5),A0			;Track

	MOVE.L	(A0),A1				;Track->Cumdur

	MOVEQ	#0,D0				;Track->Segdur
	MOVE.B	9(A0),D0
	BEQ.S	FillReturn			;Nothing to move

	MOVE.W	14(A0),D1			;Track->Target


*	;------ Fill Track->Cumdur with Track->Target for Track->Segdur

	SUBQ.W	#1,D0

FillTrack:
	MOVE.W	D1,(A1)+
	DBF	D0,FillTrack

FillReturn:
	UNLK	A5				;Fixup stack
	RTS					;   and return


	DSEG
	PUBLIC	_Last_dur,_This_dur

	CSEG
	PUBLIC	_AdjustTrack

;void AdjustTrack(Track)			C calling sequence
;struct	PhonetParms  *Track;

_AdjustTrack:
	LINK	A5,#0				;C linkage
	MOVEM.L	A2/A3,-(SP)			;Save some registers


*	;------	Move track data over so that beginning of track
	;	now corresponds to data for "This_dur".

	MOVE.L	8(A5),A0			;Track
	MOVE.L	4(A0),A2			;Track->Local (start of track)

	MOVE.L	#0,D0				;Track->Local + Last_dur
	MOVE.B	_Last_dur,D0			;   (start of This_dur data)
	ASL.L	#1,D0				;Adjust Last_dur for word offset
	MOVE.L	D0,A3				;Add to Track->Local
	ADD.L	A2,A3

	MOVEQ	#0,D0				;Get number of words to move
	MOVE.B	_This_dur,D0
	BEQ.S	AdjNomove			;Nothing to move
	SUBQ.W	#1,D0				;Adjust for DBF

MoveTrack:
	MOVE.W	(A3)+,(A2)+			;Move the data over	
	DBF	D0,MoveTrack


*	;------ Fixup Track variables to reflect new situation

AdjNomove:
	MOVE.L	A2,(A0)				;Update Track->Cumdur
	MOVE.W	-2(A2),16(A0)			;Update Track->Oldval
	MOVE.B	_This_dur,8(A0)			;Update Track->Olddur


	MOVEM.L	(SP)+,A2/A3			;Restore registers
	UNLK	A5				;Fixup stack
	RTS					;   and return

	END
