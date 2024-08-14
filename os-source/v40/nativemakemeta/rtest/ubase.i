***********************************************************************
*                                                                 
* ubase.i -- definition of utility.library base                  
*                                                               
* Copyright (C) 1985, 1989 Commodore Amiga Inc.  All rights reserved.
*
* $Id: ubase.i,v 36.2 90/11/05 18:55:18 peter Exp $
***********************************************************************

   IFND  UTILITY_BASE_I
UTILITY_BASE_I SET 1


   IFND  EXEC_TYPES_I
   INCLUDE  "exec/types.i"
   ENDC   ; EXEC_TYPES_I

   IFND  EXEC_LISTS_I
   INCLUDE  "exec/lists.i"
   ENDC   ; EXEC_LISTS_I

   IFND  EXEC_LIBRARIES_I
   INCLUDE  "exec/libraries.i"
   ENDC   ; EXEC_LIBRARIES_I


;-----------------------------------------------------------------------
;
; library data structures
;
;-----------------------------------------------------------------------

;  Note that the library base begins with a library node

   STRUCTURE UtilityBase,LIB_SIZE
   UBYTE   ub_Flags
   UBYTE   ub_pad
   ;We are now longword aligned
   ULONG   ub_SysLib
   ULONG   ub_SegList
   LABEL   UtilityBase_SIZEOF


   ENDC  ;UTILITY_BASE_I
