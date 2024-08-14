/******************************************************************************
*
*	Source Control
*	--------------
*	$Id: animation.c,v 42.1 93/07/20 13:40:14 chrisg Exp $
*
*	$Locker:  $
*
*	$Log:	animation.c,v $
 * Revision 42.1  93/07/20  13:40:14  chrisg
 * umuls gone.
 * 
 * Revision 42.0  93/06/16  11:19:16  chrisg
 * initial
 * 
 * Revision 42.0  1993/06/01  07:20:48  chrisg
 * *** empty log message ***
 *
*   Revision 39.0  91/08/21  17:33:02  chrisg
*   Bumped
*   
*   Revision 37.3  91/05/14  11:23:02  chrisg
*   changed to include gfxbase.h and macros.h for exec pragmas
*   
*   Revision 37.2  91/05/13  14:35:02  chrisg
*   removed compiler warnings for lattice
*   made use prototypes
*   got rid of interface code.. made routines take
*   arguments in registers.
*   
*   Revision 37.1  91/02/12  15:48:05  spence
*   autodocs
*   
*   Revision 37.0  91/01/07  15:21:56  spence
*   initial switchover from V36
*   
*   Revision 33.2  90/07/27  16:35:23  bart
*   id
*   
*   Revision 33.1  90/03/28  09:27:03  bart
*   *** empty log message ***
*   
*   Revision 33.0  86/05/17  15:22:08  bart
*   added to rcs for updating
*   
*
******************************************************************************/


/*** gel4.c ******************************************************************
 *
 *  gel4 contains the animation routines
 *
 *  Confidential Information: Amiga Computer, Inc.
 *  Copyright (c) Amiga Computer, Inc.
 *
 *                  Modification History
 *  date    :   author :    Comments
 *  -------     ------      ---------------------------------------
 *  9-28-84     -=RJ=-      added this header file
 *                          commented out all EXEC14 stuff
 *
 ****************************************************************************/


#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/rastport.h>
#include <graphics/gels.h>
#include <graphics/gfxbase.h>
#include <clib/exec_protos.h>
#include "/macros.h"
#include "gels.protos"

#define umuls(a,b) (((UWORD) a)*((UWORD) b))

/*???#define DEBUG 1*/

/****** graphics.library/AddAnimOb ********************************************
*
*   NAME
*	AddAnimOb  --  Add an AnimOb to the linked list of AnimObs.
*
*   SYNOPSIS
*	AddAnimOb(anOb, anKey, rp)
*	          A0    A1     A2
*
*	void AddAnimOb(struct AnimOb *,struct AnimOb **, struct RastPort *);
*
*   FUNCTION
*	Links this AnimOb into the current list pointed to by animKey.
*	Initializes all the Timers of the AnimOb's components.
*	Calls AddBob with each component's Bob.
*	rp->GelsInfo must point to an initialized GelsInfo structure.
*
*   INPUTS
*	anOb  = pointer to the AnimOb structure to be added to the list
*	anKey = address of a pointer to the first AnimOb in the list
*	        (anKey = NULL if there are no AnimObs in the list so far)
*	rp    = pointer to a valid RastPort
*
*   RESULT
*
*   BUGS
*
*   SEE ALSO
*	Animate() graphics/rastport.h graphics/gels.h
*****************************************************************************/
void __asm AddAnimOb(register __a0 struct AnimOb *anOb,
					 register __a1 struct AnimOb **animKey,
					 register __a2 struct RastPort *RPort)
// void _AddAnimOb(anOb, animKey, RPort)
// struct AnimOb *anOb, **animKey;
// struct RastPort *RPort;
{
    struct AnimComp *aC;
    struct AnimOb *aO;

    anOb->PrevOb = 0;

    if (aO = *animKey)
    {
        aO->PrevOb = anOb;
        anOb->NextOb = aO;
    }
    else anOb->NextOb = 0;

    *animKey = anOb;

    for(aC = anOb->HeadComp; aC; aC = aC->NextComp)
    {
        aC->Timer = aC->TimeSet;
        AddBob(aC->AnimBob, RPort );
    }
}


