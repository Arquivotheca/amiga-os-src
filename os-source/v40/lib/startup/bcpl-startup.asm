*
* Interface code to run BCPL programs under Amiga Tripos
*
* We are entered with a stack allocated, and saved on the stack
* Return Address; StackSize, PreviousStack
*
* Registers are set up with normal BCPL calling conventions,
* but in general these need not be saved.
* 
* 
*
* Register definition
*
Z         EQUR    A0                 Constant Zero
P         EQUR    A1                 BCPL P ptr
G         EQUR    A2                 BCPL G ptr
L         EQUR    A3                 Work reg
B         EQUR    A4                 Addr of current procedure
S         EQUR    A5                 Save routine addr
R         EQUR    A6                 Return addr
*
G_START   EQU         1*4
G_STOP    EQU         2*4
G_SBASE   EQU        12*4
G_GLOBIN  EQU        28*4
G_GVEC    EQU        29*4
G_FVEC    EQU        30*4
G_SEGLIST EQU        89*4
G_CLI     EQU       134*4
G_MIN     EQU      149
C_LINK    EQU        0               Link to next co-routine
C_CLLR    EQU        4               Caller coroutine
C_SEND    EQU        8               Stack end - 32
C_RESP    EQU       12               Resumption ptr
C_FUNC    EQU       16               Function
CLI_UPB   EQU       15               Size of CLI structure
NEGSIZE   EQU       50
*
ENTER
          MOVEA.L    G_SEGLIST(G),B
          MOVEQ      #12,D0
          JSR        (S)                    Get the segment list
          MOVE.L     D1,D2                  Save for later
          MOVE.L     #G_MIN,D1              D1 to hold Gvec size
          LEA.L      ENTER,B                Get this entry point
          MOVEA.L    -4(B),B                BCPL ptr to next section
          MOVE.L     B,-(SP)                Save this for later
          MOVE.L     D2,-(SP)               Save seglist for later

ACT1      ADDA.L     B,B
          ADDA.L     B,B                    B = MC addr of next section
          MOVE.L     B,D0                   Test for end of list
          BEQ.S      ACT5                   Ended
*
ACT2      MOVE.L     4(B),D0                D0 = size of new section
          ASL.L      #2,D0                  Now in bytes
          CMP.L      0(B,D0.L),D1           J if Gvec size >= the size
          BGE.S      ACT3                     required by this section
          MOVE.L     0(B,D0.L),D1           Yes - update Gvec size
*
ACT3      MOVEA.L    (B),B                  Next section
          BRA.S      ACT1
*
* D1 = required size of the global vector in (long) words
*
ACT5      MOVE.L     D1,D6                  D6 = Gvec UPB
          ADDI.L     #NEGSIZE,D1            Include negative globals
          SUBA.L     Z,Z
*
          MOVEA.L    G_GVEC(G),B
          MOVEQ      #12,D0
          JSR        (S)                    Allocate the global vector
*
ACT6      TST.L      D1
          BEQ        ACTERR                 J if allocation not successful
          ADD.L      #NEGSIZE,D1            Skip past negtive globals
          MOVE.L     G_GLOBIN(G),D5         Save address of GLOBIN
          MOVEA.L    D1,G
          ADDA.L     G,G
          ADDA.L     G,G                    GV now set
          MOVE.L     D1,D7                  D7 = BCPL ptr to G vector
*
*
* At this point:
*
*         D7 = BCPL ptr to the global vector
*         G  = MC address of the global vector
*         D6 = Gvec UPB
*
          MOVEA.L    G,Z
          MOVE.L     D6,(Z)+                Set G0 = Gvec size
          MOVEA.L    D6,B
          ADDA.L     D7,B
          ADDA.L     B,B
          ADDA.L     B,B                    B = MC addr of last Gvec entry
*
          MOVE.L     #$474C0003,D0          Form UNGLOB + 2*1
*
ACT7      MOVE.L     D0,(Z)+                Put UNGLOB+2*n in global n
          ADDQ.L     #2,D0
          CMPA.L     B,Z
          BLE.S      ACT7                   Loop until all done
          SUBA.L     Z,Z
