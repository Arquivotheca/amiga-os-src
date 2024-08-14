;*************************************************************************
;
; djmount.i -- DOS equates for the jdisk mount command
;
; Copyright (c) 1986, Commodore Amiga Inc.,  All rights reserved.
;
;*************************************************************************

;------ from dos/DOS/IOHDR...
 STRUCTURE  JMDev,0
    ULONG   jmd_Next
    ULONG   jmd_Type
    ULONG   jmd_Task
    ULONG   jmd_Lock
    ULONG   jmd_Handler
    ULONG   jmd_StackSize
    ULONG   jmd_Priority
    ULONG   jmd_Startup
    ULONG   jmd_SegList
    ULONG   jmd_GlobVec
    ULONG   jmd_Name
    LABEL   jmd_SIZEOF

;------ jmd_Type
JMD_DEVICE  EQU 0
JMD_DIR     EQU 1
JMD_VOLUME  EQU 2


;------ from dos/DOS/MANHDR...
 STRUCTURE  JMEnvir,0
    ULONG   jme_StructSize      ; = 11
    ULONG   jme_BlockSize
    ULONG   jme_SectorOrigin
    ULONG   jme_NSurfaces
    ULONG   jme_NSectsPerBlk
    ULONG   jme_NBlksPerTrack
    ULONG   jme_NReservedBlks
    ULONG   jme_NPreallocated
    ULONG   jme_Interleave
    ULONG   jme_LowCyl
    ULONG   jme_HighCyl
    ULONG   jme_NBuffers
    LABEL   jme_SIZEOF


;------ from looking at "Mount" source and Amiga memory...
 STRUCTURE  JMStartup,0
    ULONG   jms_Unit            ; the device unit number
    BPTR    jms_DevName         ; the device name, BCPL string
    BPTR    jms_Envir           ; the environment vector
    ULONG   jms_Flags           ; the device flags
    ULONG   jms_pad             ; (mount seems to allocate this)
    LABEL   jms_SIZEOF


 STRUCTURE  JMAll,0
    STRUCT  jma_jmd,jmd_SIZEOF
    STRUCT  jma_jme,jme_SIZEOF
    STRUCT  jma_jms,jms_SIZEOF
    ULONG   jma_DosName
    ULONG   jma_DosNameTerm    ; need null terminated for sys/format to work
    STRUCT  jma_ExecName,16
    LABEL   jma_SIZEOF


