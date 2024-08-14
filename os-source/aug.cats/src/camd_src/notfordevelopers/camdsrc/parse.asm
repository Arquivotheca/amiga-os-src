*************************************************************************
*     C. A. M. D.       (Commodore Amiga MIDI Driver)                   *
*************************************************************************
*                                                                       *
* Design & Development  - Roger B. Dannenberg                           *
*                       - Jean-Christophe Dhellemmes                    *
*                       - Bill Barton                                   *
*                       - Darius Taghavy                                *
*                                                                       *
* Copyright 1990 by Commodore Business Machines                         *
*                                                                       *
*************************************************************************
*
* parse.asm   - MIDI Parser
*
*************************************************************************

; This file is based on a combination of the original CAMD parser and
; the MIDI Library parser.
;
; This is really two parsers in one:
;       ParseMidiTime - private parser called by unit receive handler task
;                       w/ time stamp bytes intermingled with data.
;       ParseMidi     - public parser.

; Notes
;   1.  EOX messages are never propogated to PutMidiToPort/IPutMidi()
;       even when they aren't attached to a sys/ex message.


      nolist
        include "exec/types.i"
        include "exec/macros.i"
        include "exec/memory.i"
        include "midi/mididefs.i"

        include "camdlib.i"
      list


; DEBUG set 1

        idnt    parsea.asm

            ; public
        xdef    ParseMidi

            ; internal
        xdef    _AllocParser
        xdef    _FreeParser
        xdef    ParseMidiTime

            ; external
        xref    MidiMsgLen
        xref    IPutMidi
        xref    PutSysExList
        xref    SendErrToMI

        section __MERGED,DATA
        xref    _CamdBase
        CODE

    ifd DEBUG
            ; debug
        xdef    ClearParser         ; this might become visible to unita.asm
        xdef    InitParser
        xdef    newsysexnode
        xdef    procsysex
        xdef    sysexerror
        xdef    sendsysex
    endc


; Algorithm
; ---------
;
;   Parse(mi,buffer,len)
;   {
;       if (in sys/ex) sysex();
;
;       while (get byte) {
;
;           if (status) {
;               if (real time) send real time
;               else {
;                   set running status
;                   if (sysex) sysex();
;                   else {
;                       get length
;                       if (no data) goto send
;                       set up ThreeByte
;                   }
;               }
;           }
;           else {
;               if (running status) {
;                   if (!MsgTime) set msg time
;                   move databyte into proper place in buffer
;                   if (no more databytes) {
;                send:
;                       send msg
;                       clear time
;                       if (status >= $f0) clear running status
;                   }
;               }
;           }
;       }
;   }
;
;   sys/ex()
;   {
;       while (getbyte) {
;           if (status byte) {
;               if (realtime) {
;                   get time
;                   SendRealTime
;               }
;               else {
;                   handle ending
;                   if (!EOX) set pointer back one byte
;               }
;           }
;           else {
;               if (no buffer space) allocate more
;               post byte
;           }
;       }
;   }
;
;
;
;   ParseTime(mi,byte)
;   {
;       if (status byte) {
;           if (real time) send real time
;           else {
;               if (running == sys/ex) {
;                   send sys/ex
;                   if (byte == EOX) goto done
;               }
;               set running status & time
;               if (!sysex) {
;                   get length
;                   if (no data) goto send
;                   set up ThreeByte
;               }
;               else {
;                   setup for sys/ex
;               }
;           }
;       }
;       else {
;           if (running status) {
;               if (status == sys/ex) {
;                   if (no buffer space) allocate more
;                   post byte
;               }
;               else {
;                   if (!MsgTime) set msg time
;                   move databyte into proper place in buffer
;                   if (no more databytes) {
;                send:
;                       send msg
;                       clear time
;                       if (status >= $f0) clear running status
;                   }
;               }
;           }
;       }
;     done:
;   }
;
;
; Notes
; -----
;   1. ThreeByte usage:
;           $0000 0000 - 1 data byte
;           $0001 0000 - 2 data bytes, next byte is 1st data byte
;           $0000 0001 - 2 data bytes, next byte is 2nd data byte


; common registers

pdr     equr    a3      ; A3 = ParserData pointer


