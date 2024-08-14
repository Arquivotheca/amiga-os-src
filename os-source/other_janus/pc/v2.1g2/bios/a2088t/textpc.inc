	IFNDEF LISTINC
.XLIST
.XCREF
	ENDIF
;	NAME	TEXTPC.INC
;	TITLE	Displayed Text Messages for the 80286 BIOS
;***************************************************************************
;* Copyright (c) 1986-1989 Phoenix Technologies Ltd.  This program	   *
;* contains proprietary and confidential information.  All rights reserved *
;* except as may be permitted by prior written consent.			   *
;***************************************************************************


;***************************************************************************
;	Revision Information	$Revision:   3.28  $	
;				$Date:   05 Dec 1989 10:29:28  $
;***************************************************************************

	IFDEF NNNNEW
		IF1
			%OUT	.		Including TEXTAT.INC
		ENDIF	;IF1
	ENDIF	;NNNNEW

;	This file contains all displayed messages for the 80286 BIOS.  These
;	texts are gathered here to facilitate their translation.  The first
;	3 macros (containing the displayed copyright message) may not be
;	altered or translated without the written consent of Phoenix
;	Technologies, Ltd.  The "0" characters terminating most texts is
;	required and should be left alone.


;***************************************************************************
;*	Sign-On Text Messages (also see MANSION2 in ROMOPT.INC)

   IFDEF  NNN_BIOS400
TM_SINO_START   MACRO
        DB      'PhoenixBIOS ',0
        ENDM
   ENDIF ;NNN_BIOS400


TM_SINO	MACRO		; signon message for AT-compatibles
        IFDEF  NNN_BIOS400

	IFDEF	PPP80386
	DB	'80386-A Version'
	ENDIF	;PPP80386
        ELSE  ;NNN_BIOS400


	IFDEF	PPP80386
	DB	'80386 '
	ENDIF	;PPP80386
        ENDIF  ;NNN_BIOS400

        IFNDEF NNN_BIOS400
        DB	'ROM BIOS PLUS Version '
   ENDIF ;NNN_BIOS400


	DB	VERSINO/100h+'0','.'
	GENASC	%(LOW VERSINO)
      IFDEF	NNNNEW
       IF	USE_REFNO
	DB	CR,LF,0
       ELSE	;USE_REFNO
	DB	'.'
	DB	0
       ENDIF	;USE_REFNO
      ELSE	;NNNNEW		
	DB	'.'
	DB	0
      ENDIF	;NNNNEW	
	ENDM
TM_INHS	MACRO		;internal use ONLY message
      IFDEF	NNNNEW
       IF	USE_REFNO
	DB	'Incremental '
	GENASC	PVER
	DB	'   For internal use ONLY',CR,LF,LF,0
       ELSE	;USE_REFNO
	GENASC	PVER
	DB	'   For internal use ONLY',CR,LF,0
       ENDIF	;USE_REFNO
      ELSE	;NNNNEW		
	GENASC	PVER
	DB	'   For internal use ONLY',CR,LF,0
      ENDIF	;NNNNEW	
	ENDM
TM_CUST	MACRO
      IFDEF	NNNNEW
       IF	USE_REFNO
	DB	'Reference ID '
	GENASC	CUSTVER
	DB	CR,LF,LF,0		
       ELSE	;USE_REFNO
	GENASC	CUSTVER
	DB	CR,LF,0
       ENDIF	;USE_REFNO
      ELSE	;NNNNEW	
	GENASC	CUSTVER
	DB	CR,LF,0
      ENDIF	;NNNNEW	
	ENDM
TM_COPY	MACRO		; copyright message part 1
CPYNOT	DB	'Copyright (C) 1985-1989 '
CPYMSG	DB	'Phoenix Technologies Ltd.',0
	ENDM

IFDEF	NNNCONSTRUCT

TM_ALRR	MACRO		; copyright message part 2
        DB	CR,LF,'All Rights Reserved',CR,LF,LF,0
	ENDM

ELSE	;NNNCONSTRUCT

TM_ALRR	MACRO		; copyright message part 2
        DB	CR,LF,'All Rights Reserved',CR,LF,LF
	ENDM

ENDIF	;NNNCONSTRUCT

;***************************************************************************
;*	Memory Test Text Messages:
    IFDEF NNN_MEM_START

TM_START MACRO
	DB	'Beginning memory Test',CR,LF
	DB	'Press the SPACEBAR to terminate the memory test.'
	DB	CR,LF,0	    
	ENDM
   ENDIF ;NNN_MEM_START	
 
