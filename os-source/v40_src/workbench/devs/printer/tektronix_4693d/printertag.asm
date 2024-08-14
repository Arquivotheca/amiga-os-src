**********************************************************************
*   Tekronix 4693D - Bill Koester                                    *
*                                                                    *
*   Copyright 1985, Commodore-Amiga Inc.   All rights reserved.      *
*   No part of this program may be reproduced, transmitted,          *
*   transcribed, stored in retrieval system, or translated into      *
*   any language or computer language, in any form or by any         *
*   means, electronic, mechanical, magnetic, optical, chemical,      *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated, 983 University Ave. Building #D,   *
*   Los Gatos, California, 95030                                     *
*                                                                    *
**********************************************************************
*
*	printer device dependent code tag
*
*   Source Control
*   --------------
*   $Header: printertag.asm,v 1.1 88/09/19 14:13:00 daveb Exp $
*
*   $Locker:  $
*
*   $Log:	printertag.asm,v $
*   Revision 1.1  88/09/19  14:13:00  daveb
*   no change
*   
*   Revision 1.0  88/05/06  01:06:59  daveb
*   added to rcs
*   
*   
*
**********************************************************************

   SECTION      printer

*------ Included Files -----------------------------------------------

   INCLUDE      "exec/types.i"
   INCLUDE      "exec/nodes.i"
   INCLUDE      "exec/strings.i"

   INCLUDE	"tektronix_4693D_rev.i"

   INCLUDE      "devices/prtbase.i"

*------ Imported Names -----------------------------------------------

   XREF      _Init
   XREF      _Expunge
   XREF      _Open
   XREF      _Close
   XREF      _CommandTable
   XREF      _PrinterSegmentData
   XREF      _DoSpecial
   XREF      _Render
   XREF      _ConvFunc

*------ Exported Names -----------------------------------------------

   XDEF      _PEDData

**********************************************************************

      MOVEQ   #0,D0           ; show error for OpenLibrary()
      RTS
      DC.W	VERSION
      DC.W	REVISION
_PEDData:
      DC.L   printerName
      DC.L   _Init
      DC.L   _Expunge
      DC.L   _Open
      DC.L   _Close
      DC.B   PPC_COLORGFX     ; PrinterClass
      DC.B   PCC_BGR          ; ColorClass
      DC.B   0                ; MaxColumns
      DC.B   0                ; NumCharSets
      DC.W   1                ; NumRows
      DC.L   2440             ; MaxXDots
      DC.L   2492             ; MaxYDots
      DC.W   300              ; XDotsInch
      DC.W   300              ; YDotsInch
      DC.L   _CommandTable    ; Commands
      DC.L   _DoSpecial
      DC.L   _Render
      DC.L   10               ; Timeout
      DC.L   0                ; no ExtendedCharTable
      DS.L   1                ; PrintMode (reserve space)
      DC.L   _ConvFunc        ; ptr to char conversion function

printerName:
      STRING   <'Tektronix_4693D'>

      END
