*******************************************************************************
*
*	Source Control
*	--------------
*	$Id: sane_names.i,v 39.0 91/08/21 17:12:41 chrisg Exp $
*
*	$Locker:  $
*
*	$Log:	sane_names.i,v $
*   Revision 39.0  91/08/21  17:12:41  chrisg
*   Bumped
*   
*   Revision 37.0  91/01/07  15:15:12  spence
*   initial switchover from V36
*   
*   Revision 33.4  90/07/27  16:32:10  bart
*   *** empty log message ***
*   
*   Revision 33.3  90/03/28  09:40:07  bart
*   *** empty log message ***
*   
*   Revision 33.2  89/05/02  09:33:23  bart
*   copyright 1989
*   
*   Revision 33.1  88/06/23  18:39:48  dale
*   cp
*   
*   Revision 33.0  86/05/17  14:57:57  bart
*   added to rcs for updating
*   
*
*******************************************************************************


	ifd	HARDWARE_CUSTOM_I
* atleast I can pronounce these
cop2ptr	equ cop2lc
fwmask	equ bltafwm
lwmask	equ bltalwm
bltpta	equ bltapt
bltptb	equ bltbpt
bltptc	equ bltcpt
bltptd	equ bltdpt
bltmda	equ bltamod
bltmdb	equ bltbmod
bltmdc	equ bltcmod
bltmdd	equ bltdmod
adata	equ bltadat
bdata	equ bltbdat
cdata	equ bltcdat
	endc
*	include 'hardware/intbits.i'
	ifd	HARDWARE_INTBITS_I
INTF_MASTER	equ INTF_INTEN	* real brain damage here
	ENDC