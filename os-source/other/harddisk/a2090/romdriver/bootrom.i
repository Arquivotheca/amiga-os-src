*******************************************************************************
*
*	Source Control
*	--------------
*	$Header: bootrom.i,v 34.21 87/12/04 19:13:33 bart Exp $
*
*	$Locker:  $
*
*	$Log:	bootrom.i,v $
*   Revision 34.21  87/12/04  19:13:33  bart
*   checkpoint
*   
*   Revision 34.20  87/12/04  12:08:05  bart
*   checkpoint before adding check for existing dosname on eb_mountlist
*   
*   Revision 34.19  87/10/15  09:34:19  bart
*   10-13 rev 1
*   
*   Revision 34.18  87/10/14  14:15:36  bart
*   beginning update to cbm-source.10.13.87
*   
*   Revision 34.17  87/07/08  14:00:41  bart
*   y
*   
*   Revision 34.16  87/06/12  10:55:30  bart
*   ok, so init needs the definition of BootNode from romboot_base after all
*   
*   Revision 34.15  87/06/12  10:50:59  bart
*   removed references to romboot_base... not necessary anymore!
*   
*   Revision 34.14  87/06/11  15:47:30  bart
*   working autoboot 06.11.87 bart
*   
*   Revision 34.13  87/06/10  12:45:23  bart
*   *** empty log message ***
*   
*   Revision 34.12  87/06/10  11:24:16  bart
*   *** empty log message ***
*   
*   Revision 34.11  87/06/03  13:06:47  bart
*   AutoBoot
*   
*   Revision 34.10  87/06/03  10:58:45  bart
*   checkpoint
*   
*   Revision 34.9  87/05/31  18:22:21  bart
*   INCLUDE "libraries/configvars.i"
*   
*   Revision 34.8  87/05/31  16:35:26  bart
*   chickpoint
*   
*   Revision 34.7  87/05/29  19:39:03  bart
*   checkpoint
*   
*   Revision 34.6  87/05/28  19:56:35  bart
*   *** empty log message ***
*   
*   Revision 34.5  87/05/28  18:37:05  bart
*   checkpoint
*   
*   Revision 34.4  87/05/28  16:19:18  bart
*   *** empty log message ***
*   
*   Revision 34.3  87/05/28  16:17:20  bart
*   *** empty log message ***
*   
*   Revision 34.2  87/05/28  16:16:40  bart
*   *** empty log message ***
*   
*   Revision 34.1  87/05/28  15:41:47  bart
*   *** empty log message ***
*   
*   Revision 34.0  87/05/28  15:40:20  bart
*   added to rcs for updating
*   
*
*******************************************************************************


	INCLUDE	"exec/types.i"
	INCLUDE	"exec/resident.i"
	INCLUDE	"libraries/expansionbase.i"
	INCLUDE	"libraries/configregs.i"
	INCLUDE	"libraries/configvars.i"
	INCLUDE	"libraries/dos.i"
	INCLUDE	"libraries/romboot_base.i"

NULL	EQU	0

BOOT_NAME:	MACRO
	DC.B	'romboot.device',0
	DS.W	0
	ENDM
