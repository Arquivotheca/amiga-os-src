        IFND APPN_SERVICES_I
APPN_SERVICES_I SET     1
**
** $Id: services.i,v 37.2 92/06/04 18:40:57 kcd Exp $
**
** (C) Copyright 1992, Commodore-Amiga, Inc.
**
** Public Structures and definitions for services.library
**
**

        INCLUDE "exec/types.i"
        INCLUDE "utility/tagitem.i"

;---------------------------------------------------------------------------
;
; Defined tags for FindService()
;

FSVC_Dummy      EQU TAG_USER + 2048

FSVC_Error      EQU FSVC_Dummy + $02
FSVC_UserName   EQU FSVC_Dummy + $03
FSVC_PassWord   EQU FSVC_Dummy + $04

;---------------------------------------------------------------------------
;
; Defined tags for StartService()
;


SVCAttrs_Dummy          EQU TAG_USER + 4096

SVCAttrs_Name           EQU SVCAttrs_Dummy + 1    ; Your Service Name

;---------------------------------------------------------------------------

    ENDC    ; APPN_SERVICES_I