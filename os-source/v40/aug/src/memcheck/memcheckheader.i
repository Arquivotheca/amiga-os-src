;*****************************************************************************/
;
; memcheckheader.i
; public structures used by MemCheck
;
;*****************************************************************************/

    IFND EXEC_TYPES_I
    INCLUDE "exec/types.i"
    ENDC	; EXEC_TYPES_I

    IFND DOS_DOS_I
    INCLUDE "dos/dos.i"
    ENDC	; DOS_DOS_I

;*****************************************************************************/

NAME	equ	'« MemCheck »',0

;*****************************************************************************/

 STRUCTURE MemCheck,0
    ULONG	mc_Cookie		; Identifier (0xC0DEF00D)
    ULONG	mc_Size			; Size of allocation
    ULONG	mc_Hunk			; Hunk within executable where allocation was made
    ULONG	mc_Offset		; Offset within hunk where allocation was made
    APTR	mc_Task			; Task address
    ULONG	mc_CheckSum		; Check sum of the cookie
    STRUCT	mc_DateStamp,ds_SIZEOF	; Allocation date stamp (3 LONG)
    ULONG	mc_Pad			; Alignment
    STRUCT	mc_Name,(16)		; Allocation task name
    STRUCT	mc_StackPtr,(4*16)	; 16 long-words of stack at allocation time
 LABEL		mc_SIZE

;*****************************************************************************/

 STRUCTURE MemCheckInfo,0
    STRUCT	mci_DateStamp,ds_SIZEOF	; When MemCheck was activated
    UWORD	mci_Version		; Version of MemCheck
    UWORD	mci_Revision		; Revision of MemCheck
    ULONG	mci_Flags;		; Additional flags
    UWORD	mci_Stack		; Number of lines of stack to show
    UWORD	mci_InfoSize		; Size of MemCheck structure
    UWORD	mci_PreSize		; Size of pre-wall
    UWORD	mci_PostSize		; Size of post-wall
    UBYTE	mci_FillChar		; Fill character for wall
    UBYTE	mci_Pad			; Word aligned
 LABEL		mci_SIZE

;*****************************************************************************/

INFOSIZE	equ	mc_SIZE
PRESIZE		equ	32
POSTSIZE	equ	32
FILLCHAR	equ	$BB
