
*****************************************************************
*                                                               *
* Copyright 1985, 1990 Commodore Amiga Inc.All rights reserved. *
* No part of this program may be reproduced, transmitted,       *
* transcribed, stored in retrieval system, or translated into   *
* any language or computer language, in any form or by any      *
* means, electronic, mechanical, magnetic, optical, chemical,   *
* manual or otherwise, without the prior written permission of  *
* Commodore Amiga Incorporated, 983-D University Blvd, Los      *
* Gatos, CA                                                     *
*                                                               *
*****************************************************************
;
;SERPER = 3579545/baud - 1      (round to the nearest integer)
;
;This yields 113.55 or 114 when rounded.  Here's a little table showing
;that either 113 or 114 are within the tolerances for the MIDI standard:
;
;  Baud   Difference
;  -----  ------------------
;  31126  SERPER=114 (-0.4%)
;  31250  MIDI baud rate (+/- 1% is acceptable)
;  31400  SERPER=113 (+0.5%)
;
;For reference, here's a few other baud rates:
;
;  Baud   SERPER
;  -----  ------
;  300    11931
;  1200   2982
;  9600   372

*****************************************************************
*
* serialdev.i -- external declarations for Serial Port Driver
*
* SOURCE CONTROL
* ------ -------
* $Id: serialdev.i,v 1.3 90/12/01 20:50:43 bryce Exp $
*
* $Locker:  $
*
*****************************************************************

*    IFND    DEVICES_SERIAL_I
*DEVICES_SERIAL_I SET 1
*
*    IFND    EXEC_STRINGS_I
*    include 'exec/strings.i'
*    ENDC    !EXEC_STRINGS_I
*
*
*    IFND     EXEC_IO_I
*    include 'exec/io.i'
*    ENDC    !EXEC_IO_I
*
*--------------------------------------------------------------------
*
* Useful constants
*
*--------------------------------------------------------------------
*
SER_CTL       EQU     $11130000 ; default char's for xON,Xoff,reserved,rsvd.

PERIOD_9600_BAUD EQU 372	;_serper value for 9600 baud (default)
SER_DRBITS    EQU     $FF       ; default read word size (bits masked on)
SER_DWBITS    EQU     $FF       ; default write word size (bits masked on)
SER_DSBITS    EQU     $100      ; default stop bit(s) set (bit masked on)
;FIXED-bryce:These are bit numbers, not masks
SERB_PER9W    EQU     15        ; bit in _serper reg for 9bit data read
;SERB_OVERRUN  EQU     7         ; bit in _serdatr HOB for read overrun
SER_DBAUD     EQU     9600      ; default baud
SER_DSPEED    EQU     $0174     ; default "baud"s hex of 0373
SER_DRBFSIZE  EQU     $200      ; dflt size for read buffers
MIN_RBUFSIZE  EQU     64        ; minimum size for read  buffers
SER_DSBX      EQU     $08080100 ; dflt wordlengths for read, write,stop;flags
SER_DBRKTIME  EQU     250000    ; default break duration in milleseconds
CharMax       EQU     8         ; maximum number of bits in data word
StopMax       EQU     2         ; maximum number of stop bits
MinBaud       EQU     112       ; serper/UART's slowest baud
Magic110Baud  EQU     110       ; when we see 110, we round to 112
MaxBaud       EQU     292000    ; Magic constant's maximum baud 
;                                PAL (50 hertz)  NTSC (60 hertz video)
;                                -------------------- 
;Color Clock Times               281.93 ns       279.36 ns
;Crystal Frequency               28.37516 MHz    28.63636 
BaudMagic     EQU     25000000  ; Magic constant: don't change (it is wrong!-bryce)
PALBaudMagic  EQU     24772416  ; Another Magic constant: don't change
ParityMagic   EQU     $69966996 ; Magic constant for parity mask
MagicMult     EQU     7         ; Magic constant: don't change
MagicShift    EQU     6         ; 6 bit shift= 22 baud cycles per handshake
*                                 wait. 7bs=>45, 5bs=>11, etc...
prae          EQU     $BFD000   ; hardware address of serial status register
ddrae         EQU     $BFD200   ; address of serial status data direction reg
writedir      EQU     $FF       ; data direction = write
readdir       EQU     $0        ; data direction = read
SerQueryBits  EQU     $FC       ; prae bits that Query shows to user
SerRegBits    EQU     $F8       ; prae bits belonging to serial port
*                                 (this should be EQU COMDTR+RTS+CD+CTS+DSR)
SerStatMask   EQU     $1F       ; show these status bits to user's IORqst
SER_TIMEOUT   EQU     1500   	; serial device timeout
;bryce-the timeout had to be increased... sometimes it would break the
;printer.device to the punch.
*
*--------------------------------------------------------------------
*
* Driver Specific Commands

