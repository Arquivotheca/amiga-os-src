**
** $Id: dns.asm,v 1.7 92/03/14 14:41:22 kcd Exp $
**
** nipc.library Domain Name Service
**
** (C) Copyright 1992, Commodore-Amiga, Inc.
**
**
    NOLIST

    include "exec/types.i"
    include "exec/memory.i"
    include "dns.i"

**
** Cross References to library code
**
    xdef    _DoDNS,_DNSInit,_DNSDeinit,_DNSTimer
    xref    _CopyFromBuffer,_FreeBuffer,_DataLength,_OpenUDPPort,_CloseUDPPort
    xref    _UDP_Output,_AbsExecBase

**
** Useful macros
**

jsrlib  MACRO
        xref    _LVO\1
        jsr     _LVO\1(a6)
        ENDM
**
**
** DNS Startup Code
**

_DNSInit:
        movem.l d1/a0-a6,-(sp)
        lea.l   ServerIP(pc),a0
        move.l  d0,(a0)
        lea.l   NIPCLibBase(pc),a0
        lea.l   endtable(pc),a1
        move.l  a1,d1
        move.l  a0,d0
        sub.l   d0,d1
.a      move.b  #0,(a0)+
.b      dbra    d1,.a
        lea.l   NIPCLibBase(pc),a0
        move.l  a6,(a0)
        movea.l _AbsExecBase,a6
        move.l  #MEMF_CLEAR,d1
        move.l  #768+MLH_SIZE+MLH_SIZE+MLH_SIZE,d0
        jsrlib  AllocMem
        lea.l   ServerNameBuff(pc),a0
        move.l  d0,(a0)+
        beq     _DNSDeinit
        add.l   #256,d0
        move.l  d0,(a0)+
        add.l   #256,d0
        move.l  d0,(a0)+
        add.l   #256,d0
        move.l  d0,(a0)+
        add.l   #MLH_SIZE,d0
        move.l  d0,(a0)+
        add.l   #MLH_SIZE,d0
        move.l  d0,(a0)+
        move.l  nri_queue(pc),a0
        bsr.b   _newlist

**
** Set up our UDP port
**
        lea.l   ProcessUDP(pc),a0
        moveq.l #-1,d0
        move.l  a6,-(sp)
        movea.l NIPCLibBase(pc),a6
        jsr     _OpenUDPPort
        lea.l   UDPConn(pc),a0
        move.l  d0,(a0)
        beq     _DNSDeinit

**
** Initialize our message port
**
        movea.l (sp)+,a6
        jsrlib  CreateMsgPort
        lea.l   nri_port(pc),a0
        move.l  d0,(a0)
        beq     _DNSDeinit
        movem.l (sp)+,d1/a0-a6
        rts

_newlist
        NEWLIST a0
        rts

**
**
** Start of SendRequest Routine
**
** void sendrequest(nrn *a0, ip:d0)
**
** Assume everything in the nrn has been set up already.
** Just create a packet and send it off to the server in d0
**

SendRequest:
        movem.l a0-a6/d0-d7,-(sp)
        movea.l a0,a4               ;a4 -> nrn
        move.l  d0,d7               ;d7 = IP Addr
        movea.l _AbsExecBase,a6
        move.l  #512,d0
        move.l  #MEMF_CLEAR,d1
        jsrlib  AllocMem
        tst.l   d0
        bne.b   1$
        movem.l (sp)+,a0-a6/d0-d7
        rts
1$      movea.l d0,a5
        move.w  nrn_id(a4),(a5)     ;Header ID
        move.w  #$100,2(a5)         ;Header Flags = Recursive, please
        move.w  #1,4(a5)            ;QDCOUNT
        move.w  #0,6(a5)            ;ANCOUNT
        move.w  #0,8(a5)            ;NSCOUNT
        move.w  #0,10(a5)           ;ARCOUNT
        lea     12(a5),a1
        lea     nrn_name(a4),a0
        bsr     CopyName

