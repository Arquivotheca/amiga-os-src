;*********************************************************************
;
; services.i -- define common service numbers between PC and Amiga
;
; Copyright (c) 1986, Commodore Amiga Inc., All rights reserved
;
;*********************************************************************
;
; this is the table of hard coded services.  Other services may exist
; that are dynamically allocated via AllocJanusService.
;
; Service numbers constrained by hardware:
; These services generate interrupts by PC Hardware events:
;
JSERV_MINT	EQU	0	; monochrome display written to
JSERV_GINT	EQU	1	; color display written to
JSERV_CRT1INT	EQU	2	; mono display's control registers changed
JSERV_CRT2INT	EQU	3	; color display's control registers changed
JSERV_ENBKB	EQU	4	; keyboard ready for next character
JSERV_LPT1INT	EQU	5	; parallel control register
JSERV_COM2INT	EQU	6	; serial control register
;
; hard coded service numbers
;
JSERV_PCBOOTED	EQU	7	; PC is ready to service soft interrupts

JSERV_SCROLL	EQU	8	; PC is scrolling its screen
JSERV_HARDDISK	EQU	9	; Amiga reading PC hard disk
JSERV_READAMIGA EQU	10	; PC reading Amiga mem
JSERV_READPC	EQU	11	; Amiga reading PC mem
JSERV_AMIGACALL EQU	12	; PC executing Amiga subroutine
JSERV_PCCALL	EQU	13	; Amiga causing PC interrupt
JSERV_PCFILE	EQU	14	; PC transferring Amiga file
JSERV_NEWPCSERV EQU	15	; Amiga initiating PC side of a new service

