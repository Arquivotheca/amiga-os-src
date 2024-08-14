#ifndef	DATATYPES_SOUNDCLASS_H
#define	DATATYPES_SOUNDCLASS_H
/*
**  $Id: soundclass.h,v 39.1 92/06/24 01:12:40 davidj Exp Locker: davidj $
**
**  Interface definitions for DataType sound objects.
**
**  (C) Copyright 1992 Commodore-Amiga, Inc.
**	All Rights Reserved
*/

#ifndef	UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif

#ifndef	DATATYPES_DATATYPESCLASS_H
#include <datatypes/datatypesclass.h>
#endif

#ifndef	LIBRARIES_IFFPARSE_H
#include <libraries/iffparse.h>
#endif

/*****************************************************************************/

#define	SOUNDDTCLASS		"sound.datatype"

/*****************************************************************************/

/* Sound attributes */
#define	SDTA_Dummy		(DTA_Dummy + 500)
#define	SDTA_VoiceHeader	(SDTA_Dummy + 1)
#define	SDTA_Sample		(SDTA_Dummy + 2)
#define	SDTA_SampleLength	(SDTA_Dummy + 3)
#define	SDTA_Period		(SDTA_Dummy + 4)
#define	SDTA_Volume		(SDTA_Dummy + 5)
#define	SDTA_Cycles		(SDTA_Dummy + 6)

/*****************************************************************************/

#define CMP_NONE     0
#define CMP_FIBDELTA 1

struct VoiceHeader
{
    ULONG vh_OneShotHiSamples;
    ULONG vh_RepeatHiSamples;
    ULONG vh_SamplesPerHiCycle;
    UWORD vh_SamplesPerSec;
    UBYTE vh_Octaves;
    UBYTE vh_Compression;
    ULONG vh_Volume;
};

/*****************************************************************************/

/* IFF types */
#define ID_8SVX MAKE_ID('8','S','V','X')
#define ID_VHDR MAKE_ID('V','H','D','R')
#define ID_BODY MAKE_ID('B','O','D','Y')

/*****************************************************************************/

#endif	/* DATATYPES_SOUNDCLASS_H */