SDCMD_QUERY     EQU     CMD_NONSTD
SDCMD_BREAK     EQU     CMD_NONSTD+1
SDCMD_SETPARAMS EQU     CMD_NONSTD+2

SER_DEVFINISH   EQU     CMD_NONSTD+2 ; number of device comands 

*--------------------------------------------------------------------

*-- SERIALNAME is a generic macro to get the name of the driver.  This
*-- way if the name is ever changed you will pick up the change
*-- automatically.
*--
*-- Normal usage would be:
*--
*-- internalName:	SERIALNAME
*--

SERIALNAME:	MACRO
		dc.b 'serial.device',0
		cnop 0,2
		ENDM

        BITDEF  SER,XDISABLED,7   ; SERFLAGS xOn-xOff feature disabled bit
        BITDEF	SER,EOFMODE,6     ;    "     EOF mode enabled bit
        BITDEF	SER,SHARED,5      ;    "     non-exclusive access
        BITDEF  SER,RAD_BOOGIE,4  ;    "     high-speed mode active
        BITDEF	SER,QUEUEDBRK,3   ;    "     queue this break request
        BITDEF	SER,7WIRE,2       ;    "     RS232 7-wire protocol
        BITDEF	SER,PARTY_ODD,1   ;    "     parity feature enabled bit
        BITDEF	SER,PARTY_ON,0    ;    "     parity-enabled bit 
        BITDEF	IO,QUEUED,6       ; IO_FLAGS rqst-queued bit
        BITDEF	IO,ABORT,5        ;     "    rqst-aborted bit
        BITDEF	IO,ACTIVE,4       ;     "    rqst-qued-or-current bit
        BITDEF	SER,IRQLOCK,7     ; UNSER_FLAG2 lock out irqs kludge
        BITDEF	SER,EXPUNGE,6     ;     "    delayed expunge    
        BITDEF	SER,9BITRD,5      ;     "    set for 9bit reads
        BITDEF  SER,WEOL,4        ;     "    set if wrt char = $00
        BITDEF  SER,XOFFRSET,3    ;     "    xoff (read) active
        BITDEF  SER,XOFFWSET,2    ;     "    xoff (write) active
        BITDEF	SER,RBUSY,1       ;     "    read busy
        BITDEF	IOST,PARITYERR,7  ; IOST_HOB parity err at RBUFOD
        BITDEF	IOST,READINGBRK,6 ;     "    actually reading break
        BITDEF	IOST,WRITINGBRK,5 ;     "    actually writing break
        BITDEF	IOST,XOFFREAD,4   ;     "    receive currently xOFF'ed
        BITDEF	IOST,XOFFWRITE,3  ;     "    transmit currently xOFF'ed
        BITDEF	IOST,READBREAK,2  ;     "    break was latest input
        BITDEF	IOST,WROTEBREAK,1 ;     "    break was latest output
        BITDEF	IOST,OVERRUN,0    ;     "    status word RBF overrun
*       BITDEF's (longword field) ; EXTFLAGS 
        BITDEF	SEXT,MSPON,1      ;     "    use mark-or-space parities:
*                                 ;      (this will also set SERB_PARTY_ON)
        BITDEF  SEXT,MARK,0       ;     "    mark (vs. space) parity
        BITDEF	ADKCON,SETCLR,15  ; ADCKON reg bit for setting/clearing
        BITDEF	ADKCON,UARTBRK,11 ;   "    reg bit for BREAK

*
********************************************************************************
 STRUCTURE TERMARRAY,0
        ULONG    TERMARRAY_0
        ULONG    TERMARRAY_1
        LABEL    TERMARRAY_SIZE

 STRUCTURE IOEXTSER,IOSTD_SIZE

