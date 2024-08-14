
/* Graphics Rendering Routines */

#include <exec/types.h>
#include <exec/memory.h>
#include <devices/prtbase.h>
#include <devices/printer.h>
#include <prefs/printerps.h>
#include <string.h>
#include <stdio.h>

#include <clib/exec_protos.h>

#include <pragmas/exec_pragmas.h>

#include "driver.h"
#include "convfunc.h"
#include "textbuf.h"
#include "dospecial.h"
#include "pixelread.h"
#include "render.h"


/*****************************************************************************/


extern struct PrinterPSPrefs           ps_pref;
extern far struct PrinterExtendedData  PED;
extern struct PrinterData             *PD;
extern struct Library                 *SysBase;

extern STRPTR  outbuf1;
extern STRPTR  outbuf2;
extern STRPTR  optr;
extern BOOLEAN outflg;
extern BOOLEAN any_outflg;
extern BOOLEAN forced_page;
extern LONG    curcol;
extern LONG    curloc;
extern BOOLEAN beginline;
extern LONG    xdpi0, ydpi0;

static const char *hex = "0123456789ABCDEF";


/*****************************************************************************/


#define MIN_REPEAT 8

static LONG Compress(STRPTR indata, STRPTR outdata, LONG len)
{
STRPTR optr;
STRPTR start;
STRPTR curptr;
LONG   i,j,k,l;
char   c;

    optr = outdata;
    curptr = indata;

    k = 0;
    start = curptr;

    for (i=0;i<=len;i++)
    {
        c = *curptr;

        if (i != len)
        {
            // Look forward for a repeat
            for (j=1; j < (len-i) && curptr[j] == c && j < 250;j++);
        }

        if (((j > MIN_REPEAT) || (i == len) || (k > 250)) && (k > 0))
        {
            *(optr++) = hex[((k & 0xf0) >> 4) & 0xf];
            *(optr++) = hex[k & 0x0f];

            for (l=0;l<k;l++)
            {
                *(optr++) = hex[((*start & 0xf0) >> 4) & 0xf];
                *(optr++) = hex[*start & 0x0f];

                ++start;
            }

            // Re-initialize the run
            k = 0;
            start = curptr;
        }


        if (i == len)
        {
            // Indicate end of the row
            *(optr++) = 'F';
            *(optr++) = 'F';
            break;
        }

        if (j > MIN_REPEAT)
        {
            // This is a big enought repeat

            // Save the length and character
            *(optr++) = '0';
            *(optr++) = '0';
            *(optr++) = hex[((j & 0xf0) >> 4) & 0xf];
            *(optr++) = hex[j & 0x0f];
            *(optr++) = hex[((c & 0xf0) >> 4) & 0xf];
            *(optr++) = hex[c & 0x0f];

            // Reset the run counter
            curptr += j;
            k = 0;
            start = curptr;

            i += (j - 1);
            continue;
        }

        // Increment for the run
        ++curptr;
        ++k;
    }

    return (optr - outdata);
}


/*****************************************************************************/


static LONG HexLine(STRPTR ibuf, STRPTR obuf, LONG len, STRPTR old)
{
LONG   i,j;
STRPTR optr;

    if (old)
    {
        // Check if this line is the same as the last
        for (i=0;i < len && ibuf[i] == old[i];i++);

        if (i == len)
        {
            // Save the line type
            obuf[0] = '0';
            obuf[1] = '3';

            // Point past the buffer data
            optr = obuf + 2;

            goto all_done;
        }
    }

    // First check if the line is all the same color
    for (j = ibuf[0],i=1;i<len && j == ibuf[i];i++);

    if (i == len)
    {
        // The line is all one color

        // Save the line type
        obuf[0] = '0';
        obuf[1] = '2';

        // Save the repeat character
        obuf[2] = hex[((j & 0xf0) >> 4) & 0xf];
        obuf[3] = hex[j & 0x0f];

        // Point past the buffer data
        optr = obuf + 4;

        goto all_done;
    }

    // This value is the cut-off to determine if the line is
    // compressed enough to output compressed.
    // The normal line is twice the size of the input since it
    // is converted to ASCII hex.  So the cut-off is 1 & 1/2
    // the size of the input.  There is a trade-off between
    // the decompression speed and the transmittion speed,
    // on most printers the decompression is pretty slow.
    j = len + (len >> 1);

    if ((i = Compress(ibuf,obuf+2,len)) < j)
    {
        // Indicate that this is a run length compressed line
        obuf[0] = '0';
        obuf[1] = '1';

        // Include the line type
        i += 2;

        // Point past the buffer data
        optr = obuf + i;
    }
    else
    {
        optr = obuf;

        // Indicate this is an uncompressed line
        *(optr++) = '0';
        *(optr++) = '0';

        for (i=0;i<len;i++)
        {
            j = *(ibuf++);

            *(optr++) = hex[((j & 0xf0) >> 4) & 0xf];
            *(optr++) = hex[j & 0x0f];
        }
    }

all_done:
    *(optr++) = '\n';
    *optr = 0;

    return (optr - obuf);
}


/*****************************************************************************/


