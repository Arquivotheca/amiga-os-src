        IFND ENVOY_SERVICES_I
ENVOY_SERVICES_I SET     1
**
** $Id: services.i,v 37.3 92/06/09 15:16:12 kcd Exp $
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

    ENDC    ; ENVOY_SERVICES_I