*     STRUCT   MsgNode
*   0   APTR     Succ
*   4   APTR     Pred
*   8   UBYTE    Type
*   9   UBYTE    Pri
*   A   APTR     Name
*   E   APTR     ReplyPort
*  12   UWORD    MNLength
*     STRUCT   IOExt
*  14   APTR     IO_DEVICE
*  18   APTR     IO_UNIT
*  1C   UWORD    IO_COMMAND
*  1E   UBYTE    IO_FLAGS
*  1F   UBYTE    IO_ERROR
*     STRUCT   IOStdExt
*  20   ULONG    IO_ACTUAL
*  24   ULONG    IO_LENGTH
*  28   APTR     IO_DATA
*  2C   ULONG    IO_OFFSET
*

* IMPORTANT !! DON'T CHANGE the long-word alignment of any of these fields !!
*  30
        ULONG   IO_CTLCHAR      ; control char's (order = xON,xOFF,INQ,ACK)
        ULONG   IO_RBUFLEN      ; length in bytes of serial port's read buffer
        ULONG   IO_EXTFLAGS     ; additional serial flags (see bitdefs above)
	ULONG   IO_BAUD         ; baud rate requested (true baud)
	ULONG   IO_BRKTIME      ; duration of break signal in MICROseconds
        STRUCT  IO_TERMARRAY,TERMARRAY_SIZE ; termination character array
	UBYTE   IO_READLEN      ; read word length (count)
	UBYTE   IO_WRITELEN     ; write word length (count)
	UBYTE   IO_STOPBITS     ; stopbits for read (count)
        UBYTE   IO_SERFLAGS     ; see SERFLAGS bit definitions above 
        UWORD   IO_STATUS       ; status of serial port, as follows:
*
*                  BIT  ACTIVE  FUNCTION
*                   0    low    busy (!private!)
*                   1    low    paper out (!private!)
*                   2    low    select & A500/A2000 ring indicator
*                   3    low    Data Set Ready
*                   4    low    Clear To Send
*                   5    low    Carrier Detect
*                   6    low    Ready To Send
*                   7    low    Data Terminal Ready
*                   8    high   read overrun
*                   9    high   break sent
*                  10    high   break received
*                  11    high   transmit x-OFFed       
*                  12    high   receive x-OFFed       
*               13-15           reserved
*
        LABEL   IOEXTSER_SIZE

*********************************************************************************
*
 STRUCTURE UNITEXTSER,DD_SIZE
*     STRUCT   ListNode
*   0   APTR     Succ
*   4   APTR     Pred
*   8   UBYTE    Type
*   9   UBYTE    Pri
*   A   APTR     Name
*     STRUCT   Lib(rary)
*   E   UBYTE    LIB_FLAGS
*   F   UBYTE    LIB_pad
*  10   UWORD    LIB_NEGSIZE
*  12   UWORD    LIB_POSSIZE
*  14   UWORD    LIB_VERSION
*  16   UWORD    LIB_REVISION
*  18   APTR     LIB_IDSTRING
*  1C   ULONG    LIB_SUM
*  20   UWORD    LIB_OPENCNT
*

