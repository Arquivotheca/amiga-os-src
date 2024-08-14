**
**  $Filename$
**  $Release: 1.4 $
**  $Revision: 1.1 $
**  $Date: 92/08/20 13:45:04 $
**
**  ResidentTag and module identification
**
**  (C) Copyright 1985,1986,1987,1988,1992 Commodore-Amiga, Inc.
**      All Rights Reserved
**
    SECTION strap

*   Included Files

    INCLUDE     "exec/types.i"
    INCLUDE     "exec/resident.i"

    INCLUDE     "strap_rev.i"

*   Imported Names

    XREF        RMInit
    XREF        SMInit
    XREF        SMEndModule


**********************************************************************

rmResident:
        dc.w    RTC_MATCHWORD
        dc.l    rmResident
        dc.l    smResident
        dc.b    RTF_COLDSTART
        dc.b    VERSION
        dc.b    0
        dc.b    -40     ; boot late in the sequence
        dc.l    rlName
        dc.l    rlID
        dc.l    RMInit


smResident:
        dc.w    RTC_MATCHWORD
        dc.l    smResident
        dc.l    SMEndModule
        dc.b    RTF_COLDSTART
        dc.b    VERSION
        dc.b    0
        dc.b    -60     ; boot late in the sequence
        dc.l    smName
        dc.l    smID
        dc.l    SMInit

rlName:
        dc.b    'romboot',0
rlID:
        dc.b    'romboot',13,10,0

smName:
        dc.b    'strap',0
smID:
        VSTRING

        ds.w    0

    END