** Now fill in the rest of the question. Stuff like qtype and qclass. For this
** case, these will be A (1) and IN (1). So 68000's don't blow chunks, we'll
** have to stick these 16-bit values in one byte at a time because QNAME doesn't
** use padding to guarrantee word-alignment.

        move.b  #0,(a1)+            ;A  - 32-bit Address (type)
        move.b  #1,(a1)+
        move.b  #0,(a1)+            ;IN - Internet (class)
        move.b  #1,(a1)+

** Now, get the length of this message....

        move.l  a5,d3               ;start of query
        move.l  a1,d0               ;end of query
        sub.l   d3,d0               ;d2 - length of query

** Now we are almost ready to send this off to the nameserver
** "MandHandle" the tcbx's destination IP address

        move.l  a6,-(sp)
        movea.l NIPCLibBase(pc),a6
        move.l  ServerIP(pc),d1
        movea.l UDPConn(pc),a0
        clr.l   d2
        move.w  20(a0),d2
        movea.l a5,a0
        move.l  #53,d3
        jsr     _UDP_Output
        movea.l (sp)+,a6
        movea.l a5,a1
        move.l  #512,d0
        jsrlib  FreeMem
        movem.l (sp)+,a0-a6/d0-d7
        rts

**
** FindNRN_ID
**
** nrn *:d0 (id:d0)
**

FindNRN_ID:
        movem.l d1-d7/a0-a6,-(sp)
        move.l  d0,d7
        clr.l   d3
        movea.l nri_queue(pc),a0
2$      movea.l (a0),a0
        tst.l   (a0)
        beq.b   4$
        cmp.w   #NetRes_NESTED,nrn_state(a0)
        beq     2$
        cmp.w   nrn_id(a0),d7
        bne     2$
        move.l  a0,d3
        bra     4$
4$      move.l  d3,d0
        movem.l (sp)+,a0-a6/d1-d7
        rts


**
** PackName
**
** void PackName (source:a0,dest:a1)
**
** Now we need to stuff the domain name into the message in the way specified
** by RFC 1035.   First, a byte giving the number of characters in this label,
** and then the characters of the label. Note: only 1-63 characters are allowed
** for each label.        Since this in only going to be 1 query, I don't have
** to worry name about compression.
** A0 - Source Name in ascii nnn.nnn.nnn.nnn format
** A1 - Points to the character count
** A2 - Points to where next character goes

PackName:
        movem.l  a0-a2/d0,-(sp)
1$      clr.b   (a1)
        movea.l a1,a2
        add.l   #1,a2
2$      move.b  (a0)+,d0
        beq.b   4$
        cmp.b   #'.',d0
        bne.b   3$
        movea.l a2,a1
        bra     1$
3$      move.b  d0,(a2)+
        add.b   #1,(a1)
        bra     2$
4$      movem.l (sp)+,a0-a2/d0
        rts


**
** CopyName
**
** void CopyName (source:a0,dest:a1)
**
** Copies a name from one place to another.
**

CopyName:
        move.l  a0,-(sp)
1$      move.b  (a0)+,(a1)+
        bne     1$
        movea.l (sp)+,a0
        rts


**
** void ProcessUDP(struct UDPConnection *c, struct Buffer *b)
**
** Get connection * in a2
** Get struct Buffer * in a3
**
**

ProcessUDP:
        movem.l d2-d7/a2-a6,-(sp)
        movea.l a0,a2
        movea.l a1,a3


12$     movea.l NIPCLibBase(pc),a6
        movea.l a3,a0
        jsr     _DataLength
        move.l  d0,d2
        move.l  #MEMF_CLEAR,d1
        move.l  a6,-(sp)
        movea.l _AbsExecBase,a6
        jsrlib  AllocMem
        lea.l   resbuf(pc),a0
        move.l  d0,(a0)
        lea.l   reslen(pc),a0
        move.l  d2,(a0)
        bne.b   PacketOk
        movea.l a3,a0
        movea.l (sp)+,a6
        jsr     _FreeBuffer
        movem.l (sp)+,d2-d7/a2-a6
        rts