/****** graphics.library/Animate **********************************************
*
*   NAME
*	Animate  --  Processes every AnimOb in the current animation list.
*
*   SYNOPSIS
*	Animate(anKey, rp)
*	        A0     A1
*
*	void Animate(struct AnimOb **, struct RastPort *);
*
*   FUNCTION
*	For every AnimOb in the list
*	    - update its location and velocities
*	    - call the AnimOb's special routine if one is supplied
*	    - for each component of the AnimOb
*	        - if this sequence times out, switch to the new one
*	        - call this component's special routine if one is supplied
*	        - set the sequence's VSprite's y,x coordinates based
*	          on whatever these routines cause
*
*   INPUTS
*	ankey = address of the variable that points to the head AnimOb
*	rp    = pointer to the RastPort structure
*
*   RESULT
*
*   BUGS
*
*   SEE ALSO
*	AddAnimOb() graphics/gels.h graphics/rastport.h
*
*****************************************************************************/
void __asm Animate(register __a0 struct AnimOb **animKey,
					register __a1 struct RastPort *RPort)
{
    struct AnimOb *thisOb;
    struct AnimComp *thisComp;
    struct AnimComp *newComp;
    struct Bob *thisBob, *newBob;
    struct VSprite *thisVSprite, *newVSprite;

#ifdef DEBUG
    if (Debug) printf("Animate(%lx, %lx, %lx)\n",animKey, RPort);
#endif

    for(thisOb = *animKey; thisOb; thisOb = thisOb->NextOb)
    {
        thisOb->Clock++;

        thisOb->AnOldX = thisOb->AnX;
        thisOb->AnOldY = thisOb->AnY;

        thisOb->AnX += thisOb->XVel;
        thisOb->AnY += thisOb->YVel;

        thisOb->XVel += thisOb->XAccel;
        thisOb->YVel += thisOb->YAccel;

        if ((LONG)thisOb->AnimORoutine) (*thisOb->AnimORoutine)(thisOb);

        for(thisComp=thisOb->HeadComp; thisComp; thisComp=thisComp->NextComp)
	{
            /* start out by testing to switch to new image...
             *  if thisComp->Timer is zero, this image never switches
             *  if it reaches zero, it's time to switch
             */

            if ((thisComp->Timer) && (!(--thisComp->Timer)))
                /* OK!  Now change to the next image of this
                    animation sequence */
	    {
                newComp = thisComp->NextSeq;
                thisBob = thisComp->AnimBob;
                newBob = newComp->AnimBob;
                thisVSprite = thisBob->BobVSprite;
                newVSprite = newBob->BobVSprite;

		if(newBob != thisBob) /* avoid infinite loops - bart */
		{
		    if (newBob->Before = thisBob->Before)
			thisBob->Before->After = newBob;
		    if (newBob->After = thisBob->After)
			thisBob->After->Before = newBob;
		    newBob->Flags = thisBob->Flags;
		}

                newVSprite->X = thisVSprite->X;
                newVSprite->Y = thisVSprite->Y;

               RemBob(thisBob);
                AddBob(newBob, RPort);

                /* push this sprite all the way up left */
                thisVSprite->X = thisVSprite->Y = -32767;

                newVSprite->ClearPath = thisVSprite->ClearPath;
                newVSprite->DrawPath = thisVSprite->DrawPath;

                if (thisComp->PrevComp) thisComp->PrevComp->NextComp = newComp;
                if (thisComp->NextComp) thisComp->NextComp->PrevComp = newComp;
                newComp->NextComp = thisComp->NextComp;
                newComp->PrevComp = thisComp->PrevComp;

                newComp->Timer = newComp->TimeSet;
                if (thisOb->HeadComp == thisComp) thisOb->HeadComp = newComp;

                if (newComp->Flags & RINGTRIGGER)
		{
                    thisOb->AnX += thisOb->RingXTrans;
                    thisOb->AnY += thisOb->RingYTrans;
		}

                thisComp = newComp;
	    }

            if (thisComp->AnimCRoutine)
                (*thisComp->AnimCRoutine)(thisComp);

            thisVSprite = thisComp->AnimBob->BobVSprite;
            thisVSprite->X = 
                (thisOb->AnX + thisComp->XTrans + ANIMHALF) >> ANFRACSIZE;
            thisVSprite->Y = 
                (thisOb->AnY + thisComp->YTrans + ANIMHALF) >> ANFRACSIZE;

	}
    }
}



