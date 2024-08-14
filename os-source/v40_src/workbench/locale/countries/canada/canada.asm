
	NOLIST

        INCLUDE "prefs/locale.i"
        INCLUDE "prefs/prefhdr.i"
	INCLUDE "libraries/locale.i"

	LIST

;---------------------------------------------------------------------------

PUTPAD	MACRO <Label>,<FieldSize>
\@	DS.B	\2+\1-\*VALOF(\@)
	ENDM

PADSTR	MACRO <String>,<FieldSize>
\@      CSTRING \1
        PUTPAD  \*VALOF(\@),\2
	ENDM

;---------------------------------------------------------------------------

L000 DC.B   "FORM"			; IFF FORM identifier
L001 DC.L   PrefHeader_SIZEOF+CountryPrefs_SIZEOF+20
L002 DC.B   "PREF"			; form type

L003 DC.B   "PRHD"			; pref header chunk identifier
L004 DC.L   PrefHeader_SIZEOF		; chunk length
L005 DC.W   0				; ph_Version
L006 DC.W   0				; ph_Type
L007 DC.W   0				; ph_Flags

L008 DC.B   "CTRY"
L009 DC.L   CountryPrefs_SIZEOF
L010 DC.L   0,0,0,0			; cp_Reserved
L011 DC.B   'CAN',0			; cp_CountryCode
L012 DC.L   1				; cp_TelephoneCode
L013 DC.B   MS_ISO			; cp_MeasuringSystem

L014 PADSTR "%A %B %e %Y %H:%M:%S",80	; cp_DateTimeFormat
L015 PADSTR "%A %B %e %Y",40		; cp_DateFormat
L016 PADSTR "%H:%M:%S",40		; cp_TimeFormat

L017 PADSTR "%d/%m/%y %H:%M:%S",80	; cp_ShortDateTimeFormat
L018 PADSTR "%d/%m/%y",40		; cp_ShortDateFormat
L019 PADSTR "%H:%M",40			; cp_ShortTimeFormat

L020 PADSTR ",",10			; cp_DecimalPoint
L021 PADSTR " ",10			; cp_GroupSeparator
L022 PADSTR " ",10			; cp_FracGroupSeparator
L023 PADSTR 3,10			; cp_Grouping
L024 PADSTR 3,10			; cp_FracGrouping

L025 PADSTR ",",10			; cp_MonDecimalPoint
L026 PADSTR " ",10			; cp_MonGroupSeparator
L027 PADSTR " ",10			; cp_MonFracGroupSeparator
L028 PADSTR 3,10			; cp_MonGrouping
L029 PADSTR 3,10			; cp_MonFracGrouping
L030 DC.B   2				; cp_MonFragDigits
L031 DC.B   2				; cp_MonIntFracDigits

L032 PADSTR "$",10			; cp_MonCS
L033 PADSTR "¢",10			; cp_MonSmallCS
L034 PADSTR "CDN",10			; cp_MonIntCS

L035 PADSTR 0,10			; cp_MonPositiveSign
L036 DC.B   SS_NOSPACE			; cp_MonPositiveSpaceSep
L037 DC.B   SP_PREC_ALL			; cp_MonPositiveSignPos
L038 DC.B   CSP_SUCCEEDS		; cp_MonPositiveCSPos

L039 PADSTR "-",10			; cp_MonNegativeSign
L040 DC.B   SS_NOSPACE			; cp_MonNegativeSpaceSep
L041 DC.B   SP_PREC_ALL			; cp_MonNegativeSignPos
L042 DC.B   CSP_SUCCEEDS		; cp_MonNegativeCSPos

L043 DC.B   0				; cp_Pad

	END
