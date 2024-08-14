
#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/rastport.h>
#include <graphics/modeid.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include "pixelread.h"


/*****************************************************************************/


extern struct Library *SysBase;
extern struct Library *GfxBase;


/*****************************************************************************/


struct RGB32
{
    ULONG Red;
    ULONG Green;
    ULONG Blue;
};

struct RastPort *rastPort;
struct RGB      *rgb;
ULONG            startX;
ULONG            endX;
ULONG            currentY;
BOOL             ham;
BOOL             ham6;


/*****************************************************************************/


BOOL OpenReader(struct RastPort *rp, struct ColorMap *cm, ULONG displayMode,
                ULONG x, ULONG y, ULONG w, ULONG h)
{
ULONG         depth;
ULONG         numColors;
BOOL          ehb;
ULONG         i;
struct RGB32 *temp;

    depth     = rp->BitMap->Depth;
    numColors = (1 << depth);

    if (rgb = AllocVec(sizeof(struct RGB) * numColors,MEMF_ANY))
    {
        if (temp = AllocVec(sizeof(struct RGB32) * numColors,MEMF_ANY))
        {
            /* expand color map into straight lookup table */
            GetRGB32(cm,0,numColors,(ULONG *)temp);

            /* convert everything to 8 bit color components */
            for (i = 0; i < numColors; i++)
            {
                rgb[i].Red   = temp[i].Red >> 24;
                rgb[i].Green = temp[i].Green >> 24;
                rgb[i].Blue  = temp[i].Blue >> 24;
            }

            FreeVec(temp);

            /* Now don't yell at me, Chris told me this is guaranteed to work
             * forever, and was supported.
             */
            ehb  = ((displayMode & EXTRAHALFBRITE_KEY) != 0);
            ham  = ((displayMode & HAM_KEY) != 0);
            ham6 = (depth <= 6);

            if (ehb)
            {
                /* if EHB, duplicate the lower 32 colors into 32-63 */
                for (i = 0; i < 32; i++)
                {
                    rgb[i+32].Red   = rgb[i].Red >> 1;
                    rgb[i+32].Green = rgb[i].Green >> 1;
                    rgb[i+32].Blue  = rgb[i].Blue >> 1;
                }
            }

            startX   = x;
            endX     = x + w;
            currentY = y;
            rastPort = rp;

            /* wait till nobody is touching the rastport, just in case */
            WaitBlit();

            return(TRUE);
        }
        FreeVec(rgb);
    }

    return(FALSE);
}


/*****************************************************************************/


VOID CloseReader(VOID)
{
    FreeVec(rgb);
}


/*****************************************************************************/


VOID ReadNextLine(UBYTE *redPlane, UBYTE *greenPlane, UBYTE *bluePlane)
{
ULONG x;
LONG  pen;
UBYTE r, g, b;

    if (ham)
    {
        /* initialize to background color */
        r = rgb[0].Red;
        g = rgb[0].Green;
        b = rgb[0].Blue;
        x = 0;

        if (ham6)
        {
            while (x < startX)
            {
                pen = ReadPixel(rastPort,x,currentY);

                if (pen < 0)
                    pen = 0;

                switch (pen >> 4)
                {
                    case 0: /* Hold */
                            r = rgb[pen].Red;
                            g = rgb[pen].Green;
                            b = rgb[pen].Blue;
                            break;

                    case 1: /* Modify Blue */
                            b = pen & 0xf;
                            b = b | (b << 4);
                            break;

                    case 2: /* Modify Red */
                            r = pen & 0xf;
                            r = r | (r << 4);
                            break;

                    case 3: /* Modify Green */
                            g = pen & 0xf;
                            g = g | (g << 4);
                            break;
                }

                x++;
            }

            while (x < endX)
            {
                pen = ReadPixel(rastPort,x,currentY);

                if (pen < 0)
                    pen = 0;

                switch (pen >> 4)
                {
                    case 0: /* Hold */
                            r = rgb[pen].Red;
                            g = rgb[pen].Green;
                            b = rgb[pen].Blue;
                            break;

                    case 1: /* Modify Blue */
                            b = pen & 0xf;
                            b = b | (b << 4);
                            break;

                    case 2: /* Modify Red */
                            r = pen & 0xf;
                            r = r | (r << 4);
                            break;

                    case 3: /* Modify Green */
                            g = pen & 0xf;
                            g = g | (g << 4);
                            break;
                }

                *redPlane   = r;
                *greenPlane = g;
                *bluePlane  = b;

                redPlane++;
                greenPlane++;
                bluePlane++;

                x++;
            }
        }
        else  /* ham8 */
        {
            while (x < startX)
            {
                pen = ReadPixel(rastPort,x,currentY);

                if (pen < 0)
                    pen = 0;

                switch (pen >> 6)
                {
                    case 0: /* Hold */
                            r = rgb[pen].Red;
                            g = rgb[pen].Green;
                            b = rgb[pen].Blue;
                            break;

                    case 1: /* Modify Blue */
                            b = ((pen & 0x3f) << 2) | (b & 3);
                            break;

                    case 2: /* Modify Red */
                            r = ((pen & 0x3f) << 2) | (r & 3);
                            break;

                    case 3: /* Modify Green */
                            g = ((pen & 0x3f) << 2) | (g & 3);
                            break;
                }
                x++;
            }

            while (x < endX)
            {
                pen = ReadPixel(rastPort,x,currentY);

                if (pen < 0)
                    pen = 0;

                switch (pen >> 6)
                {
                    case 0: /* Hold */
                            r = rgb[pen].Red;
                            g = rgb[pen].Green;
                            b = rgb[pen].Blue;
                            break;

                    case 1: /* Modify Blue */
                            b = ((pen & 0x3f) << 2) | (b & 3);
                            break;

                    case 2: /* Modify Red */
                            r = ((pen & 0x3f) << 2) | (r & 3);
                            break;

                    case 3: /* Modify Green */
                            g = ((pen & 0x3f) << 2) | (g & 3);
                            break;
                }

                *redPlane   = r;
                *greenPlane = g;
                *bluePlane  = b;

                redPlane++;
                greenPlane++;
                bluePlane++;

                x++;
            }
        }
    }
    else
    {
        x = startX;
        while (x < endX)
        {
            pen = ReadPixel(rastPort,x,currentY);
            if (pen < 0)
                pen = 0;

            *redPlane   = rgb[pen].Red;
            *greenPlane = rgb[pen].Green;
            *bluePlane  = rgb[pen].Blue;

            redPlane++;
            greenPlane++;
            bluePlane++;

            x++;
        }
    }

    currentY++;
}
