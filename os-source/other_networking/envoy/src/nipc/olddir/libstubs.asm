
   XDEF   BeginTransaction,DoTransaction,AbortTransaction,WaitTransaction,CheckTransaction
   XDEF   AllocTransaction,CreateEntity,DeleteEntity,WaitEntity,FindEntity,LoseEntity
   XDEF   GetTransaction,ReplyTransaction,WaitTransaction,FreeTransaction

   XREF   _BeginTransaction,_DoTransaction,_AbortTransaction,_WaitTransaction,_CheckTransaction
   XREF   _AllocTransaction,_CreateEntity,_DeleteEntity,_WaitEntity,_FindEntity,_LoseEntity
   XREF   _GetTransaction,_ReplyTransaction,_WaitTransaction,_FreeTransaction

BeginTransaction:
   movem.l  d2-d7/a2-a6,-(sp)
   move.l   a2,-(sp)
   move.l   a1,-(sp)
   move.l   a0,-(sp)
   jsr      _BeginTransaction
   add.l    #12,sp
   movem.l  (sp)+,d2-d7/a2-a6
   rts

DoTransaction:
   movem.l  d2-d7/a2-a6,-(sp)
   move.l   a2,-(sp)
   move.l   a1,-(sp)
   move.l   a0,-(sp)
   jsr      _DoTransaction
   add.l    #12,sp
   movem.l  (sp)+,d2-d7/a2-a6
   rts

AbortTransaction:
   movem.l  d2-d7/a2-a6,-(sp)
   move.l   a1,-(sp)
   jsr      _AbortTransaction
   add.l    #04,sp
   movem.l  (sp)+,d2-d7/a2-a6
   rts

CheckTransaction:
   movem.l  d2-d7/a2-a6,-(sp)
   move.l   a1,-(sp)
   jsr      _CheckTransaction
   add.l    #04,sp
   movem.l  (sp)+,d2-d7/a2-a6
   rts

AllocTransaction:
   movem.l  d2-d7/a2-a6,-(sp)
   move.l   a0,-(sp)
   jsr      _AllocTransaction
   add.l    #04,sp
   movem.l  (sp)+,d2-d7/a2-a6
   rts

FreeTransaction:
   movem.l  d2-d7/a2-a6,-(sp)
   move.l   a1,-(sp)
   jsr      _FreeTransaction
   add.l    #04,sp
   movem.l  (sp)+,d2-d7/a2-a6
   rts

CreateEntity:
   movem.l  d2-d7/a2-a6,-(sp)
   move.l   a0,-(sp)
   jsr      _CreateEntity
   add.l    #04,sp
   movem.l  (sp)+,d2-d7/a2-a6
   rts

DeleteEntity:
   movem.l  d2-d7/a2-a6,-(sp)
   move.l   a0,-(sp)
   jsr      _DeleteEntity
   add.l    #04,sp
   movem.l  (sp)+,d2-d7/a2-a6
   rts

WaitEntity:
   movem.l  d2-d7/a2-a6,-(sp)
   move.l   a0,-(sp)
   jsr      _WaitEntity
   add.l    #04,sp
   movem.l  (sp)+,d2-d7/a2-a6
   rts

FindEntity:
   movem.l  d2-d7/a2-a6,-(sp)
   move.l   a1,-(sp)
   move.l   a0,-(sp)
   jsr      _FindEntity
   add.l    #08,sp
   movem.l  (sp)+,d2-d7/a2-a6
   rts

LoseEntity:
   movem.l  d2-d7/a2-a6,-(sp)
   move.l   a0,-(sp)
   jsr      _LoseEntity
   add.l    #04,sp
   movem.l  (sp)+,d2-d7/a2-a6
   rts

GetTransaction:
   movem.l  d2-d7/a2-a6,-(sp)
   move.l   a0,-(sp)
   jsr      _GetTransaction
   add.l    #04,sp
   movem.l  (sp)+,d2-d7/a2-a6
   rts

ReplyTransaction:
   movem.l  d2-d7/a2-a6,-(sp)
   move.l   a1,-(sp)
   jsr      _ReplyTransaction
   add.l    #04,sp
   movem.l  (sp)+,d2-d7/a2-a6
   rts

WaitTransaction:
   movem.l  d2-d7/a2-a6,-(sp)
   move.l   a1,-(sp)
   jsr      _WaitTransaction
   add.l    #04,sp
   movem.l  (sp)+,d2-d7/a2-a6
   rts


 end
