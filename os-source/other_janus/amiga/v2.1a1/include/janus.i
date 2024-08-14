   IFND  JANUS_JANUS_I
JANUS_JANUS_I  SET   1

;*************************************************************************
; (Amiga side file)
;
; janus.i -- Include all other janus include files
;
; Copyright (c) 1986, Commodore Amiga Inc.,  All rights reserved.
;
; 7-15-88 - Bill Koester - Created this file
;*************************************************************************

   INCLUDE  "janus/janusbase.i"
   INCLUDE  "janus/janusreg.i"
   INCLUDE  "janus/janusvar.i"
   INCLUDE  "janus/memory.i"
   INCLUDE  "janus/memrw.i"
   INCLUDE  "janus/harddisk.i"
   INCLUDE  "janus/services.i"
   INCLUDE  "janus/syscall.i"
   INCLUDE  "janus/dosserv.i"
   INCLUDE  "janus/timeserv.i"

   ENDC  ;End of JANUS_JANUS_I conditional assembly
