
*
* setreqtexta.asm --- 10/88 cas
*    Position independent code part of AutoRequest wedge
*
* AutoRequest(Window,Body,PText,NText,PFlag,NFlag,W,H)
*               a0    a1   a2    a3    d0    d1  d2 d3
*
   INCLUDE   "exec/types.i"
   INCLUDE   "exec/execbase.i"
   INCLUDE   "exec/tasks.i"
   INCLUDE   "libraries/dosextens.i"
   INCLUDE   "intuition/intuition.i"

ABSEXECBASE EQU 4

   XREF   _LVORawDoFmt
   XREF   _LVOSupervisor
   XREF   _LVOWait

   XDEF   _startFix
   XDEF   _endFix
   XDEF   _textPtrs
   XDEF   _useCount


EXNUM    EQU  60
USPC00   EQU  66

  STRUCTURE StkVars,0
    LONG olda0
    LONG olda1
    LONG olda2
    LONG olda3
    LONG oldd0
    LONG sab1
    LONG sab2
    LONG sab3
    LONG sapg
    LONG sang
    LONG saw
    LONG tmpa0
    LONG tmpa5
    LONG tmpa6
    LONG tmpd1
    LONG bug
    LABEL sv_SIZEOF 

* Works for 1.2, 1.3
* KLUDGE1 is how far back (+) on this stack I find crashed guy's SP
* KLUDGE2 is what I must add to found SP for stuff DOS pushed on
KLUDGE1  EQU  sv_SIZEOF+104
KLUDGE2  EQU  68

    SECTION CODE

* Position Critical on these first items

_startFix:

_oldAutoRequest  DC.L   0      ;init'd by reqtext.c

_newAutoRequest:         ;position critical

   suba.l   #sv_SIZEOF,sp          ;get my stack
   move.l   a0,olda0(sp)           ;save a0

   lea.l    _useCount(PC),a0       ;increment useCount
   addq.l   #1,(a0)

   move.l   a1,olda1(sp)           ;save rest
   move.l   a2,olda2(sp)
   move.l   a3,olda3(sp)
   move.l   d0,oldd0(sp)

   move.l   #0,sab1(sp)            ;null string pointer storage
   move.l   #0,sab2(sp)
   move.l   #0,sab3(sp)
   move.l   #0,sapg(sp)
   move.l   #0,sang(sp)
   move.l   #0,saw(sp)
   move.l   #0,bug(sp)

   move.l   olda1(sp),d0           ;Body IntuiText ptr to d0
   beq      endbody
   move.l   d0,a0
   movea.l  it_IText(a0),a1        ;its string to a1
   move.l   a1,sab1(sp)            ; and save area
   bsr      matchit	           ;preserves a0, old or new str in a1
   move.l   a1,it_IText(a0)        ;

*----- Special new debug stuff if 3-liner with replacement string1==DEBUG
*----- New string is in a1, Intuitext structure in a0
*----- Can use a2,a3,d0

   movea.l  a1,a2         ;new string to a2
   lea.l    debstr(PC),a3          ;"DEBUG" to a3
looper:
   tst.b    (a3)                   ;get "DEBUG" char
   beq.s    isdebug                ;if got to null, is DEBUG
   cmpm.b   (a2)+,(a3)+            ;else compare chars and bump index
   bne      notdebug               ;if different, exit
   bra.s    looper                 ;else check next

isdebug:
   move.l   it_NextText(a0),d0     ;2nd line in original ?
   beq      notdebug               ;no
   move.l   d0,a2         ;yes
   move.l   it_NextText(a2),d0     ;3rd line in original ?
   beq      notdebug               ;no

   move.l   #1,bug(sp)             ;yes - we be buggin
   move.l   a0,tmpa0(sp)           ;save pointer to first IntuiText

   move.l   a6,tmpa6(sp)
   movea.l  ABSEXECBASE,a6

*----- Get into Supervisor mode
   move.l   a5,tmpa5(sp)   ;save a5
   move.l   d1,tmpd1(sp)   ;save d1
   lea.l    insuper(PC),a5
   jmp      _LVOSupervisor(a6)
