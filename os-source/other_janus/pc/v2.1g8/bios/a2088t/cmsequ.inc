;************************************************************************
;*	Copyright (c) 1985,1986,1987,1988 Phoenix Technologies Ltd.	*
;*	This program contains proprietary and confidential information.	*
;*	All rights reserved except as may be permitted by prior		*
;*	written consent.						*
;************************************************************************


;************************************************************************
;	Revision Information	$Revision:   3.17  $	
;				$Date:   23 Oct 1989 13:03:40  $
;************************************************************************

	IFDEF NNNNEW
		IF1
			%OUT	.		Including CMSEQU.INC
		ENDIF	;IF1
	ENDIF	;NNNNEW

;************************************************************************
;*	I/O address of RTC/CMOS RAM					*
;************************************************************************

CMOSAD	EQU	70h		; CMOS RAM Address register port (write only)
CMOSDT	EQU	CMOSAD+01h	; CMOS Data port (read/write)


;************************************************************************
;*	CMOS RAM Address Definitions					*
;************************************************************************

CMSEC	EQU	00h		; CMOS BCD Seconds
CMSALM	EQU	01h		; CMOS BCD Seconds Alarm
CMMIN	EQU	02h		; CMOS BCD Minutes
CMMALM	EQU	03h		; CMOS BCD Minutes Alarm
CMHOUR	EQU	04h		; CMOS BCD Hour
CMHALM	EQU	05h		; CMOS BCD Hour Alarm
CMDAY	EQU	06h		; CMOS BCD Day of week
CMDATE	EQU	07h		; CMOS BCD Date
CMMON	EQU	08h		; CMOS BCD Month
CMYEAR	EQU	09h		; CMOS BCD Year
CMAST	EQU	0Ah		; CMOS Register A Status
CMBST	EQU	0Bh		; CMOS Register B Status
CMCST	EQU	0Ch		; CMOS Register C Status
CMDST	EQU	0Dh		; CMOS Register D Status
CMSTAT	EQU	0Eh		; CMOS Diagnostic status
CMSHUT	EQU	0Fh		; CMOS Shutdown reason
CMFDSK	EQU	10h		; CMOS Floppy disk types,
CM10REC	RECORD	FLPA_10:4, FLPB_10:4
;
;	EQU	11h		; CMOS Reserved
CMHDSK	EQU	12h		; CMOS Hard disk types
				;   Drive C upper nibble, Drive D lower nibble
CMPASSWD EQU	13h		; CMOS power-on password type bits
CM13REC	RECORD	CM13_TBD:6, NETW_PWD:1, POWON_PWD:1
CMEQPT	EQU	14h		; CMOS Equipment installed
CM13REC	RECORD	CM14_TBDHI:1, TWOFLP_14:1, DSPLAY_14:2, CM14_TBDLO:1, NDP_14:1, BOOTFLP_14: 1
CMBML	EQU	15h		; CMOS Base Memory in k (low byte)
CMBMH	EQU	16h		; CMOS Base Memory in k (high byte)
CMEML	EQU	17h		; CMOS Expanded Memory in k (low byte)
CMEMH	EQU	18h		; CMOS Expanded Memory in k (high byte)
CMHDXC	EQU	19h		; CMOS Hard Disk Extended Type for C:
CMHDXD	EQU	1Ah		; CMOS Hard Disk Extended Type for D:
;
;	Configuration Enable Byte definition (1Fh)
;
CMCFGEN	EQU	1Fh		; CMOS Configuration Enables in lower 3 bits
CM1FREC RECORD  CM1F_TBD:3, SHVID_1F:1, SHBIOS_1F:1, FLP_1F:1, CRT_1F:1, KYB_1F:1
SH_VIDEO EQU 	MASK SHVID_1F
SH_BIOS	EQU	MASK SHBIOS_1F
KYBEN	EQU	MASK KYB_1F
CRTEN	EQU	MASK CRT_1F
FLPEN	EQU	MASK FLP_1F
;
       IFDEF	NNNTYPE49
	IF	HDCM48
CMHDP1	EQU	20h		; cmos address for hard disk param. type 48
;	thru	27h		; 8 bytes of HDPARM, the other eight default
	ENDIF	; HDCM48
       ELSE	; NNNTYPE49		
CMHDP1	EQU	20h		; cmos address for hard disk param. type 48
;	thru	27h		; 8 bytes of HDPARM, the other eight default
       ENDIF	; NNNTYPE49	
; 28h thru 2Dh	; undefined
CMCKSH	EQU	2Eh		; CMOS Checksum of address 10h - 2Dh (high byte)
CMCKSL	EQU	2Fh		; CMOS Checksum of address 10h - 2Dh (low byte)
CMEMAL	EQU	30h		; CMOS Actual Expanded Memory in k (low byte)
CMEMAH	EQU	31h		; CMOS Actual Expanded Memory in k (high byte)
CMCENT	EQU	32h		; CMOS Century
;
;	Information Flag Byte definition (33h)
;
CMINFO	EQU	33h			; CMOS Setup information
EXP128	EQU	80h			; 128k expansion installed
SETUPF	EQU	40h			; Setup flag, SETUP program puts out a 
					;   1st user message after intial setup
