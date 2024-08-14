
;***************************************************************************
;* Copyright (c) 1985-1989 Phoenix Technologies Ltd.  This program	   *
;* contains proprietary and confidential information.  All rights reserved *
;* except as may be permitted by prior written consent.			   *
;***************************************************************************
;
;***************************************************************************
;	Revision Information	$Revision:   3.8.1.2  $	
;				$Date:   22 Feb 1990 15:17:26  $
;***************************************************************************
;;
;	RBP 3.10E Bug Fix and enhancements
;
;
;	FOLLOWING WERE ENHANCEMENTS FOR FLOPPY PERFORMANCE ENHANCEMENTS
;
NNNMACRO	EQU	TRUE	; Use macros in line to set carry and
				; clear carry bit. At present they
				; call a subroutine
NNNMOTOR_ON	EQU	TRUE	; INT13 IF NOTOR ALREADY ON NO SENSE
				; TURNING IT ON A SECOND TIME
NNNSEEK_UPDATE	EQU	TRUE	; DOSEEK UPDATE TEST FOR ALREADY THERE
				; BEFORE WE WAIT
NNN_PASS_WAIT	EQU	TRUE	; IN THE ROUTINE WAITINT, AFTER THE WAIT,
				; WOULD WAIT ANOTHER 100 MICROSECOND BEFORE
				; TESTING IF INTERRUPT PRESENT. NOW CHECK
				; BEFORE WAIT. IF NOT PRESENT THEN WAIT AND 
				; AND TEST AGAIN.
NNN_FAST_SEEK	EQU	TRUE	; SOME OLD CODE WOULD SET THE TRANSFER RATE
				; TO 250, THIS EFFECTED THE SEEK STEPPING
				; RATE. IN ALL CASES THE SEEK RATE IS SET FOR
				; 500, THE FASTEST STEP PULSE. BEFORE ATTEMPTING
				; TRANSFER, TRANSFER RATE RE-ISSUED.
NNN_FIXUP	EQU	TRUE	; Several minor changes were made as a result
				; of the code review. All these changes have
				; been included with this label
NNN_POST_NEW	EQU	TRUE	; There were several changes in the POST messages
;NNN_BIOS400     EQU     TRUE    ; Used to create new signon message				
;NNN_ECR_1       EQU     TRUE    ; This ECR identifies the need for revision
                                ; levels for each module.
NNN_ECR_20      EQU     TRUE    ; This ECR includes the light pen into line code
                                ; in CRTDSR.ASM
NNN_ECR_28      EQU     TRUE
NNN_ECR_29      EQU     TRUE
;
;	FOLLOWING WERE ENHANCEMENTS FOR MEMORY TESTING
;
NNN_MEM_START	EQU	TRUE	; MEMORY START TEST MESSAGE
NNN_FAILED	EQU	TRUE	; IF MEMORY FAILED REPORT ERROR
NNN_NEW_MEMORY	EQU	TRUE	; new memory test
NNNDEL_MODE	EQU	TRUE	; USED IN MTESTS TO DELETE THE MULTIPLE
				; VMODE TO RMODE CALLS
NNNFAST_SPINUP	EQU	TRUE	; ALLOW FOR ONLY 75 MILLISECOND RATHER
				; THAN 1 SECOND ON SPINUP WAIT
;
;	BUG FIXES
;
NNNCO89030721	EQU	TRUE	; INT 13 FUNC 1 ERRSTAT GET RESET
NNNCE89042810	EQU	TRUE	; INT 13 DRIVE OPEN FOR FUNCTION 5 SHOULD
				; REPORT 06 REPORTS 01 ERROR
NNNEN89030722	EQU	TRUE	; INT 13 FUNCT 17 DOES NOT REPORT ILLEGAL DRIVE
NNNEN89022203	EQU	TRUE
NNNEN88121440	EQU	TRUE	; abort memory test message
NNNEN89041203   EQU	TRUE	; call nmiack does not restore DX reg.
NNNEN88121443	EQU	TRUE		
NNNCE89061702	EQU	TRUE	; INT 13 Func 02/03/04 The AL regsiter
				; get reset to zero when illegal drive.
