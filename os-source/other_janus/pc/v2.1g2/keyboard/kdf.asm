	TITLE	PC DOS 3.3 Keyboard Definition File

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; PC DOS 3.3 - NLS Support - Keyboard Definition File
;; (c) Copyright IBM Corp 198?,...
;;
;; Update:	Torsten Burgdorff/Commodore BSW
;;         20.1.89 KDFSV.asm for swedish keyboard with Mitsumi layout
;;         20.1.89 KDFSD.asm for swedish keyboard with IBM layout
;;
;;		Holger Heinemeyer/Commodore BSW
;;	   03.08.90  KDF.ASM  Version of KEYBOARD.SYS
;;
;; This the file header and table pointers ONLY.
;; The actual tables are contained in seperate source files.
;; These are:
;;	     KDFSP.ASM	- Spanish
;;	     KDFGR.ASM	- German
;;	     KDFIT.ASM	- Italian
;;	     KDFFR.ASM	- French
;;	     KDFSG.ASM	- Swiss German
;;	     KDFDK.ASM	- Danish
;;	     KDFUK.ASM	- English
;;	     KDFNO.ASM	- Norway
;;	     KDFSV.ASM	- SWEDEN (Mitsumi layout)
;;	     KDFSD.ASM	- Sweden/Finland (IBM layout)	
;;	     KDFUS.ASM	- US (DUMMY)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
				       ;;
CODE	SEGMENT PUBLIC 'CODE'          ;;
	ASSUME CS:CODE,DS:CODE	       ;;
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;***************************************
;; File Header
;;***************************************
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
DB   0FFh,'KEYB   '                  ;; signature
DB   2,2			     ;; VERSION	(vhh) 03.08.90
DB   6 DUP(0)			     ;; reserved
DW   650			     ;; maximum size of Common Xlat Sect
DW   350			     ;; max size of Specific Xlat Sect
DW   400			     ;; max size of State Logic
DD   0				     ;; reserved
DW   10 			     ;; number of languages
DB   'SP'
DW   OFFSET SP_LANG_ENT,0
DB   'FR'
DW   OFFSET FR_LANG_ENT,0
DB   'DK'
DW   OFFSET DK_LANG_ENT,0
DB   'SG'
DW   OFFSET SG_LANG_ENT,0
DB   'GR'
DW   OFFSET GE_LANG_ENT,0
DB   'IT'
DW   OFFSET IT_LANG_ENT,0
DB   'UK'
DW   OFFSET UK_LANG_ENT,0
DB   'NO'
DW   OFFSET NO_LANG_ENT,0
DB   'SV'
DW   OFFSET SV_LANG_ENT,0
DB   'US'
DW   OFFSET US_LANG_ENT,0
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;***************************************
;; Language Entries
;;***************************************
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
   EXTRN SP_LOGIC:NEAR		       ;;
   EXTRN SP_437_XLAT:NEAR	       ;;
   EXTRN SP_850_XLAT:NEAR	       ;;
				       ;;
SP_LANG_ENT:			       ;; language entry for SPANISH
  DB   'SP'                            ;;
  DW   0			       ;; reserved
  DW   OFFSET SP_LOGIC,0	       ;; pointer to LANG kb table
  DW   2			       ;; number of code pages
  DW   437			       ;; code page
  DW   OFFSET SP_437_XLAT,0	       ;; table pointer
  DW   850			       ;; code page
  DW   OFFSET SP_850_XLAT,0	       ;; table pointer
				       ;;
;****************************************************************************
    EXTRN FR_LOGIC:NEAR 		;;
    EXTRN FR_437_XLAT:NEAR		;;
    EXTRN FR_850_XLAT:NEAR		;;
					;;
 FR_LANG_ENT:				;; language entry for POTUGAL
   DB	'FR'                            ;;
   DW	0				;; reserved
   DW	OFFSET FR_LOGIC,0		;; pointer to LANG kb table
   DW	2				;; number of code pages
   DW	437				;; code page
   DW	OFFSET FR_437_XLAT,0		;; table pointer
   DW	850				;; code page
   DW	OFFSET FR_850_XLAT,0		;; table pointer
					;;
