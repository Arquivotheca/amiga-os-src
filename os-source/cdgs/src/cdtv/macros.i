
NEWLISTX    MACRO   ; list
	    MOVE.L  \1,LH_HEAD(\1)
	    ADDQ.L  #4,(\1)	;Get address of LH_TAIL
	    CLR.L   LH_TAIL(\1)	;Clear LH_TAIL
	    MOVE.L  \1,LH_TAILPRED(\1)    ;Address of LH_TAIL to LH_HEAD
	    ENDM


ADDTAILX    MACRO   ; A0-list(destroyed) A1-node D0-(destroyed)
	    LEA     LH_TAIL(A0),A0
	    MOVE.L  LN_PRED(A0),D0
	    MOVE.L  A1,LN_PRED(A0)
	    MOVE.L  A0,(A1)
	    MOVE.L  D0,LN_PRED(A1)
	    MOVE.L  D0,A0
	    MOVE.L  A1,(A0)
	    ENDM