insuper:
   addq.l   #6,SP          ; ignore the SR, PC saved by that
   btst.b   #AFB_68010,AttnFlags+1(a6)
   beq.s    1$
   addq.l   #2,SP          ; remove format word
1$
   move.l   SP,a5          ; get SSP
   andi.w   #(~$2000),SR   ; back to user mode

*----- Format debug lines

*----- Need Exception number, Task address, PC, SP, SSP
   lea.l    nbuf(PC),a1             ; a1 = nbuf
   move.l   EXNUM(a5),0(a1)         ; show exception number
   andi.l   #$00FF,0(a1)
   move.l   ThisTask(a6),a0         ; Task to a0
   move.l   a0,4(a1)                ; show task address
   move.l   #USPC00,d0              ;offset on superstack to user PC
   cmp.l    #3,EXNUM(a5)            ;unless exception 3
   bne.s    not3		    ;
   btst.b   #AFB_68010,AttnFlags+1(a6)     ;on a plain 68000
   bne.s    not3
   add.l    #8,d0                   ;in which case it is 8 bytes further
not3:
   move.l   0(a5,d0.l),8(a1)        ; PC
   movea.l  sp,a0                   ; current DOS SP
   adda.l   #KLUDGE1,a0             ; where dos stashed my sp
   movea.l  0(a0),a0                ; get my sp
   adda.l   #KLUDGE2,a0             ; adjust for DOS pushes
   move.l   a0,12(a1)               ; we'll show this

   move.l   a5,16(a1)               ; SSP
   lea.l    tstr(PC),a0
   lea.l    sbuf1(PC),a3
   lea.l    putchproc(PC),a2        ;will trash a3
   jsr      _LVORawDoFmt(a6)
   move.l   tmpa0(sp),a0
   lea.l    sbuf1(PC),a1
   move.l   it_IText(a0),sab1(sp)  ;save old string pointer for line 1
   move.l   a1,it_IText(a0)        ;substitute new string
   move.l   it_NextText(a0),tmpa0(sp)  ;get next IntuiText

   move.l   a5,a1                      ; D registers at 0 SSP offset
   lea.l    dstr(PC),a0
   lea.l    sbuf2(PC),a3
   lea.l    putchproc(PC),a2           ;will trash a3
   jsr      _LVORawDoFmt(a6)
   move.l   tmpa0(sp),a0               ;this IntuiText (stored above)
   lea.l    sbuf2(PC),a1
   move.l   it_IText(a0),sab2(sp)  ;save old string pointer for line 2
   move.l   a1,it_IText(a0)        ;substitute new string
   move.l   it_NextText(a0),tmpa0(sp)   ;get next IntuiText

   lea.l    32(a5),a1                  ;A registers next, except A7
   lea.l    astr(PC),a0                ;format string
   lea.l    sbuf3(PC),a3               ;output buffer
   lea.l    putchproc(PC),a2           ;will trash a3
   jsr      _LVORawDoFmt(a6)           ;format all but a7

   move.l   tmpa0(sp),a0               ;this IntuiText (stored above)
   lea.l    sbuf3(PC),a1
   move.l   it_IText(a0),sab3(sp)  ;save old string pointer for line 3
   move.l   a1,it_IText(a0)        ;substitute new string


*----- Now put task/command name in left gadget
   movea.l  ThisTask(a6),a1            ;this task
   cmpi.b   #NT_PROCESS,LN_TYPE(a1)    ;process ?
   bne.s    justtask                   ;no
   move.l   pr_CLI(a1),d0              ;cli process ?
   beq.s    justproc                   ;no
   lsl.l    #2,d0                      ;bptr to ptr
   move.l   d0,a1                      ;cli ptr in a1
   move.l   cli_CommandName(a1),d0     ;is a command running ?
   beq.s    justtask                   ;no

   lsl.l    #2,d0                      ;bptr to ptr
   movea.l  d0,a1                      ;ptr command name bstr in a1
   moveq.l  #0,d0                      ;clear out d0
   move.b   (a1)+,d0                   ;len to d0 while bump a1 to 1st char
   clr.b    0(a1,d0.l)                 ;null terminate
   lea.l    cnstr(PC),a0               ;command name format string
   bra.s    doname
justtask:
   lea.l    tnstr(PC),a0               ;task name format string
   bra.s    tpname
