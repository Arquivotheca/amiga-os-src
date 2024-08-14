*******************************************************************************
*
*	Source Control
*	--------------
*	$Header: hddisk.i,v 34.14 88/04/02 13:48:40 bart Exp $
*
*	$Locker: bart $
*
*	$Log:	hddisk.i,v $
*   Revision 34.14  88/04/02  13:48:40  bart
*   overscan support
*   
*   Revision 34.13  88/02/19  09:10:09  bart
*   scsi direct io standardisation
*   
*   Revision 34.12  88/02/01  10:24:32  bart
*   reserved commands 0-9 prior to scsi direct command
*   
*   Revision 34.11  87/12/04  19:14:08  bart
*   checkpoint
*   
*   Revision 34.10  87/12/04  12:08:38  bart
*   checkpoint before adding check for existing dosname on eb_mountlist
*   
*   Revision 34.9  87/10/14  14:38:05  bart
*   10-13 rev 1
*   
*   Revision 34.8  87/10/14  14:16:07  bart
*   beginning update to cbm-source.10.13.87
*   
*   Revision 34.7  87/07/08  14:14:50  bart
*   changed name of device to "hddisk" device
*   
*   Revision 34.6  87/07/08  14:01:06  bart
*   y
*   
*   Revision 34.5  87/06/11  15:48:12  bart
*   working autoboot 06.11.87 bart
*   
*   Revision 34.4  87/06/03  10:59:12  bart
*   checkpoint
*   
*   Revision 34.3  87/05/31  16:35:57  bart
*   chickpoint
*   
*   Revision 34.2  87/05/29  19:39:33  bart
*   checkpoint
*   
*   Revision 34.1  87/05/29  17:52:18  bart
*   *** empty log message ***
*   
*   Revision 34.0  87/05/29  17:39:51  bart
*   added to rcs for updating
*   
*
*******************************************************************************


*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************



	IFND	DEVICES_HDDISK_I
DEVICES_HDDISK_I	SET	1

	IFND	EXEC_IO_I
	INCLUDE	"exec/io.i"
	ENDC	!EXEC_IO_I

*--------------------------------------------------------------------
*
* Physical drive constants
*
*--------------------------------------------------------------------


NUMCYLS		EQU	612			; normal # of cylinders
NUMSECS		EQU	17
NUMHEADS	EQU	4
MAXRETRY	EQU	255
MAXMAXRETRY	EQU	512
MAXMAXTRY   EQU 60
NUMTRACKS	EQU	NUMCYLS*NUMHEADS
NUMUNITS	EQU	2

*--------------------------------------------------------------------
*
* Useful constants
*
*--------------------------------------------------------------------


HD_SECTOR	EQU	512
HD_SECSHIFT	EQU	9			; log HD_SECTOR
*						;    2


*--------------------------------------------------------------------
*
* Driver Specific Commands
*
*--------------------------------------------------------------------

*-- HD_NAME is a generic macro to get the name of the driver.  This
*-- way if the name is ever changed you will pick up the change
*-- automatically.
*--
*-- Normal usage would be:
*--
*-- internalName:	HD_NAME
*--

HD_NAME:	MACRO
		CNOP	0,4		; Make sure string is long word aligned
		DC.B	'hddisk.device',0
		DS.W	0
		ENDM

	BITDEF	HD,EXTCOM,15

	DEVINIT
	DEVCMD	HD_MOTOR			; control the disk's motor
	DEVCMD	HD_SEEK				; explicit seek (for testing)
	DEVCMD	HD_FORMAT			; format disk
	DEVCMD	HD_REMOVE			; notify when disk changes
	DEVCMD	HD_CHANGENUM		; number of disk changes
	DEVCMD	HD_CHANGESTATE		; is there a disk in the drive?
	DEVCMD	HD_PROTSTATUS		; is the disk write protected?
	DEVCMD	HD_SPECIAL			; Direct command to controller
	DEVCMD	HD_MOV_CMD_BLK		; Move the cmd. blk. to the dev. struct.
	DEVCMD	HD_RSVD_0			; reserved
	DEVCMD	HD_RSVD_1			; reserved
	DEVCMD	HD_RSVD_2			; reserved
	DEVCMD	HD_RSVD_3			; reserved
	DEVCMD	HD_RSVD_4			; reserved
	DEVCMD	HD_RSVD_5			; reserved
	DEVCMD	HD_RSVD_6			; reserved
	DEVCMD	HD_RSVD_7			; reserved
	DEVCMD	HD_RSVD_8			; reserved
	DEVCMD	HD_RSVD_9			; reserved
	DEVCMD  HD_SCSI         	; Direct SCSI command
	
