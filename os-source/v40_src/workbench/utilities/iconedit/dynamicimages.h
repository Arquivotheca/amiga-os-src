#ifndef DYMAMICIMAGES_H
#define DYNAMICIMAGES_H

/*****************************************************************************/

#include <exec/types.h>
#include <intuition/intuition.h>
#include <graphics/rastport.h>
#include <graphics/layers.h>

/*****************************************************************************/

#define AREA_SIZE 400

struct DynamicImage
{
    struct Layer_Info *di_LayerInfo;
    struct Layer      *di_Layer;

    struct Image       di_Image;
    struct BitMap      di_BMap;
    struct RastPort    di_RPort;
    struct AreaInfo    di_Area;
    struct TmpRas      di_TmpRas;

    UBYTE              di_AreaWork[AREA_SIZE];
    UBYTE             *di_RasWork;

    ULONG              di_Flags;	     /* Application Flags -- see defines */
};

typedef struct DynamicImage *DynamicImagePtr;

/* Application Flags --- Used to indicate what parts of the DynamicImage
 * need to be initialized. */

#define DI_FILL		(1L<<0)
#define	DI_LAYER	(1L<<1)

/*****************************************************************************/

VOID InitDynamicImage(DynamicImagePtr,USHORT depth,USHORT width,USHORT height);
BOOL AllocDynamicImage(DynamicImagePtr);
VOID FreeDynamicImage(DynamicImagePtr);

/*****************************************************************************/

#endif /* DYNAMICIMAGES_H */