*
* Now set up new BCPL stack
*
          MOVE.L     12(SP),D0              Stacksize in bytes
          LEA.L      16(SP),P               Stack high
          SUBA.L     D0,P                   Now stack low
*
          MOVE.L     #-1,C_CLLR(P)          -1 -> root coroutine
          MOVE.L     P,D1
          SUBI.L     #160,D0                Less 40 words for safety
          ADD.L      D1,D0
          MOVE.L     D0,C_SEND(P)           Stack end
          ASR.L      #2,D1                  BCPL stackbase
          MOVE.L     D1,G_SBASE(G)          Into GV
*
          MOVE.L     (SP)+,D7               D7 = BCPL ptr to SEGL
          MOVE.L     D7,D6
          ASL.L      #2,D6                  D6 = MC addr to SEGL
          ADD.L      0(Z,D6.L),D7
          ASL.L      #2,D7                  D7 = MC addr of last entry
*
ACT11     ADDQ.L     #4,D6                  D6 = MC addr of next entry
          CMP.L      D6,D7
          BLT.S      ACT12                  J if all entries processed
*
* At this point:
*
*      D5 = MC addr of GLOBIN
*      D6 = MC addr of the current entry of SEGL
*      D7 = MC address of the last entry of SEGL
*      Z  = 0
*
* GLOBIN only changes D0, D1, D2, D3, B and L
*
*
          MOVE.L     0(Z,D6.L),D1           D1 = BCPL ptr to segment
          MOVEA.L    D5,B                   Address of GLOBIN
          MOVEQ      #12,D0
          JSR        (S)
          TST.L      D1
          BEQ        ACTERR1
          BRA.S      ACT11
*
* Now initialise globals in this loaded segment
*
ACT12     MOVE.L     (SP)+,D1
          MOVEA.L    D5,B                   Address of GLOBIN
          MOVEQ      #12,D0
          JSR        (S)
          TST.L      D1
          BEQ.S      ACTERR2
*
* Now copy CLI globals into global vector
*
          MOVEA.L    G_CLI(G),B
          MOVEQ      #12,D0
          JSR        (S)
          ASL.L      #2,D1
          MOVEA.L    D1,L                   L now points at CLI globals
          MOVE.L     L,-(SP)                Save for later
          BEQ.S      NOCLI                  Not called from a CLI
          LEA.L      G_CLI(G),B             B points at GV slot
          MOVEQ      #CLI_UPB,D0
CG        MOVE.L     (L)+,(B)+
          DBRA       D0,CG
*
* Patch new version of STOP
*
NOCLI     LEA.L      STOPF(PC),B
          MOVE.L     B,G_STOP(G)
*
* We are now ready to call START()
*
          MOVEA.L    G_START(G),B
          MOVEQ      #32,D0                 Stack increment
          MOVEQ      #0,D1
          JSR        (S)                    Now call START(0)
          MOVEQ      #0,D0                  0 return code
*
* Task has finished normally - free its global vector
*
DEACT     MOVE.L     D0,D7                  Save return code
          MOVE.L     (SP)+,D1               D1 points at CLI struct
          BEQ.S      CG3                    None to copy
          MOVEA.L    D1,L                   Copy to address reg
          LEA.L      G_CLI(G),B             B points at GV slot
          MOVEQ      #CLI_UPB,D0
CG2       MOVE.L     (B)+,(L)+              Copy CLI info back
          DBRA       D0,CG2
*
CG3       MOVE.L     G,D1
          ASR.L      #2,D1
          SUB.L      #NEGSIZE,D1            Include negative slots
          MOVEA.L    G_FVEC(G),B
          MOVEQ      #12,D0
          JSR        (S)                    Call FREEVEC(gbase)
*
* Finally return to invoker with return code
*
          MOVE.L     D7,D0
          RTS
*
* Error return
*
ACTERR    TST.L      (SP)+                  Junk saved seglist
ACTERR1   TST.L      (SP)+                  Junk saved segment
ACTERR2   MOVEQ      #-1,D0                 Signal error
          RTS
*
* This is the modified STOP routine
*
STOPF     MOVE.L     D1,D0                  Copy returncode
          BRA.S      DEACT                  And J to DEACT
          END
