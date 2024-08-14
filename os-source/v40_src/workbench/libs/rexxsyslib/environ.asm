* == environ.asm =======================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ============================     GetEnv     ==========================
* Allocates and initializes an environment structure.  The global data
* structure is also the base environment.
* Registers:   A0 -- global data pointer
*              A1 -- previous environment
* Return:      D0 -- allocated environment
*              A0 -- same
GetEnv
         movem.l  d2/a2-a5,-(sp)
         movea.l  a0,a3
         movea.l  a1,a4                ; previous environment
         movea.l  a0,a5                ; target environment

         ; Check if we need to allocate a new environment structure.

         move.l   a4,d0                ; previous environment?
         beq.s    1$                   ; no
         moveq    #ev_SIZEOF,d0
         bsr      FindSpace            ; allocate a new one
         movea.l  d0,a5                ; new environment

         ; Link in the new environment and initialize it.

1$:      lea      ENVLIST(a3),a0       ; environment list header
         movea.l  a5,a1                ; new environment
         bsr      AddTail              ; link it in

         move.l   a5,gn_CurrEnv(a3)    ; current environment
         move.l   a3,ev_GlobalPtr(a5)  ; global pointer

         ; Use the global source segment as the initial segment.

         move.l   gn_SrcSeg(a3),ev_Segment(a5)

         ; Clear selected fields

         clr.l    ev_ArgList(a5)       ; argument list

         ; Initialize the Range and Execution stack headers.

         lea      ev_RStack(a5),a0     ; range stack
         clr.w    skNum(a0)            ; clear count
         CALLSYS  InitList
         lea      ev_EStack(a5),a0     ; execution stack
         clr.w    skNum(a0)
         CALLSYS  InitList

         lea      STATUS(a5),a2        ; start of inherited values
         lea      ev_Prompt(a5),a3     ; start of inherited strings
         move.l   a4,d0                ; previous environment?
         bne.s    2$                   ; yes

         ; Base environment ... initialize non-zero values.

         movea.l  a5,a0                ; environment
         bsr      GetSymTable          ; D0=A0=symbol table

         move.l   #(EXF_MOVE!TRACE_N),(a2)+
         lea      12(a2),a2            ; ev_SrcPos/ev_TraceLim/ev_TraceCnt
         move.l   d0,(a2)+             ; ev_ST
         move.w   #9,(a2)+             ; ev_NumDigits
         addq.w   #2,a2                ; (ev_NumFuzz)
         move.w   #TRINDMAX,(a2)+      ; ev_Indent
         addq.w   #2,a2                ; (ev_IntFlags)
         subq.l   #1,(a2)              ; ev_Elapse

         ; Install the default strings.

         move.l   rl_NULL(a6),(a3)+    ; NULL string
         move.l   rl_COMMAND(a6),(a3)+ ; "COMMAND" string
         move.l   rl_REXX(a6),(a3)+    ; "REXX" string

         ; Initialize the environment flags ...

         move.b   #1<<EFB_NEWST!1<<EFB_CACHE!1<<EFB_SCI,EVFLAGS(a5)
         bra.s    5$

         ; Install the inherited values from the old environment.

2$:      lea      STATUS(a4),a0        ; source for inherited values
         moveq    #ENVLCNT-1,d2        ; long-word count

3$:      move.l   (a0)+,(a2)+          ; copy values
         dbf      d2,3$                ; loop back
         andi.l   #EXECMASK,STATUS(a5)

         ; Copy the inherited strings

         lea      ev_Prompt(a4),a2     ; source for inherited strings
         moveq    #ENVSCNT-1,d2        ; string count

