
/* Purpose: Open the printer driver, allocating all needed buffers,
 *          and reading in the postscript prologue file.
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <devices/prtbase.h>
#include <devices/printer.h>
#include <graphics/gfxbase.h>
#include <prefs/prefhdr.h>
#include <prefs/printerps.h>
#include <libraries/iffparse.h>
#include <intuition/preferences.h>
#include <stdio.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "textbuf.h"
#include "dospecial.h"
#include "convfunc.h"
#include "driver.h"


/*****************************************************************************/


#define PS_PROLOGUE    "DEVS:postscript_init.ps"
#define PS_PREFERENCES "ENV:Sys/printerps.prefs"


/*****************************************************************************/


static STRPTR scr_types[] =
{
    NULL,
    "dot_scr\n",
    "vert_scr\n",
    "horiz_scr\n",
    "diag_scr\n",
};

static STRPTR paper_tray[] =
{
    "letter\n",
    "legal\n",
    "a4\n",
    "custom\n",
};


/*****************************************************************************/


struct Library                        *SysBase;
struct Library                        *DOSBase;
struct Library                        *GfxBase;
struct Library                        *UtilityBase;
struct PrinterData                    *PD;
struct PrinterPSPrefs                  ps_pref;
extern far struct PrinterExtendedData  PED;

extern LONG  pshd_cnt;
extern UBYTE fontdefs[MAX_FONTS];

STRPTR  outbuf1;
STRPTR  outbuf2;
LONG    curcol;
LONG    curloc;
LONG    xdpi0, ydpi0;
LONG    TopPage;
BOOLEAN initflg;
BOOLEAN outflg;
BOOLEAN any_outflg;
BOOLEAN beginline;
BOOLEAN forced_page;


/*****************************************************************************/


VOID SetPED(VOID)
{
    if (ps_pref.ps_Orientation == ORIENT_PORTRAIT)
    {
        PED.ped_XDotsInch = ps_pref.ps_HorizontalDPI;
        PED.ped_YDotsInch = ps_pref.ps_VerticalDPI;
        PED.ped_MaxXDots  = (ps_pref.ps_PaperWidth  * ps_pref.ps_HorizontalDPI) / 72000;
        PED.ped_MaxYDots  = (ps_pref.ps_PaperHeight * ps_pref.ps_VerticalDPI) / 72000;
    }
    else
    {
        PED.ped_XDotsInch = ps_pref.ps_VerticalDPI;
        PED.ped_YDotsInch = ps_pref.ps_HorizontalDPI;
        PED.ped_MaxXDots  = (ps_pref.ps_PaperHeight * ps_pref.ps_VerticalDPI) / 72000;
        PED.ped_MaxYDots  = (ps_pref.ps_PaperWidth  * ps_pref.ps_HorizontalDPI) / 72000;
    }
}


/*****************************************************************************/


/* This routine maps the postscript preferences into the old prefs
 * structure that the printer.device uses to calculate graphic dumps
 * and positions.
 */