;
;	CMOS Speed Byte definition (34h)
;
CMSPD	EQU	34h			; CMOS Speed Information in lower 3 bits
CMBTFL	EQU	34h			; Boot failed byte
CMCACHE_GD EQU	34h			; Cache Good byte
;
;
CM34REC	RECORD	CM34_TBD:2, LSTBTFL_34:1, BTFAIL_34:1, CACHE_34:1, SPD_34:3
LAST_BFL EQU	MASK LSTBTFL_34		; indicates last boot failed bit
BTFL_BIT EQU	MASK BTFAIL_34		; indicates this boot failed bit
CACHE_GD EQU	MASK CACHE_34		; flag indicating the cache is good
;
;
;
       IFDEF	NNNTYPE49
	IF	HDCM49
CMHDP2	EQU	35h		; cmos address for hard disk param. type 49
;	thru	3Ch		; 8 bytes of HDPARM, the other eight default
	ENDIF	; HDCM49
       ELSE	; NNNTYPE49
CMHDP2	EQU	35h		; cmos address for hard disk param. type 49
;	thru	3Ch		; 8 bytes of HDPARM, the other eight default
       ENDIF	; NNNTYPE49
;
CMDIAG	EQU	35h		; holds a copy of the DIAGNP
CMMANLO	EQU	36h		; Lo byte of number of Man Loops
CMMANHI	EQU	37h		; Hi byte of number of Man Loops
;
CMPOPW	EQU	38h		; Power-on password (conflicts with type 49)
;	thru	3Eh		; 7 bytes, null terminated only if shorter
CMPOPWCHK EQU	3Fh		; Bytesum of bytes 38h - 3Eh
;

;
;
;************************************************************************
;*	Extended CMOS RAM Addresses					*
;************************************************************************
;
;	CMCHIP Byte definition (40h)
;
CMCHIP		EQU	40H
CRAM_WAIT	EQU	40h
CROM_WAIT	EQU	40h
CMEM_RELOCATION	EQU	40h
;
CMSHCD		EQU	41h		;shadow enable for C000 to DFFF
CMSHEF		EQU	42h		;shadow enable for E000 to EFFF
CEMS_BASEMEM	EQU	43h
CEMS_IO		EQU	43h
CEMS_EXT	EQU	44h
CEMS_WAIT	EQU	45h
CEMS_SIZE	EQU	45h
DYN_SIG		EQU	4Eh
;
;
;
;
;************************************************************************
;*	Bit Definitions for CMOS RAM Addresses				*
;************************************************************************

;	Status Register A Bit definitions (0Ah)

SRAUIP	EQU	80h		; Time update in progress
SRADVM	EQU	70h		; 22-Stage Divider mask
SRAD32	EQU	20h		;   selects time-base frequency of 32.768 kHz
SRARSM	EQU	0Fh		; Rate selection mask
SRAR10	EQU	06h		; Rate selection of 1.024 kHz,
				;   period 976.562 microsecond interrupt rate
;
;	Status Register B Bit definitions (0Bh)
;
SRBRST	EQU	02h		; Value to be reset to during POST	
SRBSET	EQU	80h		; Set time and abort any update-in-progress
SRBPIE	EQU	40h		; Periodic interrupt enable
SRBAIE	EQU	20h		; Alarm interrupt enable
SRBUIE	EQU	10h		; Update-ended interrupt enable
SRBSQE	EQU	08h		; Square Wave Enabled (SQWE)
SRBDM	EQU	04h		; Date mode is in binary, else BCD
SRB24H	EQU	02h		; Hours byte is in 24 hour mode, else 12 hour
SRBDSE	EQU	01h		; Daylight Savings Enabled
;
;	Status Register C Bit definitions (0Ch)
;
SRCIRQ	EQU	80h		; Interrupt Request flag (read only)
SRCPF	EQU	40h		; Periodic interrupt flag (read only)
SRCAF	EQU	20h		; Alarm interrupt flag (read only)
SRCUF	EQU	10h		; Update-ended interrupt flag (read only)
;
;	Status Register D Bit definitions (0Dh)
;
CMSVRM	EQU	80h		; CMOS Valid RAM bit
;
;	Diagnostic Status Byte Bit definitions (0Eh)
;
LSTPWR	EQU	80h		; CMOS CHIP lost power
CMCKSM	EQU	40h		; CMOS Checksum is bad
IVLCFG	EQU	20h		; CMOS Invalid configuration information found
				;   at POST
MEMDIF	EQU	10h		; CMOS Memory size compare error at POST
HDSKIF	EQU	08h		; CMOS Hard Disk Initialization failure
INVTIM	EQU	04h		; CMOS Invalid time found
;	EQU	02h		; CMOS Not used
;	EQU	01h		; CMOS Not used
;
;	Shutdown Status Byte Bit definitions (0Fh)
;
;
;	Floppy Disk Type Byte Bit definitions (10h)
;
FDNONE	EQU	00h		; No floppy disk
FD48D	EQU	01h		; Floppy Disk, 48 TPI, Double Sided
FD96D	EQU	01h		; Floppy Disk, 96 TPI, Double Sided
;
;	Hard Disk Type Byte Bit definitions (12h)
;
HDNONE	EQU	00h		; No Hard disk
;
;	Equipment Byte Bit definitions (14h)
;
FDSK1	EQU	(1-1) SHL 6	; 1 Floppy disk drive
FDSK2	EQU	(2-1) SHL 6	; 2 Floppy disk drives
CRTMON	EQU	30h		; Monochrome primary display
CRTC80	EQU	20h		; Color, 80 column primary display
CRTC40	EQU	10h		; Color, 40 column primary display
CRTNON	EQU	00h		; No primary display
INS287	EQU	02h		; 80287 installed
FDPRES	EQU	01h		; Floppy disks present

;************************************************************************
;*	End of Include File						*
;************************************************************************

.CREF
.LIST