****************************************************************
*
*   AllocParser
*
*   SYNPOSIS
*       struct ParserData *AllocParser (void)
*
*   FUNCTION
*       Allocate a ParserData structure.
*
*   INPUTS
*       None
*
*   RESULTS
*       Pointer to initialized ParserData structure.
*
****************************************************************

_AllocParser    ; C addressable symbol
        Push    pdr/a5-a6

        move.l  _CamdBase,a5                ; A5 = CamdBase
        move.l  camd_SysBase(a5),a6         ; A6 = SysBase

        move.l  #ParserData_Size,d0         ; allocate a ParserData structure
        move.l  #MEMF_CLEAR!MEMF_PUBLIC,d1
        JSRLIB  AllocMem
        tst.l   d0
        beq.s   ap_ret

        move.l  d0,pdr
        bsr     InitParser

        move.l  pdr,d0                      ; set return value

ap_ret
        Pop
        rts


****************************************************************
*
*   FreeParser
*
*   SYNPOSIS
*       void FreeParser (struct ParserData *pd)
*                               a0
*
*   FUNCTION
*       Free a ParserData structure.
*
*   INPUTS
*       pd - pointer to ParserData to free.
*
*   RESULTS
*       None
*
****************************************************************

_FreeParser     ; C addressable symbol
        Push    pdr/a5-a6

        move.l  a0,pdr
        move.l  _CamdBase,a5                ; A5 = CamdBase
        move.l  camd_SysBase(a5),a6         ; A6 = SysBase

        bsr     ClearParser

        move.l  pdr,a1
        move.l  #ParserData_Size,d0         ; allocate a ParserData structure
        JSRLIB  FreeMem

        Pop
        rts


****************************************************************
*
*   ParseMidi
*
****************************************************************

; all autodoc'd functions contain A6 = CamdBase

        ifne 0
/****** camd.library/ParseMidi ******************************************
*
*   NAME
*       ParseMidi -- Parse a stream of bytes as MIDI messages.
*
*   SYNOPSIS
*       void ParseMidi (struct MidiLink *ml, cosnt UBYTE *Buffer,
*                             a0                 a1
*
*                   ULONG Length)
*                         d0
*
*   FUNCTION
*       Parses an unformated stream of bytes as MIDI data and transmits the
*       resulting messages to mi->SendPort.
*
*       AddMidiParser() must be called prior to calling ParseMidi() in order
*       to attach the necessary parser data to the MidiLink.
*
*   INPUTS
*       mi         - a MidiLink to send on.
*       Buffer - Pointer to buffer containing MIDI data to send.
*       Length - Number of bytes in buffer to send.
*
*   RESULTS
*       None
*
*   SEE ALSO
*       AddMidiParser()
****************************************************************************
*
*/
        endc

; Register Usage
; --------------
; D2 - End of source buffer
; D3 - "Three Byte" register
; D4 - Saved MidiLink pointer
; A2 - Source buffer pointer
; A3 - ParserData pointer (pdr)
; A5 - SysBase (note these are reveresed)
; A6 - CamdBase


ParseMidi

;REM -- re-enable this code later

        Push    pdr/d2-d4/a2/a5
        move.l  camd_SysBase(a6),a5         ; A5 = SysBase

        move.l  ml_ParserData(a0),d1
        beq.s   pm_ret                      ; maybe some error handling here?
        move.l  d1,pdr                      ; pdr = ParserData
        move.l  a0,d4                       ; preserve MidiLink ptr

        move.l  a1,a2                       ; A2 = current pos in buffer
        move.l  a2,d2
        add.l   d0,d2                       ; D2 = pointer to byte after end of buffer
        move.l  pd_ThreeByte(pdr),d3        ; D3 = "ThreeByte" register

        cmp.b   #MS_SysEx,pd_CurMsg+mm_Status(pdr)  ; in the middle of a sys/ex?
        beq.s   pm_sysex

pm_loop
        cmp.l   a2,d2                       ; at end of buffer?
        bls.s   pm_ret

        move.b  (a2)+,d0                    ; D0 = midi byte
        bpl.s   pm_databyte                 ; a data byte?

pm_statusbyte                  ; status byte
        cmp.b   #MS_RealTime,d0             ; is real time?
        bcs.s   pm_nonrt

pm_realtime                    ; real time
        move.b  d0,pd_RTMsg+mm_Status(pdr)  ; create RTMsg
        move.l  pd_RTMsg+mm_Msg(pdr),d0
        move.l  d4,a1                       ; MidiLink
        bsr.s   PutParsedMidi
        bra     pm_loop