TM_A20F	MACRO
	DB	'Gate A20 failure',CR,LF,0
	ENDM

TM_UXIP	MACRO
	DB	'Unexpected interrupt in protected mode',CR,LF,0
	ENDM

TM_DAME	MACRO
DAMMSG	DB	' Decreasing available memory',CR,LF,0
	ENDM


TM_BAME	MACRO
	DB	'064K Base Memory, ',0
	ENDM
TM_EXME	MACRO
	DB	'00000K Extended',CR,LF,0
	ENDM	 


TM_MTAB	MACRO
	DB	'Memory tests terminated by keystroke'

   IFDEF  NNNEN88121440
	DB	CR,LF
   ENDIF ;NNNEN88121440
	DB	0
	ENDM



TM_MEM1	MACRO		;beginning of memory fail message
	DB	'Memory ',0
	ENDM
  IFNDEF  NNN_FAILED
TM_MEM2	MACRO		;specific type of memory failure
	PUBLIC	M_MDAT
	PUBLIC	M_MWRE
	PUBLIC	M_MOEL
	PUBLIC	M_MADD
M_MPAR	DB 'parity',0			;for this failure, don't report details
M_MDAT	DB 'data line',0		;for this failure, don't report details
M_MOEL	DB 'odd/even logic',0		;for this failure, don't report details
M_MDWL	DB 'double word logic',0	;for this failure, don't report details
M_MHAD	DB 'high '			;for this failure, don't report details
M_MADD	DB 'address line',0
M_MWRE	DB 'write/read',0
	ENDM
  ENDIF  ;NNN_FAILED

TM_MEM3	MACRO		;next part of memory fail message
	DB	' failure at ',0
	ENDM
;now report either address, value read, value expected...
TM_MEM4	MACRO
	DB	', read ',0
	ENDM
TM_MEM5	MACRO
	DB	' expecting ',0
	ENDM
; or else report entire bank which failed
TM_MEM0	MACRO
	DB	'0000-',0
	ENDM
TM_MEMF	MACRO
	DB	'FFFF',CR,LF,0
	ENDM


;***************************************************************************
;*	Other Power-On Self-Test Text Messages:

TM_DADF	MACRO
	DB	'Display adapter failed; using alternate',CR,LF,0
	ENDM

TM_NOTT	MACRO
	DB	'No timer tick interrupt',CR,LF,0
	ENDM

TM_SHUF	MACRO
	DB	'Shutdown failure',CR,LF,0
	ENDM

TM_TC2F	MACRO
	DB	'Timer 2 failure',CR,LF,0
	ENDM

TM_KSTA	MACRO		;start of keyboard failure message
	DB	'Keyboard ',0
	ENDM
TM_KMID	MACRO		;possible middles of keyboard failure message
M_CONT	DB	'controller'
M_NULL	DB	0
M_CLOL	DB	'clock line',0
M_DALI	DB	'data line',0
M_STUK	DB	'stuck key',0
	ENDM
TM_KEND	MACRO		;end of keyboard failure message
	DB	' failure'
KEYMSG	DB CR,LF,0	;labelled because something else uses
	ENDM

TM_DDF1	MACRO
	DB	'Diskette drive failure',CR,LF,0
	ENDM

TM_DDF2	MACRO
	DB	'Diskette drive 0 failure',CR,LF,0
	ENDM

TM_DDF3	MACRO
	DB	'Diskette drive 1 failure',CR,LF,0
	ENDM

TM_DDT1	MACRO
	DB	'Diag: Initialize floppy system',CR,LF,0
	ENDM

TM_DDT2	MACRO
	DB	'Diag: Enable Janus Interface and boot OS',CR,LF,0
	ENDM

TM_DDT3	MACRO
	DB	'Diag: Reset disk system',CR,LF,0
	ENDM

TM_DDT4	MACRO
	DB	'Diag: Test floppy drive',CR,LF,0
	ENDM

TM_DDT5	MACRO
	DB	'Diag: Recalibrating floppy drive failed',CR,LF,0
	ENDM

TM_DDT6	MACRO
	DB	'Diag: Recalibrating floppy drive OK',CR,LF,0
	ENDM

TM_DDT7	MACRO
	DB	'Diag: Disktest found 40 track drive',CR,LF,0
	ENDM

TM_HCFG	MACRO
  IFDEF  NNN_POST_NEW
	DB	'Fixed disk configuration error',CR,LF,0
  ELSE  ;NNN_POST_NEW
	DB	'Hard disk configuration error',CR,LF,0
  ENDIF  ;NNN_POST_NEW
	ENDM

