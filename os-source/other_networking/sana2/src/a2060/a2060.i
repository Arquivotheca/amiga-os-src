************************************************************************
*     A2060 ARCNET SANA-II Device Driver
*
*     Copyright (c) 1992 Commodore-Amiga, Inc.
*
*     $Id: a2060.i,v 1.2 92/05/06 11:14:31 gregm Exp $
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
   include "libraries/expansion.i"
   include "libraries/configvars.i"
   include "libraries/configregs.i"
   include "sana2.i"
   include "a2060.device.i"

MAN_AMER        equ         1053                * Manufacturer's code for Ameristar
MAN_COMMODORE   equ         514                 * Manufacturer's code for Commodore
PROD_A2060      equ         9                   * The product code for the A2060 (and Ameristar's version, too)

MAXDEVS equ 6                                   * We'll accept only up to 6 Arcnet cards.

   STRUCTURE UnitStruct,0
   STRUCT   us_Link,MLN_SIZE                    * A node for linking ..
   APTR     us_DeviceBase                       * A back-ptr to the Device Base
   STRUCT   us_State,4                          * State of each buffer
   APTR     us_HardwarePtr                      * A ptr to the base of this card
   APTR     us_StatusRegister                   * A ptr to the Status register
   APTR     us_CommandRegister                  * A ptr to the Command register
   UBYTE    us_MirrorIntMask                    * A read version of the write-only register
   UBYTE    us_HardwareAddress                  * The eight-bit address of this Arcnet card
   UBYTE    us_Flags                            * Flags for this device (online/offline, etc.)
   UBYTE    us_Filler                           * padding
   APTR     us_ConfigDev                        * A ptr to the ConfigDev for this Unit, as returned by exp.lib
   STRUCT   us_ReadIOR,MLH_SIZE                 * A list for keepieng track of all queued Read requests
   STRUCT   us_WriteIOR,MLH_SIZE                * A list for keeping track of all queued Write requests
   STRUCT   us_Buffers,4*4                      * four entries, a ptr for each
   STRUCT   us_SoftIntW,IS_SIZE                 * This Unit's Write SoftInt
   STRUCT   us_SoftIntR,IS_SIZE                 * This Unit's Read SoftInt
   STRUCT   us_Events,MLH_SIZE                  * List of OnEvent IORequests still pending
   STRUCT   us_GlobStats,S2DS_SIZE              * Global Statistics (GetGlobalStats)
   LABEL    us_SIZE

USF_ONLINE          equ             1           * Device is ONLINE
USB_ONLINE          equ             0           * Device is ONLINE


_LVOCopyToBuff      equ     -6
_LVOCopyFromBuff    equ     -12


STATE_EMPTY         equ             0           * Buffer not in use
STATE_RCVDATA       equ             1           * Buffer has receive data
STATE_RCVCURRENT    equ             2           * This is the current receiver
STATE_XMTCURRENT    equ             3           * This is the current xmitter
STATE_XMTDATA       equ             4           * Buffer has xmit data
STATE_ISBUSY        equ             5           * In use.

SR_RI               equ             128         * Rcvr Inhibited
SR_ETS2             equ             64          * Extended timeout status 2
SR_ETS1             equ             32          * Extended timeout status 1
SR_POR              equ             16          * Power On Reset
SR_TEST             equ             8           * Test bit
SR_RECON            equ             4           * Reconfiguration occured
SR_TMA              equ             2           * Transmit ACK
SR_TA               equ             1           * Transmit buffer Available

SRB_RI              equ             7           * Rcvr Inhibited
SRB_ETS2            equ             6           * Extended timeout status 2
SRB_ETS1            equ             5           * Extended timeout status 1
SRB_POR             equ             4           * Power On Reset
SRB_TEST            equ             3           * Test bit
SRB_RECON           equ             2           * Reconfiguration occured
SRB_TMA             equ             1           * Transmit ACK
SRB_TA              equ             0           * Transmit buffer Available

CMD_DT              equ             1           * Disable Xmitter
CMD_DR              equ             16          * Disable receiver
CMD_CONFIG          equ             13          * Receive short & long packets
CMD_CLEAR_POR_RECON equ             30          * Clear RECON, POR flag bits
CMD_CLR_POR         equ             14          * Clear POR flag
CMD_CLR_RECON       equ             22          * Clear RECON bit



   STRUCTURE DevStruct,LIB_SIZE
   UBYTE   ds_Flags                     *
   UBYTE   ds_Pad1
   ;now longword aligned
   APTR    ds_UtlBase                   * Ptr to the utility.library
   APTR    ds_SysBase                   * Ptr to ExecBase
   APTR    ds_ExpBase                   * Ptr to the expansion library
   ULONG   ds_SegList                   * A ptr to this device's segment list
   STRUCT  ds_UnitList,MLH_SIZE         * A linked-list (in order) of our Unit structures
   STRUCT  ds_HardInt,IS_SIZE           * Interrupt struct for our hardware interrupts
   APTR    ds_GPBuffer                  * 512 byte buffer to hold packets in ..
   STRUCT  ds_TypeStats,MLH_SIZE        * Keeps track of all type-base statistics
   LABEL   DevBaseDataSize


   STRUCTURE TypeNode,0                 * Used on the ds_TypeStats list
   STRUCT  tn_Node,MLN_SUCC             * for linking ..
   ULONG   tn_PacketType                * The type that this node keeps track of
   ULONG   tn_Unit                      * The Unit that these stats are for ...
   STRUCT  tn_Stats,S2PTS_SIZE          * the stats to keep track of ...
   LABEL   TypeNodeSize                 * The length of the struct



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