;*****************************************************************************
   EXTRN DK_LOGIC:NEAR		       ;;
   EXTRN DK_865_XLAT:NEAR	       ;;
   EXTRN DK_850_XLAT:NEAR	       ;;
					;;
 DK_LANG_ENT:				;; language entry for POTUGAL
   DB	'DK'                            ;;
   DW	0				;; reserved
   DW	OFFSET DK_LOGIC,0		;; pointer to LANG kb table
   DW	2				;; number of code pages
   DW	865				;; code page
   DW	OFFSET DK_865_XLAT,0		;; table pointer
   DW	850				;; code page
   DW	OFFSET DK_850_XLAT,0		;; table pointer
					;;
;*****************************************************************************
   EXTRN SG_LOGIC:NEAR		       ;;
   EXTRN SG_437_XLAT:NEAR	       ;;
   EXTRN SG_850_XLAT:NEAR	       ;;
				       ;;
SG_LANG_ENT:			       ;; language entry for POTUGAL
  DB   'SG'                            ;;
  DW   0			       ;; reserved
  DW   OFFSET SG_LOGIC,0	       ;; pointer to LANG kb table
  DW   2			       ;; number of code pages
  DW   437			       ;; code page
  DW   OFFSET SG_437_XLAT,0	       ;; table pointer
  DW   850			       ;; code page
  DW   OFFSET SG_850_XLAT,0	       ;; table pointer
				       ;;
;*****************************************************************************
   EXTRN GE_LOGIC:NEAR		       ;;
   EXTRN GE_437_XLAT:NEAR	       ;;
   EXTRN GE_850_XLAT:NEAR	       ;;
				       ;;
GE_LANG_ENT:			       ;; language entry for POTUGAL
  DB   'GR'                            ;;
  DW   0			       ;; reserved
  DW   OFFSET GE_LOGIC,0	       ;; pointer to LANG kb table
  DW   2			       ;; number of code pages
  DW   437			       ;; code page
  DW   OFFSET GE_437_XLAT,0	       ;; table pointer
  DW   850			       ;; code page
  DW   OFFSET GE_850_XLAT,0	       ;; table pointer
				       ;;
;*****************************************************************************
    EXTRN IT_LOGIC:NEAR 		;;
    EXTRN IT_437_XLAT:NEAR		;;
    EXTRN IT_850_XLAT:NEAR		;;
					;;
 IT_LANG_ENT:				;; language entry for POTUGAL
   DB	'IT'                            ;;
   DW	0				;; reserved
   DW	OFFSET IT_LOGIC,0		;; pointer to LANG kb table
   DW	2				;; number of code pages
   DW	437				;; code page
   DW	OFFSET IT_437_XLAT,0		;; table pointer
   DW	850				;; code page
   DW	OFFSET IT_850_XLAT,0		;; table pointer
					;;
;*****************************************************************************
    EXTRN UK_LOGIC:FAR			;;
    EXTRN UK_437_XLAT:FAR		;;
    EXTRN UK_850_XLAT:FAR		;;
					;;
 UK_LANG_ENT:				;; language entry for POTUGAL
   DB	'UK'                            ;;
   DW	0				;; reserved
   DW	OFFSET UK_LOGIC,0		;; pointer to LANG kb table
   DW	2				;; number of code pages
   DW	437				;; code page
   DW	OFFSET UK_437_XLAT,0		;; table pointer
   DW	850				;; code page
   DW	OFFSET UK_850_XLAT,0		;; table pointer
					;;