static VOID MapPrefs(VOID)
{
static const LONG pitch[] = {ELITE, FINE, PICA};
LONG              cpi;

    PREF.PrintPitch = pitch[ps_pref.ps_Pitch];

    switch (PREF.PrintPitch)
    {
        case ELITE: cpi = 12; break;
        case FINE : cpi = 15; break;
        case PICA : cpi = 10; break;
    }

    PREF.PrintLeftMargin   = (((ps_pref.ps_LeftMargin - MARG_LEFT) * cpi)/72000)+1;
    PREF.PrintRightMargin  = (((ps_pref.ps_PaperWidth - ps_pref.ps_RightMargin) * cpi)/72000);
    PREF.PaperLength       = (ps_pref.ps_PaperHeight - ps_pref.ps_TopMargin - ps_pref.ps_BottomMargin) / (ps_pref.ps_FontPointSize + ps_pref.ps_Leading);
    PREF.PrintFlags       &= ~DIMENSIONS_MASK;

    if (ps_pref.ps_Aspect == ASP_HORIZ)
        PREF.PrintAspect = ASPECT_HORIZ;
    else
        PREF.PrintAspect = ASPECT_VERT;

    PED.ped_NumRows  = (ps_pref.ps_PaperHeight - (ps_pref.ps_TopMargin + ps_pref.ps_BottomMargin)) / (ps_pref.ps_FontPointSize + ps_pref.ps_Leading);
    PREF.PaperLength = PED.ped_NumRows;
    PREF.PaperType   = SINGLE;

    switch (ps_pref.ps_ScalingType)
    {
        case ST_ASPECT_ASIS: /* None */
                             PREF.PrintFlags = IGNORE_DIMENSIONS;
                             break;

        case ST_ASPECT_WIDE: PREF.PrintFlags     = BOUNDED_DIMENSIONS;
                             PREF.PrintMaxWidth  = ps_pref.ps_Width / 7200;
                             PREF.PrintMaxHeight = 65535;
                             break;

        case ST_ASPECT_TALL: PREF.PrintFlags     = BOUNDED_DIMENSIONS;
                             PREF.PrintMaxWidth  = 65535;
                             PREF.PrintMaxHeight = ps_pref.ps_Height / 7200;
                             break;

        case ST_ASPECT_BOTH: PREF.PrintFlags     = BOUNDED_DIMENSIONS;
                             PREF.PrintMaxWidth  = ps_pref.ps_Width / 7200;
                             PREF.PrintMaxHeight = ps_pref.ps_Height / 7200;
                             break;

        case ST_FITS_WIDE  : PREF.PrintFlags     = ABSOLUTE_DIMENSIONS;
                             PREF.PrintMaxWidth  = ps_pref.ps_Width / 7200;
                             PREF.PrintMaxHeight = 0;
                             break;

        case ST_FITS_TALL  : PREF.PrintFlags = ABSOLUTE_DIMENSIONS;
                             PREF.PrintMaxWidth = 0;
                             PREF.PrintMaxHeight = ps_pref.ps_Height / 7200;
                             break;

        case ST_FITS_BOTH  : PREF.PrintFlags = ABSOLUTE_DIMENSIONS;
                             PREF.PrintMaxWidth = ps_pref.ps_Width / 7200;
                             PREF.PrintMaxHeight = ps_pref.ps_Height / 7200;
                             break;
    }

    PREF.PrintXOffset = ps_pref.ps_LeftEdge / 7200;

    if (ps_pref.ps_Image == IM_NEGATIVE)
        PREF.PrintImage = IMAGE_NEGATIVE;
    else
        PREF.PrintImage = IMAGE_POSITIVE;

    if (ps_pref.ps_ScalingMath == SM_INTEGER)
        PREF.PrintFlags |= INTEGER_SCALING;
    else
        PREF.PrintFlags &= ~INTEGER_SCALING;

    xdpi0 = ps_pref.ps_HorizontalDPI;
    ydpi0 = ps_pref.ps_VerticalDPI;
}


/*****************************************************************************/


#define IFFPrefChunkCnt 2
static LONG IFFPrefChunks[] =
{
    ID_PREF, ID_PRHD,
    ID_PREF, ID_PSPD,
};