HD_LASTCOMM	EQU	HD_SCSI

*
*
* The disk driver has an "extended command" facility.  These commands
* take a superset of the normal IO Request block.
*
EHD_WRITE	EQU	(CMD_WRITE!HDF_EXTCOM)
EHD_READ	EQU	(CMD_READ!HDF_EXTCOM)
EHD_MOTOR	EQU	(HD_MOTOR!HDF_EXTCOM)
EHD_SEEK	EQU	(HD_SEEK!HDF_EXTCOM)
EHD_FORMAT	EQU	(HD_FORMAT!HDF_EXTCOM)
EHD_UPDATE	EQU	(CMD_UPDATE!HDF_EXTCOM)
EHD_CLEAR	EQU	(CMD_CLEAR!HDF_EXTCOM)


*
* extended IO has a larger than normal io request block.
*

 STRUCTURE IOEXTHD,IOSTD_SIZE
	ULONG	IOHD_COUNT	; removal/insertion count
	ULONG	IOHD_SECLABEL	; sector label data region
	LABEL	IOHD_SIZE

* labels are HD_LABELSIZE bytes per sector

    STRUCTURE	SCSIB,0			; Direct SCSI command structure
* WARNING!! SCSIB_DATA_BUFFER MUST be EVEN!!
	APTR	SCSIB_DATA_BUFFER	; Points to data to send/receive
* SCSIB_DATA_LENGTH is the length in BYTES! (may not work if odd)
*	Only low order 3 bytes are used
	ULONG	SCSIB_DATA_LENGTH	; Amount of data to transfer
	ULONG	SCSIB_DATA_ACTUAL	; Amount of data actually transferred
	APTR	SCSIB_CMD		; Points to SCSI command/parameter list
	UWORD	SCSIB_CMD_LENGTH	; # of bytes in command to transfer
	UWORD	SCSIB_CMD_ACTUAL	; # of bytes actually transferred as command
	UBYTE	SCSIB_FLAGS			; bit 0 clear means DMA TO SCSI device
								; bit 0 set means DMA FROM SCSI device
	UBYTE	SCSIB_STATUS		; Status returned by SCSI device
					;  if IO_STATUS is HD_NO_ERROR.
*					; UBYTE	SCSIB_CTLR_STATUS	; Controller status -- removed
					; $80 = GOOD
					; $32 = DMA error
					; $F1 = Illegal SCSI bus phase
					; $F2 = Unexpected SCSI bus phase
					; $F3 = Transfer restarted AFTER unexpected SCSI bus phase
					; $F4 = Message-in phase paused with ACK asserted
					; $FD = Parity Error
					; $FE = Select timed out
	LABEL	SCSIB_SIZE

*--------------------------------------------------------------------
*
* Driver error defines
*
*--------------------------------------------------------------------

HD_NO_ERROR     	EQU 0
HDERR_NotSpecified	EQU	20
HDERR_NoSecHdr		EQU	21
HDERR_BadSecPreamble	EQU	22
HDERR_BadSecID		EQU	23
HDERR_BadHdrSum		EQU	24
HDERR_BadSecSum		EQU	25
HDERR_TooFewSecs	EQU	26
HDERR_BadSecHdr		EQU	27
HDERR_WriteProt		EQU	28
HDERR_DiskChanged	EQU	29
HDERR_SeekError		EQU	30
HDERR_NoMem			EQU	31
HDERR_BadUnitNum	EQU	32
HDERR_BadDriveType	EQU	33
HDERR_DriveInUse	EQU	34

SDERR_NotSpecified	EQU	20
SDERR_SelfUnit		EQU	40	; cannot issue SCSI command to self
SDERR_DMA			EQU	41	; DMA error
SDERR_Phase			EQU	42	; illegal or unexpected SCSI phase
SDERR_Parity		EQU 43	; SCSI parity error
SDERR_SelTimeout	EQU	44	; Select timed out

	ENDC	DEVICE_HDDISK_I
