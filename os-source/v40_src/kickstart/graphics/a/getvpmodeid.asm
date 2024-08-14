*******************************************************************************
*
*	$Id: GetVPModeID.asm,v 39.1 93/03/19 11:55:50 spence Exp $
*
*******************************************************************************

    IFD WASTE_ROMSPACE

    section	graphics


******* graphics.library/GetVPModeID *******************************************
*
*   NAME
*	GetVPModeID -- get the 32 bit DisplayID from a ViewPort. (V36)
*
*   SYNOPSIS
*	modeID =  GetVPModeID( vp )
*	d0		       a0
*
*	ULONG GetVPModeID( struct ViewPort *);
*
*   FUNCTION
*	returns the normal display modeID, if one is currently  associated 
*	with this ViewPort.
*
*   INPUTS
*	vp -- pointer to a ViewPort structure.
*
*   RESULT
*	
*	modeID -- a 32 bit DisplayInfoRecord identifier associated with 
*		  this ViewPort, or INVALID_ID.
*
*   NOTES
*	Test the return value of this function against INVALID_ID, not NULL.
*	(INVALID_ID is defined in graphics/displayinfo.h).
*
*   BUGS
*
*   SEE ALSO
*	graphics/displayinfo.h, ModeNotAvailable()
*
********************************************************************************


    xref    _getvpmodeid
    xdef    _GetVPModeID

_GetVPModeID:

    move.l a0,-(sp)
    jsr	   _getvpmodeid
    addq.l #4,sp
    rts

******* graphics.library/ModeNotAvailable **************************************
*
*   NAME
*	ModeNotAvailable -- check to see if a DisplayID isn't available. (V36)
*
*   SYNOPSIS
*	error =  ModeNotAvailable( modeID )
*	d0		           d0
*
*	ULONG	ModeNotAvailable( ULONG);
*
*   FUNCTION
*	returns an error code, indicating why this modeID is not available, 
*	or NULL if there is no reason known why this mode should not be there.
*
*   INPUTS
*	modeID -- a 32 bit DisplayInfoRecord identifier.
*
*   RESULT
*	error -- a general indication of why this modeID is not available,
*		 or NULL if there is no reason why it shouldn't be available.
*
*   NOTE
*	ULONG return values from this function are a proper superset of the
*	DisplayInfo.NotAvailable field (defined in graphics/displayinfo.h).
*
*   BUGS
*
*   SEE ALSO
*	graphics/displayinfo.h, GetVPModeID()
*
********************************************************************************

    xref    _modenotavailable
    xdef    _ModeNotAvailable

_ModeNotAvailable:

    move.l d0,-(sp)
    jsr	   _modenotavailable
    addq.l #4,sp
    rts

    ENDC

    end