4$:      movea.l  a4,a0                ; previous environment
         movea.l  (a2)+,a1             ; old string
         bsr      KeepString           ; D0=A1=string
         move.l   a1,(a3)+             ; install new string
         dbf      d2,4$                ; loop back

         moveq    #ENVFMASK,d0         ; mask for inherited flags
         and.b    EVFLAGS(a4),d0       ; old flags
         move.b   d0,EVFLAGS(a5)

         ; Increment the nesting level ...

         move.w   ev_Level(a4),d0      ; previous level
         addq.w   #1,d0                ; plus one
         move.w   d0,ev_Level(a5)      ; new level

5$:      movea.l  a5,a0                ; new environment
         move.l   a0,d0
         movem.l  (sp)+,d2/a2-a5
         rts

* ==========================     FreeEnv     ===========================
* Releases the current environment structure and any related memory.
* Registers:   A0 -- global data pointer
FreeEnv
         movem.l  d2/a2-a4,-(sp)
         movea.l  a0,a3                ; base environment
         movea.l  gn_CurrEnv(a3),a4    ; current environment

         ; Release the symbol table if one was created.

         btst     #EFB_NEWST,EVFLAGS(a4)
         beq.s    1$                   ; ... no symbol table
         movea.l  a4,a0                ; current environment
         bsr      FreeSymTable         ; release symbol table

         ; Release any clauses in the execution pipeline.

1$:      movea.l  a4,a0                ; this environment
         bsr      FlushClause

         ; Release all control ranges ...

2$:      movea.l  a4,a0                ; this environment
         bsr      FreeRange            ; D0=boolean
         bne.s    2$                   ; loop back

         ; Release the arglist clause, if present.

         move.l   ev_ArgList(a4),d0    ; an arglist?
         beq.s    3$                   ; no
         movea.l  a4,a0                ; this environment
         movea.l  d0,a1                ; clause
         bsr      FreeClause           ; release it

         ; Release the inherited strings.

3$:      moveq    #ENVSCNT-1,d2        ; string count
         lea      ev_Prompt(a4),a2     ; first string

4$:      movea.l  a3,a0                ; base environment
         movea.l  (a2)+,a1             ; string
         bsr      FreeKeepStr          ; release it
         dbf      d2,4$                ; loop back

         ; Update the current environment and unlink.

         move.l   EVPRED(a4),gn_CurrEnv(a3)
         movea.l  a4,a1                ; this environment
         bsr      Remove               ; unlink it

         ; Release the environment structure (never the base).

         movea.l  a3,a0                ; base environment
         movea.l  a4,a1                ; this environment
         moveq    #ev_SIZEOF,d0
         bsr      FreeSpace            ; release it

         movem.l  (sp)+,d2/a2-a4
         rts

* =========================     FreeGlobal     =========================
* Strips the global data structure of any allocated resources.  The global
* structure itself is not released, since it was allocated externally.
* Registers:      A0 -- global data pointer
FreeGlobal
         move.l   a3,-(sp)
         movea.l  a0,a3

         lea      rt_MsgPort(a3),a0    ; global replyport
         CALLSYS  FreePort             ; deallocate port resources

         ; Release the global message packet.

         move.l   gn_MsgPkt(a3),d0     ; a message packet?
         beq.s    1$                   ; no
         movea.l  d0,a0
         CALLSYS  DeleteRexxMsg        ; release it

         ; Release the global work buffer argstring, if any.

1$:      movea.l  gn_TBuff(a3),a0      ; argstring or 0
         CALLSYS  DeleteArgstring      ; release it

         ; Release the temporary value argstring, if any.

         movea.l  gn_TempValue(a3),a0  ; argstring or 0
         CALLSYS  DeleteArgstring      ; release it

         ; Deallocate the MemList.

         lea      MEMLIST(a3),a0       ; list header
         CALLSYS  RemRsrcList

         ; Deallocate the FileList.

         lea      FILELIST(a3),a0      ; list header
         CALLSYS  RemRsrcList

         ; Deallocate the PortList.

         lea      PORTLIST(a3),a0      ; list header
         movea.l  (sp)+,a3             ; restore register
         jmp      _LVORemRsrcList(a6)  ; release everything