justproc:
   lea.l    pnstr(PC),a0               ;process name format string
tpname:
   movea.l  LN_NAME(a1),a1             ;show task/process name
doname:
*----- Store name ptr in nbuf and point a1 there for RawDoFmt stream
   lea.l    nbuf(PC),a3                ;temp buffer for args
   lea.l    sustr(PC),a2
   move.l   a2,4(a3)                   ;ptr to SUSPEND str
   move.l   a1,0(a3)                   ;ptr to task/proc/command name
   movea.l  a3,a1                      ;arg pointer for format string
   lea.l    sbuf4(PC),a3               ;output buffer
   lea.l    putchproc(PC),a2
   jsr      _LVORawDoFmt(a6)
   move.l   olda2(sp),a0               ;Positive gadget IntuiText
   lea.l    sbuf4(PC),a1
   move.l   it_IText(a0),sapg(sp)  ;save old string pointer for PosGad
   move.l   a1,it_IText(a0)        ;substitute new string

   move.l   olda3(sp),a0               ;Negative gadget IntuiText
   lea.l    rebstr(PC),a1
   move.l   it_IText(a0),sang(sp)  ;save old string pointer for NegGad
   move.l   a1,it_IText(a0)        ;substitute new string


*---- cleanup and go to exit code
   move.l   tmpa5(sp),a5
   move.l   tmpa6(sp),a6
   move.l   tmpd1(sp),d1
   move.l   d2,saw(sp)
   move.l   #640,d2
   bra      endneg                ;skip normal rest of string processing
notdebug:
*-----


   move.l   it_NextText(a0),d0     ;Next IntuiText ?
   beq      endbody
   move.l   d0,a0
   movea.l  it_IText(a0),a1        ;its string to a1
   move.l   a1,sab2(sp)            ; and save area
   bsr      matchit                ;preserves a0, old or new str in a1
   move.l   a1,it_IText(a0)        ;

   move.l   it_NextText(a0),d0     ;Next IntuiText ?
   beq      endbody
   move.l   d0,a0
   movea.l  it_IText(a0),a1        ;its string to a1
   move.l   a1,sab3(sp)            ; and save area
   bsr      matchit                ;preserves a0, old or new str in a1
   move.l   a1,it_IText(a0)        ;

endbody:

   move.l   olda2(sp),d0           ;IntuiText ptr to d0
   beq      endpos
   move.l   d0,a0
   movea.l  it_IText(a0),a1        ;its string to a1
   move.l   a1,sapg(sp)            ; and save area
   bsr      matchit                ;preserves a0, old or new str in a1
   move.l   a1,it_IText(a0)        ;

endpos:

   move.l   olda3(sp),d0           ;IntuiText ptr to d0
   beq      endneg
   move.l   d0,a0
   movea.l  it_IText(a0),a1        ;its string to a1
   move.l   a1,sang(sp)            ; and save area
   bsr      matchit                ;preserves a0, old or new str in a1
   move.l   a1,it_IText(a0)        ;

endneg:

   move.l   olda0(sp),a0           ;restore AutoRequest registers
   move.l   olda1(sp),a1
   move.l   olda2(sp),a2
   move.l   olda3(sp),a3
   move.l   oldd0(sp),d0
   tst.l    bug(sp)                ;are we buggin ? (set condition codes)
   beq.s    diskiok                ;no - leave PosFlags alone
   andi.l   #~DISKINSERTED,d0      ;yes - remove retries on diskinserted
diskiok:
   pea.l    comeback(PC)                  ;where AutoRequest returns to
   move.l   _oldAutoRequest(PC),-(sp)     ;where AutoRequest is
   rts                 ;jsr to AutoRequest()


comeback:

   move.l   a0,olda0(sp)           ;save a couple of registers
   move.l   d0,oldd0(sp)

test1a:             ;test each string pointer
   movea.l  olda1(sp),a0           ;if not null, replace
   cmpa.l   #0,a0
   beq.s    test2
   move.l   sab1(sp),d0
   beq.s    test1b
   move.l   d0,it_IText(a0)
test1b:
   movea.l  it_NextText(a0),a0
   cmpa.l   #0,a0
   beq.s    test2
   move.l   sab2(sp),d0
   beq.s    test1c
   move.l   d0,it_IText(a0)