static VOID MakeBW(UBYTE *redPlane, UBYTE *bluePlane, UBYTE *greenPlane,
                   UBYTE *resultPlane, ULONG width, ULONG threshold)
{
ULONG i, value;

    threshold = threshold | (threshold << 4);

    for (i = 0; i < width; i++)
    {
        value = (redPlane[i]*77 + greenPlane[i]*151 + bluePlane[i]*28) / 256;
        if (value >= threshold)
            resultPlane[i] = 255;
        else
            resultPlane[i] = 0;
    }
}


/*****************************************************************************/


static VOID MakeGreyScale(UBYTE *redPlane, UBYTE *bluePlane, UBYTE *greenPlane,
                          UBYTE *resultPlane, ULONG width)
{
ULONG i;

    for (i = 0; i < width; i++)
        resultPlane[i] = (redPlane[i]*77 + greenPlane[i]*151 + bluePlane[i]*28) / 256;
}


/*****************************************************************************/


LONG __saveds __stdargs DriverRender(struct IODRPReq *request, LONG x, LONG y,
                                     UBYTE status)
{
ULONG              passes;
LONG               bytesPerRow;
LONG               i, j;
LONG               len;
LONG               err;
static const LONG  dpi[] = {100, 150, 300, 600, 1200, 2400};
UBYTE             *colorPlanes[3];
UBYTE             *conversionPlane;
APTR               hexBuf[2];
APTR               previous;
BOOL               lineToggle;
LONG               xloc;
char               tmpbuf[100];
char               tmp1[20], tmp2[20];
LONG               k;
LONG               w,h;

    if (status == 5)
    {
        // Pre-master Initialization
        i = ((x & SPECIAL_DENSITYMASK) >> 8) - 2;

        if (i > 5 || i < 0)
        {
            ps_pref.ps_HorizontalDPI = xdpi0;
            ps_pref.ps_VerticalDPI   = ydpi0;
        }
        else
        {
            ps_pref.ps_HorizontalDPI = dpi[i];
            ps_pref.ps_VerticalDPI   = dpi[i];
        }

        SetPED();
    }

    if (status)
        return (PDERR_NOERR);

    if (request->io_Special & SPECIAL_NOPRINT)
        return (PDERR_NOERR);

    // Make sure we are not already past the end of the page
    CheckNewPage(FALSE,TRUE);

    /* Flush any text waiting to be output */
    ProcChar(0);

    PSFlush();

    // Normalize the height and width to 1/1000 of a point
    w = (72000 * x) / ps_pref.ps_HorizontalDPI;
    h = (72000 * y) / ps_pref.ps_VerticalDPI;

    if (request->io_Special & SPECIAL_CENTER)
    {
        /* Calculate the desired destination width
         * from the printer device specified values
         */
        w = (2*w) - ((72000*PED.ped_MaxXDots) / ps_pref.ps_HorizontalDPI);
    }
    else
    {
        // Remove the left edge offset
        w -= 7200 * PREF.PrintXOffset;
    }

    // Swamp the width and height
    if (PREF.PrintAspect == ASPECT_VERT)
    {
        i = w;
        w = h;
        h = i;
    }

    bytesPerRow = ((request->io_SrcWidth * 8) + 7) / 8;

    /* Initialize the output to blank */
    *outbuf2 = 0;

    /* Check if this dump will fit on this page if not move to an other */
    if (PREF.PrintAspect == ASPECT_VERT)
    {
        i = w;
        j = h;
    }
    else
    {
        i = h;
        j = w;
    }

    if (ps_pref.ps_Centering == CENT_VERT || ps_pref.ps_Centering == CENT_BOTH)
    {
        k = ps_pref.ps_PaperHeight -
            (ps_pref.ps_TopMargin + ps_pref.ps_BottomMargin);

        k -= ps_pref.ps_TopEdge;

        curloc = (ps_pref.ps_PaperHeight -
            ps_pref.ps_TopMargin - ps_pref.ps_TopEdge) -
            (k - i)/2;
    }
    else
    {
        if (!outflg)
            curloc = ps_pref.ps_PaperHeight - ps_pref.ps_TopEdge;
    }

    curloc -= i;

    xloc = ps_pref.ps_LeftEdge;

    if (ps_pref.ps_Centering == CENT_HORIZ ||
        ps_pref.ps_Centering == CENT_BOTH ||
        request->io_Special & SPECIAL_CENTER)
        xloc += (ps_pref.ps_PaperWidth - j - xloc)/2;

    strcat(outbuf2,"save\n");

    /* Clear away the current text clipping path */
    strcat(outbuf2,"initclip\n");

    /* Make sure that there is a string setup to handle a line of
     * the image.
     */
    sprintf(tmpbuf,"/picstr %ld string def\n",bytesPerRow);
    strcat(outbuf2,tmpbuf);

    FixedPointToStr(xloc,tmp1);
    FixedPointToStr(curloc,tmp2);
    sprintf(tmpbuf,"%s %s translate\n",tmp1,tmp2);
    strcat(outbuf2,tmpbuf);

    /* Check if the image is to be rotated */
    if (PREF.PrintAspect == ASPECT_VERT)
    {
        FixedPointToStr(h,tmp1);
        strcat(outbuf2,tmp1);
        strcat(outbuf2," 0 translate\n");

        strcat(outbuf2,"90 rotate\n");
    }

    /* Scale the image */
    FixedPointToStr(w,tmp1);
    FixedPointToStr(h,tmp2);
    sprintf(tmpbuf,"%s %s scale\n",tmp1,tmp2);
    strcat(outbuf2,tmpbuf);

    sprintf(tmpbuf,"%ld %ld 8 [%ld 0 0 -%ld 0 %ld] ",
        request->io_SrcWidth,request->io_SrcHeight,
        request->io_SrcWidth,request->io_SrcHeight,request->io_SrcHeight);
    strcat(outbuf2,tmpbuf);

    if (ps_pref.ps_Shading == SHAD_COLOR)
    {
        sprintf(tmpbuf,"/picstr_r %ld string def\n",bytesPerRow);
        strcat(outbuf2,tmpbuf);
        sprintf(tmpbuf,"/picstr_g %ld string def\n",bytesPerRow);
        strcat(outbuf2,tmpbuf);
        sprintf(tmpbuf,"/picstr_b %ld string def\n",bytesPerRow);
        strcat(outbuf2,tmpbuf);

        strcat(outbuf2,"{getcstr_r} {getcstr_g} {getcstr_b} true 3 colorimage\n");
        passes = 3;
    }
    else
    {
        strcat(outbuf2,"{getcstr} image\n");
        passes = 1;
    }

    err = PDERR_BUFFERMEMORY;

    colorPlanes[0]  = AllocVec(request->io_SrcWidth + 7,MEMF_ANY);
    colorPlanes[1]  = AllocVec(request->io_SrcWidth + 7,MEMF_ANY);
    colorPlanes[2]  = AllocVec(request->io_SrcWidth + 7,MEMF_ANY);
    conversionPlane = AllocVec(bytesPerRow,MEMF_ANY);  /* wasted memory when doing a color dump, oh well... :-) */
    hexBuf[FALSE]   = AllocVec(2*bytesPerRow+256,MEMF_ANY);
    hexBuf[TRUE]    = AllocVec(2*bytesPerRow+256,MEMF_ANY);

    if (colorPlanes[0] && colorPlanes[1] && colorPlanes[2] && conversionPlane
     && hexBuf[FALSE] && hexBuf[TRUE]
     && OpenReader(request->io_RastPort,request->io_ColorMap,request->io_Modes,
                   request->io_SrcX,request->io_SrcY,request->io_SrcWidth,request->io_SrcHeight))
    {
        /* Send the image command */
        PWRITE(outbuf2,strlen(outbuf2));

        previous   = NULL;
        lineToggle = FALSE;

        /* Dump each row of data */
        for (i = 0; i < request->io_SrcHeight; i++)
        {
            ReadNextLine(colorPlanes[0],colorPlanes[1],colorPlanes[2]);

            for (j = 0; j < passes; j++)
            {
                switch (ps_pref.ps_Shading)
                {
                    case SHAD_COLOR    : len = HexLine(colorPlanes[j],hexBuf[lineToggle],bytesPerRow,previous);
                                         break;

                    case SHAD_GREYSCALE: MakeGreyScale(colorPlanes[0],colorPlanes[1],colorPlanes[2],conversionPlane,request->io_SrcWidth);
                                         len = HexLine(conversionPlane,hexBuf[lineToggle],bytesPerRow,previous);
                                         break;

                    case SHAD_BW       : MakeBW(colorPlanes[0],colorPlanes[1],colorPlanes[2],conversionPlane,request->io_SrcWidth,PREF.PrintThreshold);
                                         len = HexLine(conversionPlane,hexBuf[lineToggle],bytesPerRow,previous);
                                         break;
                }

                PWAIT();

                if ((err = PWRITE(hexBuf[lineToggle],len)) != PDERR_NOERR)
                {
                    CloseReader();
                    goto ret;
                }

                previous = hexBuf[lineToggle];
                lineToggle = !lineToggle;
            }
        }

        CloseReader();

        PWAIT();

        outflg     = TRUE;
        any_outflg = TRUE;

        strcpy(outbuf1,"restore\n");

        if (!(request->io_Special & SPECIAL_NOFORMFEED))
        {
            CheckNewPage(TRUE,FALSE);
            forced_page = TRUE;
        }

        MarginClip(FALSE);

        beginline = TRUE;
        curcol    = 0;

        err = PWRITE(outbuf1,strlen(outbuf1));
    }

    /* Setup the buffers to do character I/O */
    *outbuf1 = 0;
    optr = outbuf2;

    if (err == PDERR_NOERR)
        err = PDERR_TOOKCONTROL;

ret:
    PWAIT();

    FreeVec(colorPlanes[0]);
    FreeVec(colorPlanes[1]);
    FreeVec(colorPlanes[2]);
    FreeVec(conversionPlane);
    FreeVec(hexBuf[FALSE]);
    FreeVec(hexBuf[TRUE]);

    return (err);
}

