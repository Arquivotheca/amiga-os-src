
;***********************************************************************
;
; assembly.i -- included by all janus code before anything else
;
; Copyright (c) 1986, Commodore Amiga Inc.,  All rights reserved
;
;***********************************************************************



   SECTION section

INFOLEVEL   EQU   0   ; No messages
;INFOLEVEL   EQU   1   ; Function entry argument messages
;INFOLEVEL   EQU   2   ; Function exit messages
;INFOLEVEL   EQU   3   ; Function internal messages
;INFOLEVEL   EQU   4   ; Function internal nit picky messages
;INFOLEVEL   EQU   100   ; The works (hoo boy!)
;INFOLEVEL   EQU   105 


; hard coded signals
ACPSB_CALL   EQU   31
ACPSF_CALL   EQU   $8000
ACPSB_EXIT   EQU   30
ACPSF_EXIT   EQU   $4000

