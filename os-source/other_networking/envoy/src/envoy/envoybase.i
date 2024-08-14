
**
* $Id: envoybase.i,v 1.2 92/10/13 14:11:51 kcd Exp $
**

   IFND  ENVOY_BASE_I
ENVOY_BASE_I SET 1


   IFND  EXEC_TYPES_I
   INCLUDE  "exec/types.i"
   ENDC   ; EXEC_TYPES_I

   IFND  EXEC_LISTS_I
   INCLUDE  "exec/lists.i"
   ENDC   ; EXEC_LISTS_I

   IFND  EXEC_LIBRARIES_I
   INCLUDE  "exec/libraries.i"
   ENDC   ; EXEC_LIBRARIES_I


SYS macro
 jsr _LVO\1(a6)
 endm

XSYS macro
 xref _LVO\1
 endm


;-----------------------------------------------------------------------
;
; library data structures
;
;-----------------------------------------------------------------------

;  Note that the library base begins with a library node

 STRUCTURE EnvoyBase,0
 STRUCT eb_Link,LIB_SIZE
 APTR eb_DOSBase
 APTR eb_UtilityBase
 APTR eb_SysBase
 APTR eb_NIPCBase
 APTR eb_GadToolsBase
 APTR eb_IntuitionBase
 APTR eb_GfxBase
 APTR eb_LayersBase
 APTR eb_SegList
 APTR eb_TopazFont
 LABEL eb_SIZE

 ENDC