NNNCE89061705	EQU	TRUE	; FUNC 15 DOES NOT REPORT DMA ERROR
NNNCE89061704	EQU	TRUE	; FUNC 15 NOT CAPABLE OF REPORTING
				; A SEEK ERROR. IT WILL ALWAYS REPORT
				; ILLEGAL FUNCTION.
NNNCE89061703	EQU	TRUE	; FUNCTIONS 15,16,AND 18 DO NOT REPORT
				; ERROR 01 OF DL = 02.
NNNCE89061701	EQU	TRUE	; INT 13 fUNCT 15h DOES NOT REPORT SAME AS
				; IBM WHEN SET LOCATIONS 40:90 TO ZERO
				; AND 40:91 TO ZERO
NNNCE89061712	EQU	TRUE	; SECTOR NOT FOUND ERROR REPORTS THE NUMBER
				; OF GOOD SECTORS READ IN THE AL REGISTER
				; IBM REPORT ZERO IN THE AL
NNNCE89061711	EQU	TRUE	; IN READ/WRITE/VERIFY ROUTINE AN ERROR 
				; RETURNED NUMBER OF SUCESSFUL RECORDS. IBM
				; CLEARED THE AL. PTL NOW CLEARS THE AL.
NNNCE89070602	EQU	TRUE	; THE FD_RESET FUNCTION WOULD CLEAR THE AL
				; REGISTER. THE AL REGISTER WAS NOT TO BE 
				; CLEARED. REMOVED INSTRUCTION.

NNNCE89081102   EQU     TRUE    ; WHEN SCROLLING TAKES PLACE, SIZE OF MEMORY
                                ; OVERWRITES MESSAGE
NNNCE89081507   EQU     TRUE    ; ct8230 CHIP SET DOES NOT INCREMENT THE SI
                                ; REGSITER, AS A RESULT SYSTEM HAS 64K LESS
                                ; MEMORY
NNNCE89082302   EQU     TRUE    ; Extended memory in wrong place when scrool
                                ; takes place. Also when no extended memory
                                ; will not print "Beggining Memory Test".
NNNEA89051803	EQU	TRUE
NNNEA89051204	EQU	TRUE							     
NNNEA89053122	EQU	TRUE
NNNEA89060801	EQU	TRUE
NNNEA89061205	EQU	TRUE
NNNEA89061206	EQU	TRUE
NNNEA89061207	EQU	TRUE
NNNEA89061601 	EQU	TRUE
NNNEN88121403	EQU	TRUE

; ***************************************************************************
; Bug Fixes for Keyboard Int 9 and Int 16 Handlers
;	NUKBISR.ASM
;	NUKBDSR.ASM
; ***************************************************************************
NNNEA89052315	EQU	TRUE
NNNCO89030723	EQU	TRUE
NNNCO89030913	EQU	TRUE
NNNCO89010305	EQU	TRUE
NNNCO89031022	EQU	TRUE
NNNCO89031527	EQU	TRUE
NNNCO89030604	EQU	TRUE
; ***************************************************************************
;
NNNCO89062208	EQU	TRUE	; CONFCHK.ASM - set RTC error bit 2 in CMSTAT
;
; ***************************************************************************
; Bug fixes for hrddsr.asm - hard disk drive support

NNNCO89042801   equ     TRUE                    ; conner fix
NNNCO88121602   equ     TRUE                    ; cylinder fix
NNNCO88121601   equ     TRUE                    ; head fix
NNNCO89061918   equ     TRUE                    ; wrong value in AL returned
NNNCO89062214   equ     TRUE                    ; format (ah=05) fixes
NNNCO89070505   equ     TRUE                    ; slow hard drive fixes
                                                ; Conner 344 in particular
NNNCO89062210   equ     TRUE                    ; init drv param's fix
NNNCO89061917   equ     TRUE                    ; read/write long fixes
NNN_INT_FIX     equ     TRUE                    ; interrupt masking and 
                                                ; handling changes
NNN_HD_REVIEW   equ     TRUE                    ; changes made as a result of
                                                ; the hard disk code review
NNN_HD_REV_FIX  equ     TRUE                    ; it was found that some of
                                                ; the changes made by 
                                                ; NNN_HD_REVIEW can cause 
                                                ; hard disk boot problems
                                                ; this ifdef fixes the 
                                                ; problems