pm_nonrt
        cmp.b   #MS_EOX,d0                  ; ignore EOX's
        beq.s   pm_clear

        move.b  d0,pd_CurMsg+mm_Status(pdr)
        cmp.b   #MS_SysEx,d0                ; is sys/ex?
        beq.s   pm_sysex

pm_stdmsg                      ; ordinary messages: 0-3 bytes in length
        bsr     MidiMsgLen                  ; get length (must be 0-3) (clears upper word, too)
        subq.w  #2,d0
        bcs.s   pm_sendmsg                  ; if length < 2, send immediately (no data bytes)

        move.l  d0,d3                       ; set ThreeByte register: D3 = 0 if length == 2, D3 = 1 if length == 3
        swap    d3                          ; toggle 3rd byte flag so it's set to 2nd byte
        bra     pm_loop

pm_sysex                        ; sys/ex
        bsr.s   procsysex
        bra     pm_loop

pm_databyte                     ; data byte
        tst.b   pd_CurMsg+mm_Status(pdr)    ; running status?
        beq     pm_loop

        move.b  d0,pd_CurMsg+mm_Data1(pdr,d3.w) ; place 2nd or 3rd byte depending on d3.w
        swap    d3                          ; swap register halves (0<->1)
        tst.w   d3                          ; if this is non-zero, there's another byte to load for this message
        bne     pm_loop

pm_sendmsg                      ; send msg
        move.l  pd_CurMsg(pdr),d0           ; D0 = Msg
        move.l  d4,a1                       ; MidiLink
        bsr.s   PutParsedMidi
        cmp.b   #MS_System,pd_CurMsg+mm_Status(pdr) ; system msg?
        bcs     pm_loop
pm_clear
        clr.b   pd_CurMsg+mm_Status(pdr)    ; if so, clear running status
        bra     pm_loop

pm_ret                          ; exit routine
        move.l  d3,pd_ThreeByte(pdr)        ; save contents of ThreeByte register
        Pop

        rts

PutParsedMidi           ; d0 = msg, a1 = MidiLink
        exg.l   a6,a5                       ; A5 = CamdBase, A6 = SysBase

        clr.l   -(sp)                       ; push timeless value as mm_Time
        move.l  d0,-(sp)                    ; push mm_Msg
        move.l  sp,a0                       ; A0 = MidiMsg pointer
        bsr     IPutMidi
        addq.w  #8,sp                       ; pop MidiMsg off stack

        exg.l   a5,a6
        rts

PutParsedMidiE          ; d0 = msg, a1 = MidiLink, CamdBase & ExecBase OK

        clr.l   -(sp)                       ; push timeless value as mm_Time
        move.l  d0,-(sp)                    ; push mm_Msg
        move.l  sp,a0                       ; A0 = MidiMsg pointer
        bsr     IPutMidi
        addq.w  #8,sp                       ; pop MidiMsg off stack

        rts

****************************************************************
*
*   procsysex
*
*   FUNCTION
*       Process sys/ex stream (a bit optimized)
*
*   INPUTS
*       A2 - source buffer
*       D2 - end of source buffer
*       D4 - MidiLink
*       A3 - ParserData (pdr)
*       A5 - SysBase
*       A6 - CamdBase
*
*   RESULTS
*       A2 - new current position in buffer
*
****************************************************************

procsysex
        Push    a4
        exg.l   a5,a6                       ; A5=CamdBase, A6=SysBase
        move.l  pd_SXBCur(pdr),a4           ; A4 = sys/ex buffer pointer

psx_loop
        cmp.l   a2,d2                       ; at end of buffer?
        bls.s   psx_empty

        move.b  (a2)+,d0                    ; get data byte
        bmi.s   psx_statusbyte              ; a status byte?

psx_databyte                    ; data byte
        cmp.l   pd_SXBEnd(pdr),a4
        bcc.s   psx_newnode
        move.b  d0,(a4)+
        bra     psx_loop

psx_newnode
        bsr     newsysexnode                ; get a new ParserSXNode
        tst.l   d0
        beq.s   psx_err

        move.l  d0,a4                       ; return value is next byte to fill
        move.b  -1(a2),(a4)+
        bra     psx_loop

