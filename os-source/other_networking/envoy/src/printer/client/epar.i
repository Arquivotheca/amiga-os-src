************************************************************************
*     Envoy Parallel Portish Device Driver
*
*     Copyright (c) 1992 Commodore-Amiga, Inc.
*
*     $Id: epar.i,v 1.2 93/07/30 19:10:34 jesup Exp $
*
*
************************************************************************


   include "exec/types.i"
   include "exec/devices.i"
   include "exec/io.i"
   include "exec/memory.i"
   include "exec/interrupts.i"
   include "exec/initializers.i"
   include "exec/resident.i"
   include "exec/errors.i"
   include "exec/semaphores.i"
   include "libraries/expansion.i"
   include "libraries/configvars.i"
   include "libraries/configregs.i"
   include "envoyprint_rev.i"

MAXDEVS     equ         10

LPCMD_START_PAR     equ     2
LPCMD_START_PRT     equ     1
LPCMD_DATA          equ     4
LPCMD_END           equ     5

    STRUCTURE XmitMsg,0
    STRUCT    xm_Msg,MN_SIZE
    APTR      xm_DevBase
    LABEL     xm_SIZE


   STRUCTURE DevStruct,LIB_SIZE
   UBYTE   ep_Flags                     *
   UBYTE   ep_Pad1
   ;now longword aligned
   APTR    ep_SysBase                   * Ptr to ExecBase
   APTR    ep_NIPCBase
   APTR    ep_ServicesBase
   APTR    ep_DOSBase
   APTR    ep_IntuitionBase
   ULONG   ep_SegList                   * A ptr to this device's segment list
   STRUCT  ep_UnitList,MLH_SIZE         * A linked-list (in order) of our Unit structures
   STRUCT  ep_HardInt,IS_SIZE           * Interrupt struct for our hardware interrupts
   APTR    ep_GPBuffer                  * 512 byte buffer to hold packets in ..
   STRUCT  ep_TypeStats,MLH_SIZE        * Keeps track of all type-base statistics
   STRUCT  ep_GPMsg,xm_SIZE
   APTR    ep_LocalEntity
   STRUCT  ep_Units,MLH_SIZE
   APTR    ep_OtherTask
   STRUCT  ep_StopSign,SS_SIZE
   STRUCT  ep_UnitSemaphore,SS_SIZE
   STRUCT  ep_HostName,65
   STRUCT  ep_UserName,33
   STRUCT  ep_Password,33
   UBYTE   ep_Padding
   LABEL   DevBaseDataSize

   STRUCTURE PrintUnit,0
   STRUCT    pu_Node,MLN_SIZE
   APTR      pu_RemoteEntity
   LABEL     pu_SIZE

   STRUCTURE LocalVars,0
   APTR      lv_LocalEntity
   APTR      lv_NIPCBase
   APTR      lv_SysBase
   LONG      lv_SigBit
   APTR      lv_OurTask
   LABEL     lv_SIZE

   STRUCTURE EnvoyUnit,0
   STRUCT    eu_Node,MLN_SIZE
   APTR      eu_RemoteEntity
   APTR      eu_Job
   STRUCT    eu_IORequests,MLH_SIZE
   LABEL     eu_SIZE

* Greg's favorite asm macros:

XSYS  macro
   xref _LVO\1
   endm

SYS macro
   jsr _LVO\1(a6)
   endm


         xref _kprintf
DEBUGMSG macro
         pea.l      \1
         jsr        _kprintf
         add.l      #4,sp
         endm

DEBUG1   macro
         move.l     \2,-(sp)
         pea.l      \1
         jsr        _kprintf
         add.l      #8,sp
         endm

DEBUG2   macro
         move.l     \3,-(sp)
         move.l     \2,-(sp)
         pea.l      \1
         jsr        _kprintf
         add.l      #12,sp
         endm



