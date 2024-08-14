/******************************************************************************
*
*	Source Control
*	--------------
*	$Id: freegbuffers.c,v 39.1 92/01/21 14:51:19 chrisg Exp $
*
*	$Locker:  $
*
*	$Log:	freegbuffers.c,v $
*   Revision 39.1  92/01/21  14:51:19  chrisg
*    changed for utility protos
*   
*   Revision 39.0  91/08/21  17:33:52  chrisg
*   Bumped
*   
*   Revision 37.3  91/05/14  16:10:20  chrisg
*    added #include for exec protos
*   
*   Revision 37.2  91/05/13  14:36:02  chrisg
*   removed compiler warnings for lattice
*   made use prototypes
*   got rid of interface code.. made routines take
*   arguments in registers.
*   
*   Revision 37.1  91/02/12  15:48:25  spence
*   autodocs.
*   
*   
*   Revision 37.0  91/01/07  15:22:07  spence
*   initial switchover from V36
*   
*   Revision 33.2  90/07/27  16:37:06  bart
*   id
*   
*   Revision 33.1  90/03/28  09:27:17  bart
*   *** empty log message ***
*   
*   Revision 33.0  86/05/17  15:22:41  bart
*   added to rcs for updating
*   
*
******************************************************************************/


#include <exec/types.h>
#include <graphics/rastport.h>
#include <graphics/gels.h>
#include <graphics/gfxbase.h>
#include "/macros.h"
#include "gels.protos"

/****** graphics.library/FreeGBuffers ******************************************
*
*   NAME
*	FreeGBuffers -- Deallocate memory obtained by GetGBufers.
*
*   SYNOPSIS
*	FreeGBuffers(anOb, rp, db)
*	             A0    A1  D0
*
*	void FreeGBuffers(struct AnimOb *, struct RastPort *, BOOL);
*
*   FUNCTION
*	For each sequence of each component of the AnimOb,
*	deallocate memory for:
*	    SaveBuffer
*	    BorderLine
*	    CollMask and ImageShadow (point to same buffer)
*	    if db is set (user had used double-buffering) deallocate:
*	        DBufPacket
*	        BufBuffer
*
*   INPUTS
*	anOb = pointer to the AnimOb structure
*	rp   = pointer to the current RastPort
*	db   = double-buffer indicator (set TRUE for double-buffering)
*
*   RESULT
*
*   BUGS
*
*   SEE ALSO
*	GetGBuffers()  graphics/gels.h  graphics/rastport.h
*
*****************************************************************************/
void __asm FreeGBuffers(register __a0 struct AnimOb *anOb,
						register __a1 struct RastPort *RPort,
						register __d0 BOOL dbuf)
{
    struct AnimComp *acptr;
    struct AnimComp *seqptr;
    struct Bob *bptr;
    struct VSprite *sptr;
    WORD sw;
    WORD ibufsize, pbufsize;
 
    for(acptr = anOb->HeadComp; acptr; acptr = acptr->NextComp)
    {
        seqptr = acptr;

        do
	{
            bptr = seqptr->AnimBob;
            sptr = bptr->BobVSprite;

            sw = sptr->Width << 1; /* byte count of row */

            ibufsize = umuls(sptr->Height, sw);

            pbufsize = umuls(RPort->BitMap->Depth,ibufsize);
 
            FreeMem(bptr->SaveBuffer,pbufsize);
            if (dbuf)
	    {      
                FreeMem(bptr->DBuffer->BufBuffer,pbufsize);
                FreeMem(bptr->DBuffer,sizeof(struct DBufPacket));
	    };
            FreeMem(sptr->BorderLine,sw);
 
            FreeMem(sptr->CollMask,ibufsize);
            seqptr = seqptr->NextSeq;
	}
        while ((seqptr != acptr) && (seqptr));
    }
}
 