psx_statusbyte
        cmp.b   #MS_RealTime,d0             ; is real time?
        bcs.s   psx_send

psx_realtime                    ; real time
        move.b  d0,pd_RTMsg+mm_Status(pdr)  ; create RTMsg
        move.l  pd_RTMsg+mm_Msg(pdr),d0
        bsr     PutParsedMidiE
        bra     psx_loop

psx_send
        cmp.b   #MS_EOX,d0
        beq.s   psx_eox
        subq.w  #1,a2                       ; move source buffer pointer back to previous byte if non-EOX

psx_eox
        move.l  a4,pd_SXBCur(pdr)           ; save A4
        clr.l   pd_CurMsg+mm_Time(pdr)      ; timeless value for mm_Time
        bsr     sendsysex
        bra.s   psx_ret

psx_err             ; error
        bsr     sysexerror
        bra.s   psx_ret

psx_empty           ; end of source buffer reached
        move.l  a4,pd_SXBCur(pdr)           ; save A4

psx_ret
        exg.l   a5,a6
        Pop
        rts


****************************************************************
*
*   InMsgTime (macro)
*
*   SYNOPSIS
*       InMsgTime src,dst
*
*   FUNCTION
*       Expand 8 bit time stamp from queue to 32 bit time stamp.
*
*   INPUTS
*       src - Data register containing 8 bit time stamp.
*       dst - Output data register.
*       A5 - CamdBase
*
*   RESULTS
*       dst - resulting 32 bit time stamp.
*
*   NOTE
*       Preserves ALL registers except dst.
*
****************************************************************

