*******************************************************************************
*
*	Source Control
*	--------------
*	$Header: mymacros.asm,v 34.9 87/12/04 19:14:49 bart Exp $
*
*	$Locker:  $
*
*	$Log:	mymacros.asm,v $
*   Revision 34.9  87/12/04  19:14:49  bart
*   checkpoint
*   
*   Revision 34.8  87/12/04  12:09:31  bart
*   checkpoint before adding check for existing dosname on eb_mountlist
*   
*   Revision 34.7  87/10/14  15:53:38  bart
*   10-13 rev 1
*   
*   Revision 34.6  87/10/14  14:16:46  bart
*   beginning update to cbm-source.10.13.87
*   
*   Revision 34.5  87/07/08  14:01:39  bart
*   y
*   
*   Revision 34.4  87/06/11  15:49:03  bart
*   working autoboot 06.11.87 bart
*   
*   Revision 34.3  87/06/03  11:00:04  bart
*   checkpoint
*   
*   Revision 34.2  87/05/31  16:36:29  bart
*   chickpoint
*   
*   Revision 34.1  87/05/29  19:40:06  bart
*   checkpoint
*   
*   Revision 34.0  87/05/29  17:40:33  bart
*   added to rcs for updating
*   
*
*******************************************************************************

      IFND  MY_MACROS_ASM
MY_MACROS_ASM  SET   1
*==========================================================================
*=====  MACROS I LIKE TO USE A LOT ========================================
*==========================================================================

* For declaring external system calls
EXT_SYS     MACRO
      XREF  _LVO\1
      ENDM

* For declaring external data
EXT_DATA     MACRO
      XREF  _LVO\1
      ENDM

* For calling system routines by name (eg. SYS  OpenWindow(A6) )
SYS         MACRO
      JSR   _LVO\1
      ENDM

*==========================================================================

* For fetching the exec pointer into A6
GET_EXECPTR MACRO
      MOVEA.L     _AbsExecBase,A6
      ENDM

* For fetching the intuition pointer into A6 (Requires IntPtr to be defined)
GET_INTPTR  MACRO
      MOVEA.L     IntPtr,A6
      ENDM

* For fetching the graphics pointer into A6 (Requires GfxPtr to be defined)
GET_GFXPTR  MACRO
      MOVEA.L     GfxPtr,A6
      ENDM

* For fetching the dos pointer into A6 (Requires DosPtr to be defined)
GET_DOSPTR  MACRO
      MOVEA.L     DosPtr,A6
      ENDM

*=========================================================================

* For fetching a structure member (GET_MEMBER .W,screen,sc_Width,A0,D0)
* in this example, screen is a longword containing the address of the struct
* A0 is the address register to use and D0 is the destination (can be addr)
GET_MEMBER  MACRO
      MOVEA.L     \2,\4
      MOVE\1      \3(\4),\5
      ENDM

* For fetching address of a structure member (GET_ADDR  screen,sc_BitMap,A0)
* in this example, screen is a longword containing the address of the struct
* A0 is the address register to use for the final address
GET_ADDR    MACRO
      MOVEA.L     \1,\3
      ADDA.L      #\2,\3
      ENDM

*==========================================================================
      ENDC