PacketOk:
        movea.l d0,a1
        movea.l (sp)+,a6
        movea.l a3,a0
        move.l  d2,d0
        jsr     _CopyFromBuffer
        movea.l a3,a0
        jsr     _FreeBuffer

        movea.l resbuf(pc),a5
        adda.l  #28,a5              ;Kludge. Skip over ip header and UDP header.

** Now let's see what we got back from Mr. NameServer
** a5 will point to the beginning of our incomming buffer.
** Here's what we'll do:
** 1) See if we have a nrn with an ID that matches one that's in our
** queue. If not, free it and check for more data.

121$    clr.l   d0
        move.w  (a5),d0
        bsr     FindNRN_ID
        tst.l   d0
        bne.b   14$

** Well, we got a response back, but not ours...could be anything. Ignore it.
** Or, we just don't like what we got.

13$     movea.l resbuf(pc),a1
        move.l  reslen(pc),d0
        movea.l _AbsExecBase,a6
        jsrlib  FreeMem
        lea.l   resbuf(pc),a0
        clr.l   (a0)+
        clr.l   (a0)
        movem.l (sp)+,d2-d7/a2-a6
        rts

** Well, hopefully this thing will have some useful information for us. Let's
** first check out the flag and status bits.

14$     movea.l d0,a4
        lea.l   12(a5),a0
42$     move.b  (a0)+,d0    ;Skip over the question name
        bne     42$
        adda.l  #4,a0       ;Skip qtype and qclass
        move.w  6(a5),d6    ;Check for answers from the server
        bra     123$
44$     move.b  #$c0,d1     ;skip over answer name
        and.b   (a0),d1
        bne.b   43$
        move.b  (a0)+,d0
        beq.b   45$
        bra     44$
43$     adda.l  #2,a0
45$     clr.l   d0
        bsr     _readword
        cmp.w   #1,d0       ;IP Address?
        beq.b   450$


        cmp.w   #5,d0       ;CNAME reference?
        beq.b   124$
451$
        adda.l  6,a0
        clr.l   d0
        bsr     _readword
        adda.l  d0,a0
        bra     123$

;Okay, find the parent..kill the siblings...replace the original name
;with the one from here...and start all over with a new ID number.
;First save off the cmame info in the RR
;Also, restart the timer for this query.

124$
        adda.l  #8,a0
        movea.l tempname(pc),a1
        movea.l a5,a2
        bsr     GetName
        clr.l   d0
        move.w  (a5),d0
        bsr     FindNRN_ID
        movea.l d0,a4
        movea.l d0,a0
        bsr     KillSiblings
        movea.l a4,a0
        lea.l   nrn_name(a0),a1
        movea.l tempname(pc),a0
        bsr     PackName
        movea.l a4,a0
        lea.l   NextID(pc),a3
        add.w   #1,(a3)
        move.w  NextID(pc),nrn_id(a0)
        move.l  #50,nrn_timer(a0)
        move.l  ServerIP(pc),d0
        bsr     SendRequest
        bra.b   47$
450$
        adda.l  #8,a0                   ;class(2)+ttl(4)+rlen(2)
        lea.l   nrn_ip(a4),a1
        move.b  (a0),(a1)
        move.b  1(a0),1(a1)
        move.b  2(a0),2(a1)
        move.b  3(a0),3(a1)
46$     movea.l a4,a0                   ;Send the NRN Home....
        bsr     ReplyNRN
47$     movea.l resbuf(pc),a1           ;Free the UDP Stuff....
        move.l  reslen(pc),d0
        movea.l _AbsExecBase,a6
        jsrlib  FreeMem
        lea.l   resbuf(pc),a0
        clr.l   (a0)+
        clr.l   (a0)

        movem.l (sp)+,d2-d7/a2-a6
        rts

123$
        dbra    d6,44$

** Ok, now let's see if this thing has any other machines that might know
** what we are looking for.  For right now, let's just try the first one
** that it gives us.  We'll rely on the assumption that the other servers
** have a pretty good idea of who to ask for the info we want.

