
*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************


*************************************************************************
*
* hddisk.i
*
* Source Control
* ------ -------
* 
* $Header: hddisk.i,v 27.2 85/07/12 23:16:27 neil Exp $
*
* $Locker:  $
*
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
MAXRETRY	EQU	100
MAXMAXTRY	EQU	30
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
	DEVCMD	HD_MOTOR		; control the disk's motor
	DEVCMD	HD_SEEK			; explicit seek (for testing)
	DEVCMD	HD_FORMAT		; format disk
	DEVCMD	HD_REMOVE		; notify when disk changes
	DEVCMD	HD_CHANGENUM		; number of disk changes
	DEVCMD	HD_CHANGESTATE		; is there a disk in the drive?
	DEVCMD	HD_PROTSTATUS		; is the disk write protected?
	DEVCMD	HD_SPECIAL		; Direct command to controller
	DEVCMD	HD_MOV_CMD_BLK		; Move the cmd. blk. to the dev. struct.
	DEVCMD	HD_SCSI			; Direct SCSI command
	
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

HD_LABELSIZE	EQU	16

    STRUCTURE	SCSIB,0			; Direct SCSI command structure
* WARNING!! SCSIB_DATA_BUFFER MUST be EVEN!!
	APTR	SCSIB_DATA_BUFFER	; Points to data to send/receive
* SCSIB_DATA_LENGTH is the length in BYTES! (may not work if odd)
*	Only low order 3 bytes are used
	ULONG	SCSIB_DATA_LENGTH	; Amount of data to transfer
	APTR	SCSIB_CMD		; Points to SCSI command/parameter list
	UWORD	SCSIB_CMD_LENGTH	; # of bytes in actual command
	UBYTE	SCSIB_DIRECTION		; 0 means DMA TO SCSI device
					; 1 means DMA FROM SCSI device
	UBYTE	SCSIB_STATUS		; Status returned by SCSI device
					;  if IO_STATUS is HD_NO_ERROR.
	UBYTE	SCSIB_CTLR_STATUS	; Controller status
					; $80 = GOOD
					; $32 = DMA error
					; $F1 = Illegal SCSI bus phase
					; $F2 = Unexpected SCSI bus phase
					; $F3
					; $F4
					; $FD = Parity Error
					; $FE = Select timed out
	LABEL	SCSIB_SIZE

*--------------------------------------------------------------------
*
* Driver error defines
*
*--------------------------------------------------------------------

HD_NO_ERROR		EQU	0
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
HDERR_NoMem		EQU	31
HDERR_BadUnitNum	EQU	32
HDERR_BadDriveType	EQU	33
HDERR_DriveInUse	EQU	34

	ENDC	DEVICE_HDDISK_I