InMsgTime macro ; src,dst   (doesn't damage src)
	moveq	#0,\2
;        move.l  camd_Time(a5),\2            ; get current time
;        cmp.b   \1,\2                       ; if lo byte of time < stamp byte, low 8 bits of time have wrapped since byte was received
;        bcc.s   imt\@
;        sub.l   #$100,\2
;imt\@
;        move.b  \1,\2
        endm


****************************************************************
*
*   ParseMidiTime
*
*   FUNCTION
*       Parse a time-stamped MIDI Byte from a unit.
*
*   INPUTS
*       D0.w - Time stamp in upper byte, MIDI byte in lower byte
*       A0   - MidiParser
*       A1   - MidiLink
*       A5   - CamdBase
*       A6   - SysBase
*
*   RESULTS
*       none
*
****************************************************************

ParseMidiTime
        cmp.b   #MS_RealTime,d0             ; is real time?
        bcs.s   pmt_nonrt

pmt_realtime                    ; real time
        lea     pd_RTMsg(a0),a0             ; A0 = pd_RTMsg
        move.b  d0,mm_Status(a0)
        move.w  d0,-(sp)                    ; shift 8 bits right
        move.b  (sp)+,d0                    ; D0 = time stamp
        InMsgTime d0,d1                     ; compute entire time stamp
        move.l  d1,mm_Time(a0)              ; set time stamp
        bra     IPutMidi                ; REM: what about MidiLink in a1?

pmt_nonrt
        Push    pdr/d4
        move.l  a0,pdr                      ; pdr = MidiParser
        move.l  a1,d4                       ; save MidiLink in d4

        tst.b   d0
        bpl.s   pmt_databyte                ; a data byte?

pmt_statusbyte                  ; status byte
        cmp.b   #MS_SysEx,pd_CurMsg+mm_Status(pdr)  ; in sys/ex?
        bne.s   pmt_notinsysex

        move.w  d0,-(sp)                    ; preserve D0 for stuff below (only need word)
        bsr     sendsysex                   ; send sys/ex
        move.w  (sp)+,d0                    ; restore D0

pmt_notinsysex
        cmp.b   #MS_EOX,d0                  ; ignore any and all EOX's (don't let PutMidiMsg() send an error because of it)
        beq     pmt_clear

        move.b  d0,pd_CurMsg+mm_Status(pdr) ; save status byte in CurMsg

        move.w  d0,-(sp)                    ; shift 8 bits right
        move.b  (sp)+,d0
        InMsgTime d0,d1                     ; compute entire time stamp
        move.l  d1,pd_CurMsg+mm_Time(pdr)   ; save time in CurMsg

        move.b  pd_CurMsg+mm_Status(pdr),d0 ; get status byte
        cmp.b   #MS_SysEx,d0                ; is sys/ex?
        beq     pmt_ret

pmt_stdmsg                      ; ordinary messages: 0-3 bytes in length
        bsr     MidiMsgLen                  ; get length (must be 0-3) (clears upper word, too)
                                            ; (doesn't need A6=CamdBase)
        subq.w  #2,d0
        bcs.s   pmt_sendmsg                 ; if length < 2, send immediately (no data bytes)

        swap    d0                          ; set ThreeByte register: D0 = 0 if length == 2, D0 = 1 if length == 3
        move.l  d0,pd_ThreeByte(pdr)
        bra     pmt_ret

pmt_databyte                    ; data byte
        tst.b   pd_CurMsg+mm_Status(pdr)    ; running status?
        beq     pmt_ret                     ; --BB fixed out of range short branch 03 May 1992

        cmp.b   #MS_SysEx,pd_CurMsg+mm_Status(pdr)  ; in sys/ex?
        beq.s   pmt_sysexdata

        move.l  pd_ThreeByte(pdr),d1
        move.b  d0,pd_CurMsg+mm_Data1(pdr,d1.w)     ; place 2nd or 3rd byte depending on d1.w
        swap    d1                          ; swap register halves (0<->1)
        move.l  d1,pd_ThreeByte(pdr)

        tst.l   pd_CurMsg+mm_Time(pdr)      ; check time
        bne.s   pmt_timeset
        move.w  d0,-(sp)                    ; shift 8 bits right
        move.b  (sp)+,d0
        InMsgTime d0,d1                     ; compute entire time stamp
        move.l  d1,pd_CurMsg+mm_Time(pdr)

pmt_timeset
        tst.w   pd_ThreeByte+2(pdr)         ; if this is non-zero, there's another byte to load for this message
        bne.s   pmt_ret

pmt_sendmsg                     ; send msg
        lea     pd_CurMsg(pdr),a0           ; A0 = message pointer
        move.l  d4,a1
        bsr     IPutMidi                ; REM: what about MidiLink in a1?

        CLEAR   d0
        move.l  d0,pd_CurMsg+mm_Time(pdr)   ; clear time so it'll get set for next msg

        cmp.b   #MS_System,pd_CurMsg+mm_Status(pdr) ; system msg?
        bcs.s   pmt_ret
        clr.b   pd_CurMsg+mm_Status(pdr)    ; if so, clear running status
        bra.s   pmt_ret

pmt_clear
        CLEAR   d0
        move.l  d0,pd_CurMsg+mm_Time(pdr)   ; clear time
        move.b  d0,pd_CurMsg+mm_Status(pdr) ; clear running status
        bra.s   pmt_ret

pmt_sysexdata
        move.l  pd_SXBCur(pdr),a1
        cmp.l   pd_SXBEnd(pdr),a1
        bcs.s   pmt_postsxdata

        move.b  d0,pd_CurMsg+mm_Data1(pdr)  ; save data byte in CurMsg data byte 1 (D0 gets zapped)

        bsr.s   newsysexnode                ; get a new ParserSXNode
        tst.l   d0
        beq.s   pmt_sxerr

        move.l  d0,a1                       ; return value is new BCur
        move.b  pd_CurMsg+mm_Data1(pdr),d0  ; recover data byte

pmt_postsxdata
        move.b  d0,(a1)+                    ; write byte
        move.l  a1,pd_SXBCur(pdr)           ; save BCur
        bra.s   pmt_ret

pmt_sxerr
        bsr.s   sysexerror

pmt_ret                         ; exit routine
        Pop
        rts



****************************************************************
*
*   Shared Sys/Ex stuff
*
****************************************************************


****************************************************************
*
*   newsysexnode
*
*   FUNCTION
*       Allocate a new ParserSXNode, link into SXList, set
*       buffer pointers.
*
*   INPUTS
*       A3 - ParserData (pdr)
*       A6 - SysBase
*
*   RESULTS
*       D0 - pointer to next byte to fill (sxl_BCur) or NULL on
*            failure
*       sxl_BCur & sxl_BEnd are set to point to new buffer
*
****************************************************************

newsysexnode
        Push
        move.l  #ParserSXNode_Size,d0
        move.l  #MEMF_PUBLIC,d1
        JSRLIB  AllocMem

        tst.l   d0
        beq.s   nsx_ret

        move.l  d0,a1                       ; A1 = ParserSXNode
        lea     psxn_Buffer(a1),a0          ; A0 = buffer
        move.l  a0,pd_SXBuff(pdr)           ; set Buff
        move.l  a0,pd_SXBCur(pdr)           ; set BCur
        lea     psxn_BufferEnd(a1),a0
        move.l  a0,pd_SXBEnd(pdr)           ; set BEnd

        lea     pd_SXList(pdr),a0           ; add ParserSXNode to list
        ADDTAIL
        add.w   #1,pd_NSXNodes(pdr)         ; increment node count

        move.l  pd_SXBCur(pdr),d0           ; set return value

nsx_ret
        Pop
        rts



****************************************************************
*
*   sendsysex
*
*   FUNCTION
*       Send Sys/Ex from SXList
*
*   INPUTS
*       A3 - ParserData (pdr)
*       A5 - CamdBase
*       A6 - SysBase
*       D4 - MidiList
*
*   RESULTS
*       none
*
****************************************************************

sendsysex
            ; tack EOX into buffer before sending
        move.l  pd_SXBCur(pdr),a1
        cmp.l   pd_SXBEnd(pdr),a1
        bcs.s   ssx_posteox

        bsr     newsysexnode            ; get a new ParserSXNode
        tst.l   d0
        beq.s   sysexerror

        move.l  d0,a1                   ; return value is new SXBCur

ssx_posteox
        move.b  #MS_EOX,(a1)+
        move.l  a1,pd_SXBCur(pdr)

                                        ; copy 1st 2 data bytes to CurMsg buffer
        move.b  pd_SXShortBuffer+1(pdr),pd_CurMsg+mm_Data1(pdr)
        move.b  pd_SXShortBuffer+2(pdr),pd_CurMsg+mm_Data2(pdr)

        bsr     PutSysExList
        bra.s   ClearParser             ; jump to ClearParser


****************************************************************
*
*   sysexerror
*
*   FUNCTION
*       Deal w/ a sys/ex error
*
*   INPUTS
*       A3 - ParserData (pdr)
*       A5 - CamdBase
*       A6 - SysBase
*       D4 - MidiLink
*
*   RESULTS
*       none
*
****************************************************************

sysexerror
        Push    a2

        moveq   #CMEF_ParseMem,d1

        move.l  d4,a2
        move.l  ml_MidiNode(a2),a2
        bsr     SendErrToMI

        Pop

        ; fall thru to ClearParser


****************************************************************
*
*   ClearParser
*
*   FUNCTION
*       Free ParserSXNodes, call InitParser.
*
*   INPUTS
*       A3 - ParserData (pdr)
*       A6 - SysBase
*
*   RESULTS
*       none
*
****************************************************************

ClearParser
        Push    d2

        move.l  pd_SXList(pdr),d2           ; free ParserSXNode's
cp_loop
        NEXTNODE.s d2,a1,cp_done            ; D2 = succ, A1 = node
        move.l  #ParserSXNode_Size,d0
        JSRLIB  FreeMem
        bra     cp_loop

cp_done
        Pop
        ; fall thru to InitParser


****************************************************************
*
*   InitParser
*
*   FUNCTION
*       Initialize ParserData SysExList and CurMsg.
*
*   INPUTS
*       A3 - ParserData (pdr)
*
*   RESULTS
*       none
*
****************************************************************

InitParser
        Push

        CLEAR   d0                          ; clear CurMsg
        move.b  d0,pd_CurMsg+mm_Status(pdr)
        move.l  d0,pd_CurMsg+mm_Time(pdr)

        lea     pd_SXList(pdr),a0           ; A0 = pd_SXList
        NEWLIST a0
        lea     pd_SXShortBuffer(pdr),a1    ; A1 = buffer pointer
        move.l  a1,pd_SXBuff(pdr)           ; set buffer pointer
        move.b  #MS_SysEx,(a1)+             ; first byte in buffer is SysEx status byte
        move.l  a1,pd_SXBCur(pdr)           ; cur = 1st data byte
        lea     pd_SXShortBufferEnd(pdr),a1 ; A1 = end of buffer
        move.l  a1,pd_SXBEnd(pdr)
        clr.w   pd_NSXNodes(pdr)            ; clear node count

        Pop
        rts

        end