50$     lea.l   12(a5),a0
        tst.w   10(a5)
        beq     46$                     ;No idea!
51$     move.b  (a0)+,d0                ;skip the qname field
        bne     51$
59$     adda.l  #4,a0
        move.w  8(a5),d2                ;any ns rr's to skip?
        bra.b   55$                     ;do loop
54$     movea.l tempname(pc),a1
        movea.l a5,a2
        bsr     GetName
        move.b  #0,(a1)
53$     adda.l  #8,a0                   ;skip class, type, ttl fields
        clr.l   d0
        bsr     _readword
        adda.l  d0,a0                   ;skip rr's data segment
55$     dbra    d2,54$

** By now we should have a0 pointing to the first additional rr.
** Look for the first one with a type 1 internet record (an address)
** Then, make sure that we aren't sending a request to that address (not fun)
** and then send off a request to that machine.  If we are already sending
**a request there, our search has most likely failed so we can stop looking.

        move.w  10(a5),d7
        movea.l a5,a2                   ;a2 -> start of udp segment
        bra.b   67$                     ;start search
60$     movea.l tempname(pc),a1
        bsr     GetName                 ;for later.....
        move.b  #0,(a1)                 ;null-terminate it
        clr.l   d0
        bsr     _readword
        cmp.w   #1,d0                   ;type = A ?
        beq.b   64$
        adda.l  #2,a0                   ;skip class
        bra.b   65$
64$     clr.l   d0
        bsr     _readword
        cmp.w   #1,d0                   ;Class = Internet?
        beq.b   90$
65$     adda.l  #4,a0                   ;skip TTL
        clr.l   d0
        bsr     _readword
        adda.l  d0,a0                   ;skip rr data field
67$     dbra    d7,60$

** No addresses found!!!!!
** Find the parent nrn and Reply to it with an error condition.

68$     clr.l   d0
        move.w  (a5),d0
        bsr     FindNRN_ID      ;This HAS to succeed.
        movea.l d0,a0
70$     clr.l   nrn_ip(a0)
        bsr     ReplyNRN        ;This one will be de-allocated
                                ;by the caller
        bsr     KillSiblings    ;kill off any involved nrn's NOW!
        bra     47$             ;start ALL OVER

;Get the IP Address

90$     adda.l  #6,a0           ;skip TTL and Rlen field
        bsr.s   _readlong
        move.l  d0,d7

;look for any matching nested nrn's.

        movea.l nri_queue(pc),a2
        move.w  (a5),d0
91$     movea.l (a2),a2
        tst.l   (a2)
        beq.b   100$            ;no match found
        cmp.l   nrn_ip(a2),d7
        bne     91$
        cmp.w   nrn_id(a2),d0
        bne     91$             ;so we don't interfere with other searches!

;A match was found which means we've gotten ourselves into a circular
;lookup.  Kill off the parent of this thing and all of it's children.

        movea.l a2,a0
        bra     70$

