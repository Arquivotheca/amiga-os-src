   IFND  JANUS_MEMRW_I
JANUS_MEMRW_I  SET   1

;************************************************************************
; (Amiga side file)
;
; memrw.i -- parameter area definition for access to other processors mem
;
; Copyright (c) 1986, Commodore Amiga Inc.,  All rights reserved
;
; 7-15-88 - Bill Koester - Modified for self inclusion of required files
;************************************************************************

   IFND  EXEC_TYPES_I
   INCLUDE "exec/types.i"
   ENDC

; this is the parameter block for the JSERV_READPC and JSERV_READAMIGA
; services -- read and/or write the other processors memory.

 STRUCTURE  MemReadWrite,0

   UWORD    mrw_Command       ; see below for list of commands
   UWORD    mrw_Count         ; number of bytes to transfer
   ULONG    mrw_Address       ; local address to access.  This is
                              ;   a machine pointer for the 68000, and
                              ;   a segment/offset pair for the 808x.
                              ;   The address is arranged so the native
                              ;   processor may read it directly.
   UWORD    mrw_Buffer        ; The offset in buffer memory for the
                              ;   other buffer.
   UWORD    mrw_Status        ; See below for status.

   LABEL    MemReadWrite_SIZEOF


; command definitions

MRWC_NOP          EQU   0  ; do nothing -- return OK status code
MRWC_READ         EQU   1  ; xfer from address to buffer
MRWC_WRITE        EQU   2  ; xfer from buffer to address
MRWC_READIO       EQU   3  ; only on 808x -- read from IO space
MRWC_WRITEIO      EQU   4  ; only on 808x -- write to IO space
MRWC_WRITEREAD    EQU   5  ; write from buffer, then read back


; status definitions

MRWS_INPROGRESS   EQU   $ffff ; we've noticed command and are working on it
MRWS_OK           EQU   $0000 ; completed OK
MRWS_ACCESSERR    EQU   $0001 ; some sort of protection violation
MRWS_BADCMD       EQU   $0002 ; command that the server doesn't understand


   ENDC  ;End of JANUS_MEMRW_I conditional assembly