;*****************************************************************************
     EXTRN NO_LOGIC:NEAR		 ;;
     EXTRN NO_865_XLAT:NEAR		 ;;
     EXTRN NO_850_XLAT:NEAR		 ;;
					 ;;
  NO_LANG_ENT:				 ;; language entry for NORWAY
    DB	 'NO'                            ;;
    DW	 0				 ;; reserved
    DW	 OFFSET NO_LOGIC,0		 ;; pointer to LANG kb table
    DW	 2				 ;; number of code pages
    DW	 865				 ;; code page
    DW	 OFFSET NO_865_XLAT,0		 ;; table pointer
    DW	 850				 ;; code page
    DW	 OFFSET NO_850_XLAT,0		 ;; table pointer
				     ;;
;*****************************************************************************
     EXTRN SV_LOGIC:NEAR		 ;;
     EXTRN SV_437_XLAT:NEAR		 ;;
     EXTRN SV_850_XLAT:NEAR		 ;;
				 ;;			       	
  SV_LANG_ENT:			 ;; language entry for SWEDEN/Finland
    DB	 'SV'                          ;;  with Mitsumi layout
    DW	 0			 ;; reserved
    DW	 OFFSET SV_LOGIC,0		 ;; pointer to LANG kb table
    DW	 2			 ;; number of code pages
    DW	 437			 ;; code page
    DW	 OFFSET SV_437_XLAT,0	 ;; table pointer
    DW	 850			 ;; code page
    DW	 OFFSET SV_850_XLAT,0	 ;; table pointer
				 ;;
;*****************************************************************************
     EXTRN US_LOGIC:NEAR		 ;;
     EXTRN US_437_XLAT:NEAR
				 ;;			       	
  US_LANG_ENT:			 ;; language entry for USA
    DB	 'US'                          ;;  with Mitsumi layout
    DW	 0			 ;; reserved
    DW	 OFFSET US_LOGIC,0		 ;; pointer to LANG kb table
    DW	 1			 ;; number of code pages
    DW   437
    DW   OFFSET US_437_XLAT,0
;*****************************************************************************
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
DUMMY_ENT:			       ;; language entry
  DB   'XX'                            ;;
  DW   0			       ;; reserved
  DW   OFFSET DUMMY_LOGIC,0	       ;; pointer to LANG kb table
  DW   5			       ;; number of code pages
  DW   437			       ;; code page
  DW   OFFSET DUMMY_XLAT_437,0	       ;; table pointer
  DW   850			       ;; code page
  DW   OFFSET DUMMY_XLAT_850,0	       ;; table pointer
  DW   860			       ;; code page
  DW   OFFSET DUMMY_XLAT_860,0	       ;; table pointer
  DW   863			       ;; code page
  DW   OFFSET DUMMY_XLAT_863,0	       ;; table pointer
  DW   865			       ;; code page
  DW   OFFSET DUMMY_XLAT_865,0	       ;; table pointer
				       ;;
DUMMY_LOGIC:			       ;;
   DW  LOGIC_END-$		       ;; length
   DW  0			       ;; special features
   DB  92H,0,0			       ;; EXIT_STATE_LOGIC_COMMAND
LOGIC_END:			       ;;
				       ;;
DUMMY_XLAT_437: 		       ;;
   DW	  6			       ;; length of section
   DW	  437			       ;; code page
   DW	  0			       ;; LAST STATE
				       ;;
DUMMY_XLAT_850: 		       ;;
   DW	  6			       ;; length of section
   DW	  850			       ;; code page
   DW	  0			       ;; LAST STATE
				       ;;
DUMMY_XLAT_860: 		       ;;
   DW	  6			       ;; length of section
   DW	  860			       ;; code page
   DW	  0			       ;; LAST STATE
				       ;;
DUMMY_XLAT_865: 		       ;;
   DW	  6			       ;; length of section
   DW	  865			       ;; code page
   DW	  0			       ;; LAST STATE
				       ;;
DUMMY_XLAT_863: 		       ;;
   DW	  6			       ;; length of section
   DW	  863			       ;; code page
   DW	  0			       ;; LAST STATE
				       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;*****************************************************************************
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;;
CODE	 ENDS			       ;;
	 END			       ;;