NNNCO89061909   equ     true                    ; reset drives fix
NNN_INT_REPAIR  equ     true                    ; fixes for hard disk 
                                                ; controller command writes
                                                ; and waiting for interrupts
NNNCO89101527   EQU     TRUE    ; # of sectors = 0 fix
NNNCO89092511   EQU     TRUE    ; 0 in extended drive type cmos byte fix

;****************************************************************************
;
; video bugs
;
NNNCE89110614   EQU     TRUE            ; AH does not return propper value
NNNCE89103051   EQU     TRUE            ; AX register incompatible in func 1
NNNCE89110613   EQU     TRUE            ; BH compatible in roll function
NNNCE89103052   EQU     TRUE            ; PALETTE SHOULD BE IN AL REG ON
                                        ; RETURN FROM FUNC SET MODE
NNNCE89103103   EQU     TRUE            ; FLICKERING DURING SCROLL
NNNCE89103053   EQU     TRUE            ; ILLEGAL MODE SET TO MODE 2
NNNCO89030725	EQU		TRUE 			; No light pen fix
NNNCO89072601	EQU		TRUE			;VERTST (vertical ret.test) fix.
NNNCO89040402	EQU		TRUE			;setup/post "not installed" bug.
NNNCO89062001	EQU		TRUE			;          " 
NNNCO89040403	EQU		TRUE			;video deterioration when 
NNNCO89032204	EQU		TRUE			; Setting illegal mode
										;in graphix
;NNNFASTVID		EQU		TRUE			;video performance improvement
						
;****************************************************************************
;
; New bugs numbers that were in the code, but not added to update.inc
;
; NNNEA89061412	EQU	TRUE			; (AT_DRVR.ASM)  (removed wds)
NNNEA89062001	EQU	TRUE			; (AT_DRVR.ASM) trashing AX

;****************************************************************************
;  Fixes to the Int 1Ah code (TIMDSR.ASM)
;****************************************************************************
NNNCO89081001	EQU	TRUE	; Incrementing near midnight 
NNNCO89081008   EQU	TRUE	; Wrong AH returned on illegal calls.

;****************************************************************************
; Fix for keyboard failure at low CPU speed with Extended Features enabled
; - reported by DG  (see the log file for NUKBISR.ASM.
;NNNXFEATS_BUG	EQU	TRUE
; ***************************************************************************

; To get the casdsr.asm extended block move enhancements NNNNEW must be
; TRUE.  However, this may be defined in your ROMOPT.INC already.  If it is
; not then uncomment the next line.
 NNNNEW        EQU     TRUE                    
; 
; Also one bug fix which resulted in rebooting the system when int 15 func
; 87 was used with invalid GDT is included in the NNNNEW code
; ***********************************************************************

NNNINT077  EQU     FALSE           ; for the 82077 install check see at_drvr.asm
;!!!!!! modif 03/01/90  J NOEL
USE_REFNO EQU   TRUE            ; this gives us the 3.10E copyright notice
                                ; see miscl1.inc rev 3.8 for details

;*************************************************************************
; CONSTRUCT
;*************************************************************************
;NNNCONSTRUCT	EQU	TRUE
;*************************************************************************
; Extended System Services - user new file XSERV.ASM
;*************************************************************************
;NNNESS	EQU	TRUE

;*************************************************************************
; CMOS-configurable keyboard NUMLOCK boot state
;*************************************************************************
NNNNUMLOCK	EQU	TRUE

;*************************************************************************
; More bug fixes - KILL THOSE BUGS!
;*************************************************************************

NNNCO89082507	EQU	TRUE	; changed "chipset" to "chip set" in a
				; displayed message in CUSTINIT.ASM 3.18

NNNCO89092512 equ       TRUE    ; chipset 40h bit 7 and ECMOS checksum
                                ; clash resolution

NNNCO89091812	EQU	TRUE	; Warm boots were forcing slow speed on
				; C&T Neat systems.

NNNCO89110902	EQU	TRUE	; CTRL-BREAK during NOVELL COMPSURF was
				; hanging system.

NNNCO90010902	EQU	TRUE	; When running in OS/2's DOS compatibility
				; box, typing 's' would bring up ROM-based
				; Setup

