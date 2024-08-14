

/************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
************************************************************************/

/************************************************************************
*
* internal.h
*
* Source Control
* ------ -------
* 
* $Header: internal.h,v 27.1 85/06/24 13:21:34 neil Exp $
*
* $Locker:  $
*
************************************************************************/


struct store {
    ULONG unit;
    ULONG *buffer;
    ULONG buffsize;
    ULONG lastcomm;
    ULONG lasttrack;
};

struct MfmFmt {
    UBYTE       mf_type;
    UBYTE       mf_track;
    UBYTE       mf_sector;
    UBYTE       mf_secoff;
};     
 
struct MfmSector {
    UWORD       ms_zeros[2];
    UWORD       ms_funnya1[2];
    struct MfmFmt ms_fmt[2];
    UBYTE       ms_seclabel[TD_LABELSIZE * 2];
    ULONG       ms_hdrsum[2];
    ULONG       ms_datasum[2];
    UBYTE       ms_oddbits[TD_SECTOR];
    UBYTE       ms_evenbits[TD_SECTOR];
};


#define IDTOSECTYPE(id)         (((id)>>24)&0xff)
#define IDTOTRACKNUM(id)        (((id)>>16)&0xff)
#define IDTOSECNUM(id)          (((id)>>8) &0xff)
#define IDTOSECOFFSET(id)       ( (id)     &0xff)
