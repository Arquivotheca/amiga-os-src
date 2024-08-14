* == btree.i ===========================================================
* Include file for binary tree structures.  Used for symbol table and
* expression trees.

         IFND     REXX_BTREE_I
REXX_BTREE_I   SET   1

         STRUCTURE BTEntry,0
         APTR     be_PValue            ; pointer to value structure
         APTR     be_Next              ; pointer to next symbol table node
         LABEL    be_SIZEOF            ; size: 8 bytes

         STRUCTURE BTNode,0
         APTR     bt_HiKid             ; HIGH branch descendent
         APTR     bt_LoKid        
         APTR     bt_Parent            ; pointer to parent
         UWORD    bt_Flags
         WORD     bt_Mode              ; HIGH/LOW mode
         APTR     bt_PName             ; pointer to name string structure
         STRUCT   BTENTRY,be_SIZEOF
         APTR     btPst                ; pointer to symbol table entry
         LABEL    bt_SIZEOF            ; size: 32 bytes

* Field definitions
BTHIGH   EQU      bt_HiKid
BTLOW    EQU      bt_LoKid
BTPARENT EQU      bt_Parent
BTFLAGS  EQU      bt_Flags
BTMODE   EQU      bt_Mode
BTNAME   EQU      bt_PName

* Mode switch values 
HIGHMODE EQU      BTHIGH
LOWMODE  EQU      BTLOW

* Definitions for expression-tree nodes
BTRIGHT  EQU      bt_HiKid             ; 'RIGHT' branch same as 'HIGH'
BTLEFT   EQU      bt_LoKid
EXPTYPE  EQU      bt_Flags             ; node type byte
EXPPRIOR EQU      bt_Flags+1           ; node priority byte
EXPTOKEN EQU      btPst                ; token

EXPRIGHT EQU      HIGHMODE             ; mode switches
EXPLEFT  EQU      LOWMODE

NE_TERM  EQU      1                    ; expression node types
NE_OPER  EQU      2
NE_SUBT  EQU      3

NE_SYMP  EQU       (10<<4)             ; node priority for symbols/strings
NE_MAXP  EQU       (15<<4)             ; maximum priority

         ENDC
