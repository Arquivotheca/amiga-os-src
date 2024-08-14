	OPTIMON

;---------------------------------------------------------------------------

	INCLUDE "devices/inputevent.i"
	INCLUDE "commodities.i"

;---------------------------------------------------------------------------

	XDEF	_MatchIXLVO

;---------------------------------------------------------------------------

;  BOOL MatchIXLVO(struct Inputevent *ie, struct InputXpression *ix);
;  D0              A0                     A1

_MatchIXLVO:
	move.l    d2,-(a7)
	move.b    ix_Class(a1),d0
	beq.s     Success

	cmp.b     ie_Class(a0),d0
	bne.s     Fail

	move.w    ix_Code(a1),d0
	move.w    ie_Code(a0),d1
	eor.w     d0,d1
	and.w     ix_CodeMask(a1),d1
	bne.s     Fail

	move.w    ie_Qualifier(a0),d0
	move.w    ix_QualSame(a1),d1
	beq.s     TestQualMask

TestShiftSyn:
	btst      #0,d1     ; are lshift and rshift the same?
	beq.s     TestCapsSyn
	move.w    d0,d2
	andi.w    #IXSYM_SHIFTMASK,d2
	beq.s     TestCapsSyn
	ori.w     #IXSYM_SHIFTMASK,d0

TestCapsSyn:
	btst      #1,d1     ; is caps the same as shift?
	beq.s     TestAltSyn
	move.w    d0,d2
	andi.w    #IXSYM_CAPSMASK,d2
	beq.s     TestAltSyn
	ori.w     #IXSYM_CAPSMASK,d0

TestAltSyn:
	btst	  #2,d1     ; are lalt and ralt the same?
	beq.s	  TestQualMask
	move.w	  d0,d2
	andi.w	  #IXSYM_ALTMASK,d2
	beq.s	  TestQualMask
	ori.w     #IXSYM_ALTMASK,d0

TestQualMask:
	move.w    ix_Qualifier(a1),d1
	eor.w     d1,d0
	and.w     ix_QualMask(a1),d0
	bne.s     Fail

Success:
	moveq     #1,d0
	move.l    (a7)+,d2
	rts

Fail:
	moveq     #0,d0
	move.l    (a7)+,d2
	rts

;---------------------------------------------------------------------------

	END