;Okay, fabricate an appropriate dummy nrn and resend off a request.
;Note: Dummy nrn's DON'T have their own ID numbers.  They only
;serve as "placeholders" for keeping track of who we've sent out requests
;to for the parent (so we don't get into a loop).
;They have the same ID number as their parent. FindNRN won't find them, because
;it skips these types of NRN's.

100$
        move.l  #nrn_SIZE,d0
        move.l  #MEMF_CLEAR,d1
        movea.l _AbsExecBase,a6
        jsrlib  AllocMem
        tst.l   d0
        beq     68$             ;Error...

        movea.l d0,a4           ;a4 -> dummy nrn
        move.l  d7,nrn_ip(a4)
        move.w  #NetRes_NESTED,nrn_state(a4) ;dummy placeholder
        clr.l   d0
        move.w  (a5),d0
        move.w  d0,nrn_id(a4)
        bsr     FindNRN_ID
        move.l  d0,nrn_parent(a4)
        movea.l a4,a1
        movea.l nri_queue(pc),a0
        ADDTAIL                         ;it's in the queue
        movea.l nrn_parent(a4),a0
        move.l  d7,d0
        bsr     SendRequest
        bra     47$

_readlong:
        move.b  (a0)+,d0
        lsl.l   #8,d0
        move.b  (a0)+,d0
        lsl.l   #8,d0

_readword:
        move.b  (a0)+,d0
        lsl.l    #8,d0
        move.b  (a0)+,d0
        rts


**
** Start of KillSiblings routine
**
** void KillSiblings(nrn *:a0)
** searches through our list of nrn's, killing off any that had a0 as
** their parents
**

KillSiblings:
        movem.l d0-d7/a0-a6,-(sp)
        move.l  a0,d7
        movea.l nri_queue(pc),a3
1$      move.l  (a3),a3
        tst.l   (a3)
        beq.b   .ksdone
        cmp.l   nrn_parent(a3),d7
        bne     1$
        movea.l a3,a1
        REMOVE
        movea.l a3,a1
        move.l  (a3),-(sp)
        movea.l _AbsExecBase,a6
        move.l  #nrn_SIZE,d0
        jsrlib  FreeMem
        movea.l (sp)+,a3
        bra     1$

.ksdone:
        movem.l (sp)+,d0-d7/a0-a6
        rts


**
** Start of ReplyNRN
**

ReplyNRN:
        movem.l d0-d7/a0-a6,-(sp)
        movea.l a0,a4
        movea.l a0,a1
        REMOVE
        movea.l a4,a1
        movea.l _AbsExecBase,a6
        jsrlib  ReplyMsg
        movem.l (sp)+,d0-d7/a0-a6
        rts


**
** ServicePublic
**

_DoDNS:
        movem.l d0-d7/a0-a6,-(sp)
        lea.l   NIPCLibBase(pc),a0
        move.l  a6,(a0)
1$      movea.l nri_port(pc),a0
        movea.l _AbsExecBase,a6
        jsrlib  GetMsg
        tst.l   d0
        bne.b   2$
        movem.l (sp)+,d0-d7/a0-a6
        rts

** We have a request.
** Fire off a request to the nameserver in ServerIP

2$      movea.l d0,a0
        clr.l   nrn_ip(a0)
        lea.l   NextID(pc),a3
        add.w   #1,(a3)
        move.w  NextID(pc),nrn_id(a0)
        move.l  #50,nrn_timer(a0)
        move.w  #NetRes_NORMAL,nrn_state(a0)
        move.l  a0,-(sp)
        lea.l   nrn_name(a0),a1
        lea.l   nrn_cname(a0),a0
        bsr     PackName
        movea.l (sp)+,a0
        move.l  ServerIP(pc),d0
        bsr     SendRequest
        movea.l a0,a1
        movea.l nri_queue(pc),a0
        ADDTAIL
        bra     1$


**
** ServiceTimer
**
** Gets the IO_REQUEST back from the timer.device and re-queues it for
** another second of delay.
** This routine then steps through the nrn_queue list and decrements the
** timer value in each one.  If the value counts down to zero, that
** NRN is replied with an error and all of it's siblings are killed off.
**

_DNSTimer:
        movem.l d2-d7/a2-a6,-(sp)
        movea.l nri_queue(pc),a0
1$      movea.l (a0),a0
        tst.l   (a0)
        beq.b   .stend
        cmp.w   #NetRes_NORMAL,nrn_state(a0)
        bne     1$
        sub.l   #1,nrn_timer(a0)
        bne     1$
        move.l  #0,nrn_ip(a0)
        bsr     ReplyNRN
        bsr     KillSiblings
        bra     1$
.stend:
        movem.l (sp)+,d2-d7/a2-a6
        rts

**
** Start of CleanExit routine
**
** Exit no matter what state we're in.
** First, check and see if we need shut off the outside world by
** closing our tcbx and removing our public message ports.  Then, reply
** to any Pending NRN's and then free any info in either the nc cache
** or the server cache.
** Then close the net.library, dos.libary, timer.device and exit.
** Note: we needn't ever save any registers.
**

_DNSDeinit:
        movem.l a2-a6/d2-d7,-(sp)
        move.l  UDPConn(pc),d0
        beq.b   .closeport
        movea.l d0,a0
        movem.l a6,-(sp)
        movea.l NIPCLibBase(pc),a6
        jsr     _CloseUDPPort
        movem.l (sp)+,a6

.closeport
        movea.l _AbsExecBase,a6
        move.l  nri_port(pc),d0
        beq.b   .flushpending
1$      movea.l nri_port(pc),a0
        jsrlib  GetMsg
        tst.l   d0
        beq.b   .deleteport
        movea.l d0,a1
        move.l  #0,nrn_ip(a1)
        jsrlib  ReplyMsg
        bra     1$

.deleteport
        movea.l nri_port(pc),a0
        jsrlib  DeleteMsgPort

.flushpending
        move.l  nri_queue(pc),d0
        beq.b   checkres
3$      movea.l nri_queue(pc),a0
        cmp.l   LH_TAIL+LN_PRED(a0),a0
        beq.b   checkres
        movea.l (a0),a0
        move.l  #0,nrn_ip(a0)
        cmp.w   #NetRes_NESTED,nrn_state(a0)
        beq.b   2$
        bsr     ReplyNRN
        bra     3$
2$      movea.l a0,a1
        move.l  a0,-(sp)
        REMOVE
        movea.l (sp)+,a1
        move.l  #nrn_SIZE,d0
        jsrlib  FreeMem
        bra     3$

checkres:
        lea.l   resbuf(pc),a5
        tst.l   (a5)
        beq.b   .freeextra
        movea.l (a5),a1
        move.l  reslen(pc),d0
        jsrlib  FreeMem

.freeextra
        move.l  ServerNameBuff(pc),d0
        beq.b   .done
        movea.l d0,a1
        move.l  #768+MLH_SIZE+MLH_SIZE+MLH_SIZE,d0
        jsrlib  FreeMem

.done
        movem.l (sp)+,d2-d7/a2-a6
        rts

str1    dc.b    '**FlushPending**',10,0
        cnop    0,2
str2    dc.b    '**CheckRes**',10,0
        cnop    0,2

**
** GetName Subroutine.  Called with a0 pointing to the current position in
** our buffer, a1 pointing to the current place in the output buffer, and
** a2 pointing to the message from the server.
** I decided to make this routine be recursive so that it can call itself
** in case it runs into name compression.
**

GetName:
        clr.l   d1
1$      move.b  (a0),d1
        and.b   #$c0,d1
        beq.b   2$
        clr.l   d1
        move.b  (a0)+,d1
        and.b   #$3f,d1
        lsl.l   #8,d1
        move.b  (a0)+,d1
        move.l  a0,-(sp)
        lea.l   0(a2,d1.l),a0
        bsr     GetName
        movea.l (sp)+,a0
        rts
2$      move.b  (a0)+,d1
        beq.b   4$
3$      move.b  (a0)+,(a1)+
        addq.l  #1,d0
        subq.l  #1,d1
        bne     3$
        move.b  #'.',(a1)+
        addq.l  #1,d0
        bra     1$
4$      rts

**
** Variable Space
**

ServerIP        ds.l    1
NIPCLibBase     ds.l    1
UDPConn         ds.l    1
resbuf          ds.l    1
reslen          ds.l    1
outhandle       ds.l    1
WaitMask        ds.l    1
UDPMask         ds.l    1
TimerMask       ds.l    1
PublicMask      ds.l    1
BreakMask       ds.l    1
NextID          ds.w    1
HexBuf          ds.l    4
ServerNameBuff  ds.l    1
tempname        ds.l    1
ServerBuff      ds.l    1
nri_queue       ds.l    1
nri_cache       ds.l    1
nri_servers     ds.l    1
nri_port        ds.l    1
nri_timerport   ds.l    1
nri_timerio     ds.l    1
endtable        ds.b    1