TM_HCON	MACRO
  IFDEF  NNN_POST_NEW
	DB	'Fixed disk controller failure',CR,LF,0
  ELSE  ;NNN_POST_NEW
	DB	'Hard disk controller failure',CR,LF,0
  ENDIF ;NNN_POST_NEW
	ENDM

TM_HDRV	MACRO
	DB	'Hard disk failure',CR,LF,0
	ENDM


TM_TODF	MACRO
   IFDEF  NNN_POST_NEW
	DB	'Real time clock failure',CR,LF,0
   ELSE ;NNN_POST_NEW
	DB	'Time-of-day clock stopped',CR,LF,0
   ENDIF ;NNN_POST_NEW
	ENDM

TM_INVC	MACRO
    IFDEF  NNN_POST_NEW
	DB	'Invalid configuration information - run SETUP program'
     ELSE ;NNN_POST_NEW
	DB	'Invalid configuration information - please run SETUP program'
    ENDIF ;NNN_POST_NEW
	DB	CR,LF,0
	ENDM

TM_TODS	MACRO
   IFDEF  NNN_POST_NEW
	DB	'Time-of-day not set - run SETUP program',CR,LF,0
   ELSE  ;NNN_POST_NEW
	DB	'Time-of-day not set - please run SETUP program',CR,LF,0
   ENDIF ;NNN_POST_NEW
	ENDM

TM_KLOK	MACRO
   IFDEF   NNN_POST_NEW
	DB	'Keyboard is locked - unlock',CR,LF,0
   ELSE  ;NNN_POST_NEW
	DB	'Keyboard is locked - please unlock',CR,LF,0
   ENDIF ;NNN_POST_NEW
	ENDM

TM_ROMC	MACRO
	PUBLIC	ROMMSG
  IFDEF NNN_POST_NEW
ROMMSG	DB	'Option ROM checksum failure ',0
  ELSE  ;NNN_POST_NEW
ROMMSG	DB	'ROM bad checksum = ',0
  ENDIF ;NNN_POST_NEW
	ENDM

TM_ROMO	MACRO
   IFDEF  NNN_POST_NEW
	DB	'BIOS ',0
   ELSE  ;NNN_POST_NEW
	DB	'0h Optional ',0
   ENDIF  ;NNN_POST_NEW
	ENDM


;***************************************************************************
;*	Boot-related Text Messages:


TM_WFF1	MACRO		;printed prior to boot if any POST errors
   IFDEF   NNN_POST_NEW
     DB	' Press the F1 key to continue',CR,LF,0
    ELSE   ;NNN_POST_NEW   
        DB	' Strike the F1 key to continue',CR,LF,0
    ENDIF  ;NNN_POST_NEW
	ENDM



IFDEF	NNNCONSTRUCT
TM_WF12 MACRO		;printed prior to boot if any POST errors
    IFDEF  NNN_POST_NEW
	DB	' Press the F1 key to continue, '
    ELSE  ;NNN_POST_NEW
	DB	' Strike the F1 key to continue, '
    ENDIF ;NNN_POST_NEW
	DB	'F2 to run the setup utility',CR,LF,0
	ENDM							       
 
ELSE	;NNNCONSTRUCT
       IF	ROMSET
TM_WF12 MACRO		;printed prior to boot if any POST errors
    IFDEF  NNN_POST_NEW
	DB	' Press the F1 key to continue, '
    ELSE  ;NNN_POST_NEW
	DB	' Strike the F1 key to continue, '
    ENDIF ;NNN_POST_NEW
	DB	'F2 to run the setup utility',CR,LF,0
	ENDM							       
       ENDIF	;ROMSET
ENDIF	;NNNCONSTRUCT

TM_BOOF	MACRO		;one of these is printed first if boot failure
  IFDEF  NNN_POST_NEW
BTMSG1	DB	CR,LF,'Diskette read failure -',0
BTMSG2	DB	CR,LF,'Not a boot diskette -',0
BTMSG3	DB	CR,LF,'No boot device available -',0
BTMSG4	DB	CR,LF,'Hard disk read failure -',0
BTMSG5	DB	CR,LF,'No boot sector on hard disk -',0
  ELSE ;NNN_POST_NEW
BTMSG1	DB	CR,LF,'Diskette read failure -',0
BTMSG2	DB	CR,LF,'Not a boot diskette -',0
BTMSG3	DB	CR,LF,'No boot device available -',0
BTMSG4	DB	CR,LF,'Hard disk read failure -',0
BTMSG5	DB	CR,LF,'No boot sector on hard disk -',0
   ENDIF ;NNN_POST_NEW
      IFDEF	NNNNEW
       IF	PASSWRD
