*******************************************************************************
*
*	Source Control
*	--------------
*	$Id: gelsinternal.i,v 37.2 91/01/30 12:50:09 spence Exp $
*
*	$Locker:  $
*
*	$Log:	gelsinternal.i,v $
*   Revision 37.2  91/01/30  12:50:09  spence
*   *** empty log message ***
*   
*   Revision 37.1  91/01/30  11:12:40  spence
*   appended STRUCT PosCtlData for new MOveSprite() behaviour
*   
*   Revision 37.0  91/01/07  15:32:46  spence
*   initial switchover from V36
*   
*   Revision 33.3  90/07/27  16:39:11  bart
*   id
*   
*   Revision 33.2  90/03/28  09:27:47  bart
*   *** empty log message ***
*   
*   Revision 33.1  88/11/16  09:02:50  bart
*   *** empty log message ***
*   
*   Revision 33.0  86/05/17  15:23:50  bart
*   added to rcs for updating
*   
*
*******************************************************************************

***** gelsinternal.i *************************************************
*
*               Commodore-Amiga, Inc.
*
*******************************************************************
    IFND    GRAPHICS_GELSINTERNAL_I
GRAPHICS_GELSINTERNAL_I SET 1

    STRUCTURE	gelRect,0
    WORD	gR_rX
    WORD	gR_rY
    WORD	gR_rW
    WORD	gR_rH
    WORD	gR_rRealWW
    APTR	gR_rAddr
    LABEL	gR_SIZEOF

	STRUCTURE	PosCtlData,0
	APTR	pcd_Address
	ULONG	pcd_Data
	LABEL	pcd_SizeOf

    ENDC