test1c:
   movea.l  it_NextText(a0),a0
   cmpa.l   #0,a0
   beq.s    test2
   move.l   sab3(sp),d0
   beq      test2
   move.l   d0,it_IText(a0)
test2:
   movea.l  olda2(sp),a0
   cmpa.l   #0,a0
   beq.s    test3
   move.l   sapg(sp),d0
   beq      test3
   move.l   d0,it_IText(a0)
test3:
   movea.l  olda3(sp),a0
   cmpa.l   #0,a0
   beq.s    testw
   move.l   sang(sp),d0
   beq      testw
   move.l   d0,it_IText(a0)
testw:
   tst.l    saw(sp)
   beq.s    testend
   move.l   saw(sp),d2
testend:

   move.l   oldd0(sp),d0           ;restore d0 (saved at comeback)

   lea.l    _useCount(PC),a0       ;decrement useCount
   subq.l   #1,(a0)

   movea.l  olda0(sp),a0           ;restore a0 (saved at comeback)

   tst.l    bug(sp)                ;are we buggin ? (set condition codes)
   adda.l   #sv_SIZEOF,sp          ;restore the stack pointer
   beq.s    goback                 ;not buggin

   tst.l    d0                     ;buggin - did they want to crash ?
   beq.s    goback                 ;yes
   movea.l  ABSEXECBASE,a6         ;no - they want to cancel and hang
   moveq    #0,d0                  ;so we will
   jsr      _LVOWait(a6)           ;wait forever

goback:
   rts                             ;exit to caller of AutoRequest


* string passed in a1
* must preserve a0, return new or old string in a1
* can use a2,a3,d0
* this version must match WHOLE string, not partial

matchit:
   cmpa.l   #0,a1         ;keep ptr to passed string in a1
   beq       matchex
   tst.b    (a1)                   ;screen passed null string
   beq       matchex

   move.l   a0,-(sp)               ;save a0
   lea.l    _textPtrs(PC),a0       ;addr of next compare string ptr in a0

mloop:
   movea.l  (a0),a2                ;ptr to compare string in a2
   move.l   a1,a3         ;passed string in a3

   cmpa.l   #0,a2         ;if null ptr to compare string
   beq.s    nomatch                ;  checked all, there's no match

loop:
   tst.b    (a3)                   ;if got to null of passed string
   BNE.S    notend                 ;  (These two lines added to prevent
   TST.B    (a2)                   ;  matching partial strings (as in More))
   beq.s    match                  ;  it's a match (init null screened)

notend:
   cmpm.b   (a2)+,(a3)+            ;else compare chars and bump index
   bne.s    nomatchyet             ;if different, try at next position
   bra.s    loop                   ;else check next char

nomatchyet:
   addq.l   #8,a0         ;try match with next compare string
   bra       mloop

match:
   addq.l   #4,a0         ;bump a0 to replacement str ptr
   movea.l  (a0),a1                ;put ptr in a1 for return

nomatch:            ;leave ptr to passed string in a1

   move.l   (sp)+,a0               ;restore a0

matchex:
   rts


putchproc:
   move.b  d0,(a3)+
   rts

   CNOP 0,4
sbuf1   DS.B 80
sbuf2   DS.B 80
sbuf3   DS.B 80
sbuf4   DS.B 80
nbuf    DS.B 80

tstr  DC.B 'Exception=%lx  Task=%08lx  PC=%08lx  SP=%08lx(?)  SSP=%08lx',0
dstr  DC.B 'd: %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx',0
astr  DC.B 'a: %08lx %08lx %08lx %08lx %08lx %08lx %08lx --------',0
* a7str DC.B '%08lx',0

cnstr DC.B 'Command: %s %s',0
pnstr DC.B 'Process: %s %s',0
tnstr DC.B 'Task: %s %s',0
sustr DC.B '   SUSPEND',0

debstr   DC.B 'DEBUG',0
rebstr   DC.B 'REBOOT',0

   CNOP 0,4

_useCount    DC.L   0      ;no removal if UseCount != 0

* text pointer arrays, text, and other data placed here by reqtext.c
_textPtrs:
_endFix:

   END