BTMSG6	DB	CR,LF,'Enter password: ',0
BTMSG7	DB	CR,LF,'Password is incorrect',0
BTMSG8	DB	CR,LF,'Password OK',CR,LF,0
BTMSG9	DB	CR,LF,'Password has been removed',CR,LF,0
BTMSGA	DB	CR,LF,'New password has been installed',CR,LF,0
BTMSGB	DB	CR,LF,'System halted!  Must power down.',CR,LF,0
       ENDIF	;PASSWRD
      ENDIF	;NNNNEW DEF
	ENDM

TM_BOOR	MACRO		;printed within INT 18
   IFDEF  NNN_POST_NEW
        DB	CR,LF,' press F1 to retry boot',CR,LF,0
   ELSE  ;NNN_POST_NEW
        DB	CR,LF,' strike F1 to retry boot',CR,LF,0
   ENDIF  ;NNN_POST_NEW
	ENDM


IFDEF	NNNCONSTRUCT
TM_F1F2 MACRO		;printed within INT 18 			
      IFDEF  NNN_POST_NEW
	DB	CR,LF,' press F1 to retry boot, '				
      ELSE  ;NNN_POST_NEW
	DB	CR,LF,' strike F1 to retry boot, '				
      ENDIF ;NNN_POST_NEW
	DB	'F2 for setup utility',CR,LF,0
	ENDM	      
 
ELSE	;NNNCONSTRUCT
       IF	ROMSET
TM_F1F2 MACRO		;printed within INT 18 			
      IFDEF  NNN_POST_NEW
	DB	CR,LF,' press F1 to retry boot, '				
      ELSE  ;NNN_POST_NEW
	DB	CR,LF,' strike F1 to retry boot, '				
      ENDIF ;NNN_POST_NEW
	DB	'F2 for setup utility',CR,LF,0
	ENDM	      
       ENDIF	;ROMSET
ENDIF	;NNNCONSTRUCT

;***************************************************************************
;*	Drastic-Error-During-Operation Text Messages:

				; Unexpected interrupt reporting:
TM_UXI1	MACRO			; Unexpected int leading text
	DB	CR,LF,'Unexpected ',0
	ENDM
				; Next one of the following is printed:
SWINTFL	EQU	'S'		; SW int flag and first letter of 2nd UXI text
HWINTFL	EQU	'H'		; SW int flag and first letter of 2nd UXI text
TM_UXI2	MACRO			; 2nd unexpected int text
	DB	'W interrupt ',0
	ENDM
				; Next the interrupt number is printed.
TM_UXI3	MACRO			; 3nd unexpected int text
	DB	' at ',0
	ENDM
				; Next the execution address is printed.
REBOKEY	EQU	'R'		; operator key for Reboot
TM_UXIR	MACRO			; 4th unexpected int text
	DB	'.  Type'
	PUBLIC	INTMSG5		; shared w/ NMI reporting
INTMSG5	DB	' (R)eboot, other keys'
CONMSG	DB	' to continue',0
	ENDM

				; Unexpected NMI reporting:
TM_NMI1	MACRO			; First, one of the following is printed:
M_NMII	DB	CR,LF,'I/O card parity',0
M_NMIM	DB	CR,LF,'Memory parity',0

IFDEF	NNNCONSTRUCT
M_NMIU	DB	CR,LF,'Unexpected type 02',0
ELSE	;NNNCONSTRUCT
       IF	INTENHD		; if reporting SW interrupts,
M_NMIU	DB	CR,LF,'Unexpected type 02',0
       ENDIF	;INTENHD
ENDIF	;NNNCONSTRUCT

	ENDM
TM_NMI2	MACRO			; Next unexpected NMI text
	DB	' interrupt at ',0
	ENDM
SHUTKEY	EQU	'S'		; operator key for Shutting off NMI
TM_NMIS	MACRO			; Next unexpected NMI text
	DB	'.',CR,LF
	DB	' Type (S)hut off NMI,',0
	ENDM
				; Finally, INTMSG5 (above) is printed
;***************************************************************************
; Trailing string for hex messages, string for carriage return, line feed
TM_CRLF	MACRO
	PUBLIC	HEXMSG
	PUBLIC	CRLFMSG
HEXMSG	DB	'h'
CRLFMSG	DB	CR,LF,0

	IFDEF	NNNNEW
	PRGSIZA	<CR,LF message>		; Display program size
	ELSE	;NNNNEW
	PRGSIZ	<CR,LF message>		; Display program size
	ENDIF	;NNNNEW
	ENDM


.CREF
.LIST
