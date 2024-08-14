
/* includes */
#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <string.h>

/* prototypes */
#include <clib/macros.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/layers_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/layers_pragmas.h>

/* application includes */
#include "dynamicimages.h"

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

extern struct Library *SysBase;
extern struct Library *LayersBase;
extern struct Library *GfxBase;

/*****************************************************************************/

VOID InitDynamicImage (DynamicImagePtr di, USHORT d, USHORT w, USHORT h)
{
    WORD i;

    di->di_Image.LeftEdge = 0;
    di->di_Image.TopEdge = 0;
    di->di_Image.Width = w;
    di->di_Image.Height = h;
    di->di_Image.Depth = d;
    di->di_Image.ImageData = NULL;
    di->di_Image.PlanePick = (1 << d) - 1;
    di->di_Image.PlaneOnOff = 0;
    di->di_Image.NextImage = NULL;
    di->di_RasWork = NULL;
    di->di_LayerInfo = NULL;
    di->di_Layer = NULL;

    InitBitMap (&di->di_BMap, (BYTE) d, (SHORT) w, (SHORT) h);
    InitRastPort (&di->di_RPort);
    InitArea (&di->di_Area, di->di_AreaWork, AREA_SIZE / 5);

    di->di_RPort.BitMap = &di->di_BMap;
    di->di_RPort.AreaInfo = &di->di_Area;

    for (i = 0; i < AREA_SIZE; i++)
	di->di_AreaWork[i] = 0;
}

/*****************************************************************************/

BOOL AllocDynamicImage (DynamicImagePtr di)
{
    USHORT depth = di->di_Image.Depth;
    struct BitMap *bm = &di->di_BMap;
    USHORT width = di->di_Image.Width;
    USHORT height = di->di_Image.Height;
    ULONG i, planes, data;

    planes = RASSIZE (width, height);

    if (!(data = (ULONG) AllocVec (planes * depth, MEMF_CHIP | MEMF_CLEAR)))
    {
	return (FALSE);
    }

    di->di_Image.ImageData = (APTR) data;

    for (i = 0; i < di->di_Image.Depth; i++)
    {
	bm->Planes[i] = (PLANEPTR) (data + i * planes);
    }

    if (di->di_Flags & DI_LAYER)
    {
	if (!(di->di_LayerInfo = NewLayerInfo ()))
	{
	    FreeDynamicImage (di);
	    return (FALSE);
	}

	if (!(di->di_Layer = CreateUpfrontLayer (di->di_LayerInfo, &di->di_BMap,
					      0, 0, (width - 1), (height - 1),
						 LAYERSIMPLE, NULL)))
	{
	    FreeDynamicImage (di);
	    return (FALSE);
	}

	di->di_RPort.Layer = di->di_Layer;
    }

    if (di->di_Flags & DI_FILL)
    {
	DB (kprintf ("Width=%ld, %ld\n", (LONG)width, (LONG) ((width + 63) & ~63) ));

	if (!(di->di_RasWork = AllocRaster (((width + 63) & ~63), height)))
	{
	    FreeDynamicImage (di);
	    return (FALSE);
	}

	InitTmpRas (&di->di_TmpRas, di->di_RasWork, planes);
	di->di_RPort.TmpRas = &di->di_TmpRas;
    }

    return (TRUE);
}

/*****************************************************************************/

VOID FreeDynamicImage (DynamicImagePtr di)
{
    if (di)
    {
	if (di->di_LayerInfo)
	{
	    if (di->di_Layer)
	    {
		DeleteLayer (NULL, di->di_Layer);
	    }
	    DisposeLayerInfo (di->di_LayerInfo);
	}

	if (di->di_RasWork)
	{
	    FreeRaster (di->di_RasWork, ((di->di_Image.Width + 63) & ~63), di->di_Image.Height);
	}

	FreeVec (di->di_Image.ImageData);

	di->di_Layer = NULL;
	di->di_LayerInfo = NULL;
	di->di_Image.ImageData = NULL;
	di->di_RasWork = NULL;
    }
}
