        IFND ENVOY_SERVICES_I
ENVOY_SERVICES_I SET     1
**
** $Id: services.i,v 37.4 92/10/13 11:08:09 kcd Exp $
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

FSVC_Dummy      EQU TAG_USER+2048

FSVC_Error      EQU FSVC_Dummy+$02
FSVC_UserName   EQU FSVC_Dummy+$03
FSVC_PassWord   EQU FSVC_Dummy+$04

;---------------------------------------------------------------------------
;
; Defined tags for Get/SetServicesAttrsA()
;

SVCAttrs_Dummy          EQU TAG_USER+4096

SVCAttrs_Name           EQU SVCAttrs_Dummy+1    ; Your Service Name

;---------------------------------------------------------------------------
;
; Defined tags for StartServicesA()
;
;

SSVC_Dummy		EQU TAG_USER+8192

SSVC_UserName		EQU SSVC_Dummy+1	; UserName on client
SSVC_Password		EQU SSVC_Dummy+2	; Password of User
SSVC_EntityName		EQU SSVC_Dummy+3	; The Service's Entity Name
SSVC_HostName		EQU SSVC_Dummy+4	; The host trying to connect

;---------------------------------------------------------------------------

    ENDC    ; ENVOY_SERVICES_I