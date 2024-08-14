***********************************************************************
*                                                                 
* myclassbase.i -- library base for myclasslib.library
*                                                               
* Copyright (C) 1985, 1989, 1990 Commodore Amiga Inc. 
*	All rights reserved.
*
***********************************************************************

*** MUST stay in sync with myclassbase.h	***

   IFND  MYCLASS_BASE_I
MYCLASS_BASE_I SET 1


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
   UBYTE   mlb_Flags
   UBYTE   mlb_pad
   ;We are now longword aligned
   ULONG   mlb_SegList
   APTR	   mlb_MyClass
   LABEL   MyLibBase_SIZEOF


   ENDC  ;MYCLASS_BASE_I
