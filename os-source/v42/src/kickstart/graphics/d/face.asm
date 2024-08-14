*******************************************************************************
*
*   $Id: face.asm,v 37.0 91/01/07 15:28:04 spence Exp $
*
*   $Locker:  $
*
*******************************************************************************

    include 'exec/types.i'
    include 'exec/nodes.i'
    include 'exec/lists.i'
    include 'exec/libraries.i'

    section graphics
    code

    xdef    _FindDisplayInfo
    xref    _find_info

_FindDisplayInfo:

    move.l  d0,-(sp)
    jsr     _find_info
    addq.l  #4,sp
    rts

    xdef    _NextDisplayInfo
    xref    _next_info

_NextDisplayInfo:

    move.l  d0,-(sp)
    jsr     _next_info
    addq.l  #4,sp
    rts

    xdef    _GetDisplayInfoData
    xref    _cook_chunk

_GetDisplayInfoData:

    move.l  d2,-(sp)
    move.l  d1,-(sp)
    move.l  d0,-(sp)
    move.l  a1,-(sp)
    move.l  a0,-(sp)
    jsr     _cook_chunk
    lea.l   20(sp),sp
    rts

    xdef    _SetDisplayInfoData
    xref    _uncook_chunk

_SetDisplayInfoData: ; graphics private

    move.l  d2,-(sp)
    move.l  d1,-(sp)
    move.l  d0,-(sp)
    move.l  a1,-(sp)
    move.l  a0,-(sp)
    jsr     _uncook_chunk
    lea.l   20(sp),sp
    rts

    xdef    _AddDisplayInfoData
    xref    _add_chunk

_AddDisplayInfoData: ; graphics private

    move.l  d2,-(sp)
    move.l  d1,-(sp)
    move.l  d0,-(sp)
    move.l  a1,-(sp)
    move.l  a0,-(sp)
    jsr     _add_chunk
    lea.l   20(sp),sp
    rts

    xdef    _AddDisplayInfo
    xref    _add_info

_AddDisplayInfo: ; graphics private

    move.l  a0,-(sp)
    jsr     _add_info
    addq.l  #4,sp
    rts

    end