*    IMPORTANT !!!!        DON'T CHANGE the adjacency of the next 14 fields !!
*  22
        UBYTE   UNSER_XON       ; ASCII value to use for xON (DON'T CHANGE ORDER)
        UBYTE   UNSER_XOFF      ; ASCII value to use for xOFF   "      "     "  )
        UBYTE   UNSER_INQ       ; (not used)                    "      "     "  )
        UBYTE   UNSER_ACK       ; (not used)                    "      "     "  )
        APTR    UNSER_RBUF      ; pointer to serial port's read buffer "  "  "  )
        APTR    UNSER_RBUFPUT   ; pointer where to add to read buffer  "  "  "  )
        APTR    UNSER_RBUFGET   ; pointer where to get from read buffer"  "  "  )
        APTR    UNSER_RBUFMAX   ; pointer to end of read buffer        "  "  "  )
        ULONG   UNSER_RBUFLEN   ; byte length of read buffer           "  "  "  )
        ULONG   UNSER_RBUFCNT   ; count of valid input in read buffer  "  "  "  )
        ULONG   UNSER_RBUFOD    ; if nonzero, last valid data byte in RBUF
        UBYTE   UNSER_RWLCNT    ; read word length (count)
        UBYTE   UNSER_WWLCNT    ; write word length (count)
        UBYTE   UNSER_SBLCNT    ; number of stop bits
        UBYTE   UNSER_FLAGS     ; see UNSER_FLAG2 bit definitions above
        UWORD   UNSER_RQCNT     ; count of read queue entries
        UWORD   UNSER_WQCNT     ; count of write queue entries
        APTR    UNSER_RCURIOR   ; pointer to current read IO_Rqst
        APTR    UNSER_WCURIOR   ; pointer to current write IO_Rqst
        APTR    UNSER_RDATA     ; pointer to read's current IO data-byte location
        APTR    UNSER_WDATA     ; pointer to write's current IO data-byte location
        ULONG   UNSER_RLEN      ; number of characters read in current IOR
        ULONG   UNSER_WLEN      ; number of characters written in current IOR
        APTR    UNSER_EXLIB     ; base address of the exec library
        APTR    UNSER_TIMER     ; address of timer device
        ULONG   UNSER_BRKTIME   ; number of MICROseconds duration for break
        STRUCT  UNSER_TARRAY,TERMARRAY_SIZE ; termination character array
        STRUCT  serReadStruct,IS_SIZE   ; Read interrupt data structure
        STRUCT  serWriteStruct,IS_SIZE  ; Write interrupt data structure
        STRUCT  serReadQueue,LH_SIZE    ; Read IO_Rqst queue data struct
        STRUCT  serWriteQueue,LH_SIZE   ; Write IO_Rqst queue data struct
        STRUCT  WriteSoftInt,IS_SIZE    ; SoftIntStruct for Write io_rqst
        STRUCT  ReadSoftInt,IS_SIZE     ; SoftIntStruct for Read io_rqst
        STRUCT  WrBrkMp,MP_SIZE         ; MsgPort for write-break handling
        STRUCT  WrBrkSoftInt,IS_SIZE    ; SoftIntStruct for WrBrkMp
        STRUCT  IOWBSTV,IOTV_SIZE       ; IORqst block for Write Break handling
        STRUCT  RdBrkMp,MP_SIZE         ; MsgPort for read-break handling
        STRUCT  RdBrkSoftInt,IS_SIZE    ; SoftIntStruct for RdBrkMp
        STRUCT  IORBSTV,IOTV_SIZE       ; IORqst block for Read Break handling
	ULONG   UNSER_SEGLIST   ; device node segment list
	ULONG   UNSER_OLDRBF    ; previous owner of RBF interrupt vector
	ULONG   UNSER_OLDTBE    ; previous owner of TBE interrupt vector
	ULONG   UNSER_MISC      ; address of misc resource
        ULONG   UNSER_EXTFLAGS  ; additional serial flags (see bitdefs above)
	ULONG   UNSER_PARITY    ; magic constant for parity checks
	ULONG   UNSER_BAUD      ; true baud rate
	ULONG   UNSER_XONTHRESH ; time to xON = 1/2 buffer size
	ULONG   UNSER_XOFFTHRESH ; time to xOFF = 1/4 buffer size
	UWORD   UNSER_SPEED     ; baud rate in clock cycles (not true baud)
	UWORD   UNSER_READBITS  ; read word length (bits masked on)
	UWORD   UNSER_WRITEBITS ; write word length (bits masked on)
	UWORD   UNSER_STOPBITS  ; stopbits for read (bits masked on)
	UWORD   UNSER_STATUS    ; serial port status 
        UBYTE   UNSER_FLAG2     ; see UNSER_FLAG2 bit definitions above
        UBYTE   UNSER_XDATA     ; latest X-on/off char to pass to sender
        UBYTE   UNSER_DDRA      ; status of _ciab+ciaddra saved for restore
        UBYTE   UNSER_PRA       ; status of _ciab+ciapra saved for restore
	UWORD	UNSER_TIMEOUT	; timeout flag for CTS/RTS handshaking
        LABEL   UNSER_SIZE

*--------------------------------------------------------------------
*
* Driver error definitions
*
*--------------------------------------------------------------------

SerErr_DevBusy	        EQU	1
SerErr_BufErr       	EQU	4
SerErr_InvParam        	EQU	5
SerErr_LineErr       	EQU	6
SerErr_ParityErr        EQU     9
SerErr_TimerErr       	EQU    11
SerErr_BufOverflow    	EQU    12
SerErr_NoDSR    	EQU    13
SerErr_DetectedBreak  	EQU    15

*    ENDC   !DEVICES_SERIAL_I
