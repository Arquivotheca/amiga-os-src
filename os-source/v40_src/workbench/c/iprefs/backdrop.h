#ifndef BACKDROP_H
#define BACKDROP_H


/*****************************************************************************/


#include <exec/types.h>
#include <intuition/classes.h>


/*****************************************************************************/


struct IPatternBitMap
{
    struct BitMap *pbm_BitMap;
    UWORD	   pbm_Width;
    UWORD	   pbm_Height;
    Object	  *pbm_Object;
    BOOL           pbm_DoRemap;
    char           pbm_Name;   /* name starts here */
};


/*****************************************************************************/


VOID UpdateDTBackdrops(VOID);
VOID RemoveDTBackdrops(VOID);
VOID BackdropConfig(UWORD backdropType, struct IPatternBitMap *pbm);
VOID IWBConfig(ULONG type, APTR data);

VOID AttemptCloseDT(VOID);
BOOL AttemptOpenDT(VOID);


/*****************************************************************************/


#endif /* BACKDROP_H */
