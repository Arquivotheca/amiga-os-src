
;**************************************************************************
; memrw.inc - parameter area definition for access to other processors mem
;**************************************************************************
; This is the parameter block for the JSERV_READPC and JSERV_READAMIGA
; services -- read and/or write the other processors memory.
;**************************************************************************
MemReadWrite struc

mrw_Command dw	0	      ; see below for list of commands
mrw_Count   dw	0	      ; number of bytes to transfer
mrw_Address dd	0	      ; local address to access.  This is
			      ; a machine pointer for the 68000, and
			      ; a segment/offset pair for the 808x 
			      ; (segment is the low word of a 68000
			      ; long word).
			      ; The address is arranged so the native
			      ; processor may read it directly.
			      ; For a 8088 I/O request, the segment word is 
			      ; ignored.
mrw_Buffer  dw	0	      ; The offset in buffer mem for the data buffer.
mrw_Status  dw	0	      ; See below for status.

MemReadWrite ends
;******************************************************************
; command definitions
;******************************************************************
MRWC_NOP       = 0	      ; do nothing -- return OK status code
MRWC_READ      = 1	      ; xfer from address to buffer
MRWC_WRITE     = 2	      ; xfer from buffer to address
MRWC_READIO    = 3	      ; only on 808x -- read from IO space
MRWC_WRITEIO   = 4	      ; only on 808x -- write to IO space
MRWC_WRITEREAD = 5	      ; write from buffer, then read back

;******************************************************************
; status definitions
;******************************************************************
MRWS_INPROGRESS =   0ffffH    ; we've noticed command and are working on it
MRWS_OK 	=   00000H    ; completed OK
MRWS_ACCESSERR	=   00001H    ; some sort of protection violation
MRWS_BADCMD	=   00002H    ; command that the server doesn't understand