/****** graphics.library/GetGBuffers *******************************************
*
*   NAME
*	GetGBuffers -- Attempt to allocate ALL buffers of an entire AnimOb.
*
*   SYNOPSIS
*	status = GetGBuffers(anOb, rp, db)
*	D0                   A0    A1  D0
*
*	BOOL GetGBuffers(struct AnimOb *, struct RastPort *, BOOL);
*
*   FUNCTION
*	For each sequence of each component of the AnimOb, allocate memory for:
*	    SaveBuffer
*	    BorderLine
*	    CollMask and ImageShadow (point to same buffer)
*	    if db is set TRUE (user wants double-buffering) allocate:
*	        DBufPacket
*	        BufBuffer
*
*   INPUTS
*	anOb = pointer to the AnimOb structure
*	rp   = pointer to the current RastPort
*	db   = double-buffer indicator (set TRUE for double-buffering)
*
*   RESULT
*	status = TRUE if the memory allocations were all successful, else FALSE
*
*   BUGS
*	If any of the memory allocations fail it does not free the partial
*	allocations that did succeed.
*
*   SEE ALSO
*	FreeGBuffers() graphics/gels.h
*
******************************************************************************/
BOOL __asm GetGBuffers(register __a0 struct AnimOb *anOb,
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
            pbufsize = umuls(RPort->BitMap->Depth, ibufsize);

            if ((bptr->SaveBuffer = (void *)AllocMem(pbufsize,
		    MEMF_PUBLIC | MEMF_CHIP)) == 0)
	    {
                return(FALSE);
	    }

            if (dbuf) /* is this a double-buffer? */
	    {
                if ((bptr->DBuffer = (struct DBufPacket *)
			AllocMem(sizeof(struct DBufPacket),MEMF_PUBLIC)) == 0)
		{
                    return(FALSE);
		}
                if ((bptr->DBuffer->BufBuffer = (void *)AllocMem(pbufsize,
			MEMF_PUBLIC | MEMF_CHIP)) == 0)
		{
                    return(FALSE);
		}
	    }

            /* allocate the shadow/coll/BorderLine mask buffers */

            if ((sptr->BorderLine = (void *)AllocMem(sw, 
			MEMF_PUBLIC | MEMF_CHIP)) == 0)
	    {
                return(FALSE);
	    }

            if ((sptr->CollMask = sptr->VSBob->ImageShadow = (void *)AllocMem(ibufsize, MEMF_PUBLIC | MEMF_CHIP)) == 0)
	    {
                return(FALSE);
	    }
            
	    seqptr = seqptr->NextSeq;
	}
        while ((seqptr != acptr) && (seqptr));
    }

    return(TRUE);
}

/****** graphics.library/InitGMasks *******************************************
*
*   NAME
*	InitGMasks -- Initialize all of the masks of an AnimOb.
*
*   SYNOPSIS
*	InitGMasks(anOb)
*	           A0
*
*	void InitGMasks(struct AnimOb *);
*
*   FUNCTION
*	For every sequence of every component call InitMasks.
*
*   INPUTS
*	anOb = pointer to the AnimOb
*
*   BUGS
*
*   SEE ALSO
*	InitMasks() graphics/gels.h
*
*****************************************************************************/
void __asm InitGMasks(register __a0 struct AnimOb *anOb)
{
    struct AnimComp *acptr;
    struct AnimComp *seqptr;
    
    for(acptr = anOb->HeadComp; acptr; acptr = acptr->NextComp)
    {
        seqptr = acptr;
        do
	{
            InitMasks(seqptr->AnimBob->BobVSprite);
            seqptr = seqptr->NextSeq;
	}
        while ((seqptr != acptr) && (seqptr));
    }
}



