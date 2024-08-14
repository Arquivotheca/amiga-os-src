
	SECTION KCmpStr
	INCLUDE "assembly.i"
	

******* debug.lib/KCmpStr ***********************************************
*
*   NAME
*	KCmpStr - compare two null terminated strings
*
*   SYNOPSIS
*	mismatch = KCmpStr(string1, string2)
*	D0         A0        A1
*
*   FUNCTION
*	string1 is compared to string2 using the ASCII collating
*	sequence.  0 indicates the strings are identical.
**********************************************************************

   xdef	_cmpstrexec
_cmpstrexec:

   xdef	_KCmpStr			* (source1,source2)
_KCmpStr:
		movem.l	4(sp),A0/A1

    XDEF    KCmpStr
KCmpStr:
	    MOVEQ   #-1,D0
sc1:	    MOVE.B  (A0)+,D1
	    BEQ.S   sc2
	    CMP.B   (A1)+,D1
	    DBNE    D0,sc1

	    NEG.L   D0
	    BRA.S   sc3

sc2:	    CMP.B   (A1)+,D1
	    BNE.S   sc3
	    CLEAR   D0
sc3:
	    RTS

	END
