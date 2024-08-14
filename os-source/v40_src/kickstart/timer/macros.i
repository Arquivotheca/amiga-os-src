*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************


*************************************************************************
*
* macros.i
*
* Source Control
* ------ -------
* 
* $Id: macros.i,v 36.4 91/01/25 15:46:20 rsbx Exp $
*
* $Locker:  $
*
* $Log:	macros.i,v $
* Revision 36.4  91/01/25  15:46:20  rsbx
* Change to V37
* 
* Revision 36.3  90/04/01  19:13:01  rsbx
* RCS version change.
* 
* Revision 36.2  89/08/09  19:26:36  rsbx
* *** empty log message ***
* 
* Revision 36.1  89/08/09  18:10:26  rsbx
* *** empty log message ***
* 
* Revision 1.1  89/08/09  14:08:32  rsbx
* Initial revision
*
*
*************************************************************************


TD_DISABLE	MACRO	scratch,from
		move.l	\2,\1
		DISABLE	\1,NOFETCH
		ENDM


TD_ENABLE	MACRO	scratch,from
		move.l	\2,\1
		ENABLE	\1,NOFETCH
		ENDM


*		;------ compare two timevec structures.  End result
*		;------ should set condition codes like Ay-Ax
TIMCMP		MACRO	Ax,Ay,Dscratch
		MOVE.L	(\2),\3		; implicit TV_SECS
		CMP.L	(\1),\3		; implicit TV_SECS
		BNE.S	\@end
		MOVE.L	TV_MICRO(\2),\3
		CMP.L	TV_MICRO(\1),\3
\@end
		ENDM


LTIMCMP		MACRO	Ax,Ay,Dscratch,Xoffset,Yoffset
XOFFSET		SET	\4
YOFFSET		SET	\5
		MOVE.L	YOFFSET+TV_SECS(\2),\3
		CMP.L	XOFFSET+TV_SECS(\1),\3
		BNE.S	\@end
		MOVE.L	YOFFSET+TV_MICRO(\2),\3
		CMP.L	XOFFSET+TV_MICRO(\1),\3
\@end
		ENDM


*		;------ copy timvec Ax into Ay
TIMCPY		MACRO	Ax,Ay,Xoffset,Yoffset
		IFGE	NARG-3
XOFFSET		SET	\3
YOFFSET		SET	\4
		ENDC
		IFLT	NARG-3
XOFFSET		SET	0
YOFFSET		SET	0
		ENDC
		MOVE.L	XOFFSET+TV_SECS(\1),YOFFSET+TV_SECS(\2)
		MOVE.L	XOFFSET+TV_MICRO(\1),YOFFSET+TV_MICRO(\2)
		ENDM