static VOID LoadPSPrefs(struct PrinterPSPrefs *postpref)
{
struct IFFHandle   *iff;
struct ContextNode *cn;
struct PrefHeader   phead;
struct Library     *IFFParseBase;

    if (IFFParseBase = OpenLibrary("iffparse.library",37))
    {
        if (iff = AllocIFF())
        {
            if (iff->iff_Stream = (ULONG)Open(PS_PREFERENCES,MODE_OLDFILE))
            {
                InitIFFasDOS(iff);

                if (!OpenIFF(iff,IFFF_READ))
                {
                    if (!ParseIFF(iff,IFFPARSE_STEP))
                    {
                        cn = CurrentChunk(iff);

                        if (cn->cn_ID == ID_FORM && cn->cn_Type == ID_PREF)
                        {
                            if (!StopChunks(iff,IFFPrefChunks,IFFPrefChunkCnt))
                            {
                                while (TRUE)
                                {
                                    if (ParseIFF(iff,IFFPARSE_SCAN))
                                        break;

                                    cn = CurrentChunk(iff);
                                    if ((cn->cn_ID == ID_PRHD) && (cn->cn_Type == ID_PREF))
                                    {
                                        if (ReadChunkBytes(iff,&phead,sizeof(struct PrefHeader)) != sizeof(struct PrefHeader))
                                            break;

                                        if (phead.ph_Version)
                                            break;
                                    }
                                    else if ((cn->cn_ID == ID_PSPD) && (cn->cn_Type == ID_PREF))
                                    {
                                        if (ReadChunkBytes(iff,postpref,sizeof(struct PrinterPSPrefs)) != sizeof(struct PrinterPSPrefs))
                                            break;
                                    }
                                }
                            }
                        }
                    }
                    CloseIFF(iff);
                }
                Close(iff->iff_Stream);
            }
            FreeIFF(iff);
        }
        CloseLibrary(IFFParseBase);
    }
}


/*****************************************************************************/


static BOOL GetPSPrefs(VOID)
{
BPTR   fh;
STRPTR prologue;
LONG   prologueSize;
BOOL   result = FALSE;

    /* default prefs */
    ps_pref.ps_DriverMode    = DM_POSTSCRIPT;
    ps_pref.ps_PaperFormat   = 0;           	    /* Letter */
    ps_pref.ps_Copies        = 1;
    ps_pref.ps_PaperWidth    = 85 * 72 * 100;       /* 8.5 inches */
    ps_pref.ps_PaperHeight   = 11 * 72 * 1000;      /* 11 inches  */
    ps_pref.ps_HorizontalDPI = 300;
    ps_pref.ps_VerticalDPI   = 300;
    ps_pref.ps_Font          = FONT_COURIER;
    ps_pref.ps_Pitch         = PITCH_NORMAL;
    ps_pref.ps_Orientation   = ORIENT_PORTRAIT;
    ps_pref.ps_Tab           = TAB_INCH;
    ps_pref.ps_LeftMargin    = 10 * 72 * 100;       /* 1.0 inches */
    ps_pref.ps_RightMargin   = 10 * 72 * 100;       /* 1.0 inches */
    ps_pref.ps_TopMargin     = 10 * 72 * 100;       /* 1.0 inches */
    ps_pref.ps_BottomMargin  = 10 * 72 * 100;       /* 1.0 inches */
    ps_pref.ps_FontPointSize = 10 * 1000;
    ps_pref.ps_Leading       = 2  * 1000;
    ps_pref.ps_LeftEdge      = 10 * 72 * 100;       /* 1.0 inches */
    ps_pref.ps_TopEdge       = 10 * 72 * 100;       /* 1.0 inches */
    ps_pref.ps_Width         = 65 * 72 * 100;       /* 6.5 inches */
    ps_pref.ps_Height        = 90 * 72 * 100;       /* 9 inches */
    ps_pref.ps_Image         = IM_POSITIVE;
    ps_pref.ps_Shading       = SHAD_BW;
    ps_pref.ps_Dithering     = DITH_DEFAULT;
    ps_pref.ps_Transparency  = TRANS_COLOR0;
    ps_pref.ps_Aspect        = ASP_HORIZ;
    ps_pref.ps_ScalingType   = ST_ASPECT_BOTH;
    ps_pref.ps_ScalingMath   = SM_INTEGER;
    ps_pref.ps_Centering     = CENT_BOTH;

    LoadPSPrefs(&ps_pref);

    MapPrefs();
    SetPED();

    if (fh = Open(PS_PROLOGUE,MODE_OLDFILE))
    {
        Seek(fh,0,OFFSET_END);
        prologueSize = Seek(fh,0,OFFSET_BEGINNING);

        if (prologueSize++)
        {
            if (prologue = AllocVec(prologueSize,MEMF_ANY))
            {
                if (Read(fh,prologue,(prologueSize-1)) == (prologueSize-1))
                {
                    prologue[prologueSize-1] = 0;
                    if (PSWrite(prologue))
                        result = TRUE;
                }
                FreeVec(prologue);
            }
        }

        Close(fh);
    }

    return(result);
}


