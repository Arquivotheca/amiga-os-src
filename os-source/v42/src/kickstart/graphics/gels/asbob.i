*******************************************************************************
*
*	Source Control
*	--------------
*	$Id: asbob.i,v 42.0 93/06/16 11:19:32 chrisg Exp $
*
*	$Locker:  $
*
*	$Log:	asbob.i,v $
* Revision 42.0  93/06/16  11:19:32  chrisg
* initial
* 
* Revision 42.0  1993/06/01  07:21:36  chrisg
* *** empty log message ***
*
*   Revision 39.0  91/08/21  17:33:11  chrisg
*   Bumped
*   
*   Revision 37.0  91/01/07  15:32:41  spence
*   initial switchover from V36
*   
*   Revision 33.3  90/07/27  16:35:32  bart
*   id
*   
*   Revision 33.2  90/03/28  09:27:34  bart
*   *** empty log message ***
*   
*   Revision 33.1  88/11/16  09:16:29  bart
*   *** empty log message ***
*   
*   Revision 33.0  86/05/17  15:23:27  bart
*   added to rcs for updating
*   
*
*******************************************************************************


* ***************************************************************************
*																			*
* GELS HEADER FILE															*
*	ASSEMBLY-LANGUAGE DEFINITIONS											*
*																			*
* AMIGA CORPORATION  COPYRIGHT 1984  CONFIDENTIAL PROPRIETARY INFORMATION	*
*																			*
* ***************************************************************************

*
* these are the define	s for struct blissObj
*

NEXTBLIT	equ		0
BLITROUTINE	equ		NEXTBLIT+4
BLITSTAT	equ		BLITROUTINE+4
BLITCNT		equ		BLITSTAT+1
BLITSIZE	equ		BLITCNT+1
BEAMSYNC	equ		BLITSIZE+2
CLEANUP		equ		BEAMSYNC+2
* thats all for the system-required memory locations
PPICK		equ		CLEANUP+4
PONOFF		equ		PPICK+2
BLISSINDEX	equ		PONOFF+2
SRCINDEX	equ		BLISSINDEX+1
FWM		equ		SRCINDEX+1
LWM		equ		FWM+2
PBCN0		equ		LWM+2
MINTERM		equ		PBCN0+2
BCN1		equ		MINTERM+2
BMDSRC		equ		BCN1+2
BMDDST		equ		BMDSRC+2
BLITTYPE	equ		BMDDST+2
WRITEMASK	equ		BLITTYPE+2
DUMMY		equ		WRITEMASK+1
ASRC		equ		DUMMY+1
SRCPTR		equ		ASRC+4
DESTPTR		equ		SRCPTR+32
SHADOW		equ		DESTPTR+32
SHADOWSIZE	equ		SHADOW+4
WHOSENTME	equ		SHADOWSIZE+4	* bart - 04.03.86 was +2
NEXT		equ		WHOSENTME+4
BLITSIZV	equ		NEXT+4	
BLITSIZH	equ		BLITSIZV+2


*
* flag bits for bobFlags
*
SAVEBOB	equ	$01
BWAITING	equ	$02
BDRAWN	equ		$04
BOBISCOMP	equ	$08
BOBSAWAY	equ	$10

*
* flag bits for sprFlags
*
IS_SPRITE	equ	$0001	/* set if sprite, clear if bob */
PRSRV_RAS	equ	$0002	/* set if raster is to be saved */
OVERLAY	equ	$0004	/* set to mask image of bob onto background */
MUSTDRAW	equ	$0008	/* set if virtSprite must be drawn */
BOB_PRSRVD	equ	$0100	/* copied by DrawSList, cleared by ClrBob */
BOB_UPDATE	equ	$0200	/* set by DrawBob */
GEL_GONE	equ	$0400	/* set if gel is completely clipped (offscreen) */
OVERFLOW	equ	$0800	/* set on virtual sprite overflow */

*
* bob-blit types
*
B2NORM equ 0
B2SWAP equ 1
B2BOBBER equ 2

*
* some minterms for various types of blit operations
*
FILLSHADOW	equ	ABC+ABNC+ANBC+ANBNC+NABC+NANBC+SRCA+SRCC+DEST
CLEARSHADOW	equ	NABC+NANBC+SRCA+SRCC+DEST
BLOCKCUT	equ	ABC+ABNC+NABC+NANBC+SRCB+SRCC+DEST
SHADOWCUT	equ	ABC+ABNC+NABC+NANBC+SRCA+SRCB+SRCC+DEST

* will need this when down-coding blisser
*  rastoff(r) ((umuls(r->rY , r->rRealWW)) + (r->rX >> 4))