/*****************************************************************************/


LONG __saveds __stdargs DriverOpen(VOID)
{
char tmp[100];

    /* Indicate that there has been no output */
    any_outflg  = FALSE;
    outflg      = FALSE;
    forced_page = FALSE;

    /* Indicate that the printer needs to be initialized */
    initflg = TRUE;

    /* Start the look for the auto pass-through */
    pshd_cnt = 0;

    InitTextBuf();

    /* Allocate space for the output buffers */
    if (outbuf1 = AllocVec(MAX_WIDTH*2,MEMF_CLEAR))
    {
        /* use the second half of the memory for the second buffer */
        outbuf2 = &outbuf1[MAX_WIDTH];

        /* Load the prologue file and preferences */
        if (GetPSPrefs())
        {
            /* Clear the font definition flags */
            memset(fontdefs,0,sizeof(fontdefs));

            PSWrite("\nAmigaDict begin\n\n");

            /* Define the number of copies */
            if (ps_pref.ps_Copies > 1)
            {
                sprintf(tmp,"/#copies %ld def\n",ps_pref.ps_Copies);
                PSWrite(tmp);
            }

            /* Set the graphics screen */
            if (ps_pref.ps_Dithering)
                PSWrite(scr_types[ps_pref.ps_Dithering]);

            /* Set paper tray */
            PSWrite(paper_tray[ps_pref.ps_PaperFormat]);

            if (ps_pref.ps_Orientation == ORIENT_LANDSCAPE)
            {
                PSWrite("/ls {");

                /* Handle the page rotation */
                FixedPointToStr(ps_pref.ps_PaperHeight,tmp);
                PSWrite(tmp);
                PSWrite(" 0 translate 90 rotate} def\n");
                PSWrite("ls\n");
                PSWrite("/SP {showpage ls} def\n");
            }
            else
            {
                PSWrite("/SP {showpage} def\n");
            }

            FixedPointToStr(ps_pref.ps_LeftMargin,tmp);
            PSWrite("/L ");
            PSWrite(tmp);
            PSWrite(" def\n");

            MarginClip(TRUE);

            /* Calculate the total units for the page */
            TopPage   = ps_pref.ps_PaperHeight - ps_pref.ps_TopMargin;
            curloc    = TopPage;
            curcol    = 0;
            beginline = TRUE;

            DoCmd(aRIS,NULL,NULL,NULL);

            return(0);
        }
        FreeVec(outbuf1);
    }

    return(-1);
}


/*****************************************************************************/


LONG __saveds __stdargs DriverClose(VOID)
{
    if (ps_pref.ps_DriverMode == DM_PASSTHROUGH)
    {
        if (any_outflg)
        {
            /* Send an EOF */
            PWRITE("\004",1);
        }
    }
    else
    {
        if (any_outflg)
        {
            /* Flush the current line buffer */
            ProcChar(0);

            /* Actually write the stuff out */
            PSWrite(outbuf1);

            /* If there was any output, eject the page */
            if (outflg)
                PSWrite("showpage\n");

            /* EOF */
            PSWrite("end\n\004");

            /* Flush all existing memory buffers */
            PSFlush();
        }
    }

    /* Wait for the printer device to finish writing everything */
    PWAIT();
    PWAIT();

    FreeVec(outbuf1);

    return (0);
}
