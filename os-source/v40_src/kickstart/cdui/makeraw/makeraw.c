/*******************************************************************************
 *
 * (c) Copyright 1993 Commodore-Amiga, Inc.  All rights reserved.
 *
 * This software is provided as-is and is subject to change; no warranties
 * are made.  All use is at your own risk.  No liability or responsibility
 * is assumed.
 *
 * MakeRaw - the main code to convert the files needed for the animation to
 * the file format used by playraw.
 *
 ******************************************************************************/

#define LIBVERSION 39

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define NO_CTRL_C 1
#if NO_CTRL_C == 1
int CXBRK (void) { return (0) ; }   /* disable ctrl-c */
#endif

#include <exec/types.h>
#include <exec/exec.h>
#include <clib/exec_protos.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <clib/dos_protos.h>
#include <intuition/screens.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <clib/intuition_protos.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/displayinfo.h>
#include <clib/graphics_protos.h>
#include <libraries/iffparse.h>
#include <clib/iffparse_protos.h>

#include "playanim.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/*********************************************************************/
/*                            GLOBAL VARIABLES                       */
/*********************************************************************/

struct IntuitionBase *IntuitionBase = NULL ;
struct GfxBase *GfxBase = NULL ;
struct Library *IFFParseBase = NULL;

struct RDArgs *rdargs = NULL;
BPTR OutFile = NULL;
struct IFFHandle *iff = NULL;
UBYTE *inbuff = NULL;
struct ColorMap *cm = NULL;
BPTR ScriptFile = NULL;

/**********************************************************************/
/*                                                                    */
/* void Error (char *String)                                          */
/* Print string and exit                                              */
/*                                                                    */
/**********************************************************************/

void Error (char *String)
{
    void CloseAll(void);
    
    printf(String);

    CloseAll();
    exit(0);
}


/**********************************************************************/
/*                                                                    */
/* void Init ()                                                       */
/*                                                                    */
/* Opens all the required libraries                                   */
/* allocates all memory, etc.                                         */
/*                                                                    */
/**********************************************************************/

void Init ()
{
    if ((IntuitionBase = (struct IntuitionBase *)OpenLibrary ("intuition.library", LIBVERSION)) == NULL)
    {
    Error ("Could not open the Intuition.library") ;
    }

    if ((GfxBase = (struct GfxBase *)OpenLibrary ("graphics.library", LIBVERSION)) == NULL)
    {
    Error ("Could not open the Graphics.library") ;
    }

    if ((IFFParseBase = OpenLibrary("iffparse.library", 37L)) == NULL)
    {
    Error("Could not open the iffparse.library");
    }

    return ;
}

/**********************************************************************/
/*                                                                    */
/* void CloseAll ()                                                   */
/*                                                                    */
/* Closes and tidies up everything that was used.                     */
/*                                                                    */
/**********************************************************************/

void CloseAll ()
{
    /* Close everything in the reverse order in which they were opened */

    if (cm)
    {
        FreeColorMap(cm);
    }

    if (iff)
    {
    CloseIFF(iff);
    if (iff->iff_Stream)
    {
        Close(iff->iff_Stream);
    }
    FreeIFF(iff);
    }

    if (inbuff)
    {
    FreeVec(inbuff);
    }

    if (rdargs)
    {
    FreeArgs(rdargs);
    }

    if (ScriptFile)
    {
    Close(ScriptFile);
    }

    if (OutFile)
    {
        Close(OutFile);
    }

    if (IFFParseBase)
    {
    CloseLibrary(IFFParseBase);
    }

    if (GfxBase)
    {
    CloseLibrary((struct Library *)GfxBase);
    }

    if (IntuitionBase)
    {
    CloseLibrary((struct Library *)IntuitionBase);
    }

    return;
}

/***************************************************************************/

#define ID_ILBM MAKE_ID('I','L','B','M')
#define ID_BODY MAKE_ID('B','O','D','Y')
#define ID_DLTA MAKE_ID('D','L','T','A')
#define ID_BMHD MAKE_ID('B','M','H','D')
#define ID_CMAP MAKE_ID('C','M','A','P')
#define ID_CAMG MAKE_ID('C','A','M','G')

void main (int argc, char *argv[])
{
    #define TEMPLATE "FROM/A,TO/A,M=MODEID/N,X=REPEAT/N"
    #define OPT_FROM 0
    #define OPT_TO 1
    #define OPT_MODE 2
    #define OPT_REPEAT 3
    #define OPT_COUNT 4

    #define SCREEN_W f1.f1_ImageWidth
    #define SCREEN_H f1.f1_ImageHeight
    #define SCREEN_D f1.f1_ImageDepth
    #define SCREEN_MODEID ModeID

    LONG result[OPT_COUNT];
    LONG props[] =
    {
    ID_ILBM, ID_CAMG,
    ID_ILBM, ID_CMAP,
    ID_ILBM, ID_BMHD,
    };
    STRPTR from = NULL, to = NULL;
    ULONG secsize;
    ULONG maxsec = 0;
    ULONG ModeID = INVALID_ID;
    ULONG Repeat = 0;
    LONG *val;
    LONG read;
    struct ContextNode *cn;
    struct StoredProperty *sp;
    UBYTE *cmap;
    struct Frame1Head f1;
    struct BitMapHeader bmhd;
    UWORD ncolours;

    Init () ;

    /* init the result[] */
    result[OPT_FROM] = (LONG)&from;
    result[OPT_TO] = (LONG)&to;
    result[OPT_MODE] = (LONG)&ModeID;
    result[OPT_REPEAT] = (LONG)&Repeat;

    if (rdargs = ReadArgs(TEMPLATE, result, NULL))
    {
    from = (STRPTR)result[OPT_FROM];
    to = (STRPTR)result[OPT_TO];
    if (val = (LONG *)result[OPT_MODE])
    {
        ModeID = *val;
    }
    if (val = (LONG *)result[OPT_REPEAT])
    {
        Repeat = *val;
    }
    }
    else
    {
        Error("Command line error\n");
    }

    if ((OutFile = Open(to, MODE_NEWFILE)) == NULL)
    {
    Error("OutFile did not open\n");
    }

    /* Read the first ILBM chunk */

    if (iff = AllocIFF())
    {
    if ((iff->iff_Stream = Open(from, MODE_OLDFILE)) == NULL)
    {
        Error("InFile did not open\n");
    }
    InitIFFasDOS(iff);
    if (OpenIFF(iff, IFFF_READ) == NULL)
    {
        if ((PropChunks(iff, props, 3) == 0) &&
            (StopChunk(iff, ID_ILBM, ID_BODY) == 0) &&
            (StopChunk(iff, ID_ILBM, ID_DLTA) == 0) &&
            (ParseIFF(iff, IFFPARSE_SCAN) == 0))
        {
            if ((ModeID == INVALID_ID) && (sp = FindProp(iff, ID_ILBM, ID_CAMG)))
            {
                ULONG *tmp = (ULONG *)sp->sp_Data;
                ModeID = *tmp;
            }
            if (sp = FindProp(iff, ID_ILBM, ID_BMHD))
            {
                bmhd = (*(struct BitMapHeader *)sp->sp_Data);
                printf("width %ld, height %ld, depth %ld\n", bmhd.w, bmhd.h, bmhd.nplanes);
                f1.f1_ImageWidth = bmhd.w;
                f1.f1_ImageHeight = bmhd.h;
                f1.f1_ImageDepth = bmhd.nplanes;
            }
            else
            {
                Error("No BMHD\n");
            }
            if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
            {
                int i;

                cmap = (UBYTE *)sp->sp_Data;
                printf("CMAP found\n");
                /* We need to store these before losing the context */
                if (cm = GetColorMap(ncolours = (sp->sp_Size / 3)))
                {
                    for (i = 0; i < ncolours; i++)
                    {
                        SetRGB32CM(cm, i, ((*cmap++) << 24), ((*cmap++) << 24), ((*cmap++) << 24));
                    }
                }
                else
                {
                    Error("Could not get CMap\n");
                }
            }
            else
            {
                Error("no CMAP\n");
            }

            f1.f1_ScreenWidth = SCREEN_W;
            f1.f1_ScreenHeight = SCREEN_H;
            f1.f1_ScreenDepth = SCREEN_D;
            f1.f1_ScreenModeID = SCREEN_MODEID;
            
            /* we are at the BODY chunk */
            if ((cn = CurrentChunk(iff)) &&
                (secsize = BYTES_TO_WHOLE_SECTORS(cn->cn_Size)) && 
                (inbuff = AllocVec(secsize, MEMF_CLEAR)))
            {
                struct Frame1Head *f1t = (struct Frame1Head *)inbuff;
                UBYTE *tocm = (inbuff + sizeof(f1));
                ULONG rgb[3];
                int i;

                f1.f1_MaxSectorSize = 0;
                f1.f1_ByteSize = cn->cn_Size;
                f1.f1_Sectors = BYTES_TO_SECTORS(cn->cn_Size);
                f1.f1_Colours = ncolours;
                *f1t = f1;

                /* write out the colours */
                for (i = 0; i < ncolours; i++)
                {
                    GetRGB32(cm, i, 1, rgb);
                    *tocm++ = (rgb[0] >> 24);
                    *tocm++ = (rgb[1] >> 24);
                    *tocm++ = (rgb[2] >> 24);
                }

                printf("BODY size = %lx\n", cn->cn_Size);
                read = ReadChunkBytes(iff, (inbuff + sizeof(f1) + (ncolours * 3)), cn->cn_Size);
                printf("read %lx\n", read);
                if (read != cn->cn_Size)
                {
                    Error("ILBM read error\n");
                }
                maxsec = MAX(secsize, maxsec);
            }
        }
    }
    }
        
    if ((ParseIFF(iff, IFFPARSE_SCAN) == 0) &&
    (cn = CurrentChunk(iff)) &&
    (cn->cn_ID == ID_DLTA))
    {
    ULONG *end = (ULONG *)(inbuff + sizeof(f1) + (ncolours * 3) + read);

    /* End of the ILBM contains the data for the next frame */
    *(end++) = cn->cn_Size; /* next size */
    *(end++) = BYTES_TO_SECTORS(cn->cn_Size);

    /* Write out the whole thing */
    if (Write(OutFile, inbuff, secsize) != secsize)
    {
        Error("Error writing frame 0\n");
    }
    FreeVec(inbuff);
    inbuff = NULL;
    read = cn->cn_Size;

    }
    else
    {
        Error("End of Anim\n");
    }


    /* Now read the DLTA chunks */
    while ((secsize = BYTES_TO_WHOLE_SECTORS(read)) &&
           (inbuff = AllocVec(secsize, MEMF_CLEAR)) &&
           (ReadChunkBytes(iff, inbuff, read) == read))

    {
    ULONG *end = (ULONG *)(inbuff + read);

    if ((ParseIFF(iff, IFFPARSE_SCAN) == 0) &&
            (cn = CurrentChunk(iff)) &&
            (cn->cn_ID == ID_DLTA))
    {
        /* End of the DLTA contains the data for the next frame */
        *(end++) = cn->cn_Size; /* next size */
        *(end++) = BYTES_TO_SECTORS(cn->cn_Size);
        read = cn->cn_Size;
    }
    else
    {
        /* at the end of the anim. If we need to repeat,
         * shut everythink down and start again.
         *
         * The animation was WRAPPED at the end, so we can skip the
         * first DLTA.
         */
        if (Repeat--)
        {
            printf("repeating, %ld left to go\n", Repeat);
            CloseIFF(iff);
            Close(iff->iff_Stream);
            FreeIFF(iff);
            iff = NULL;
            if ((iff = AllocIFF()) == NULL)
            {
                Error("Phooey!\n");
            }
            if ((iff->iff_Stream = Open(from, MODE_OLDFILE)) == NULL)
            {
                Error("Could not reopen the anim file\n");
            }
            InitIFFasDOS(iff);
            if (OpenIFF(iff, IFFF_READ))
            {
                Error("repeat anim Open error\n");
            }
            if ((StopChunk(iff, ID_ILBM, ID_DLTA) == NULL) &&
                (ParseIFF(iff, IFFPARSE_SCAN) == 0) &&
                (ParseIFF(iff, IFFPARSE_SCAN) == 0) &&
                (cn = CurrentChunk(iff)) &&
                (cn->cn_ID == ID_DLTA))
            {
                *(end++) = cn->cn_Size;
                *(end++) = BYTES_TO_SECTORS(cn->cn_Size);
                read = cn->cn_Size;
            }
        }
    }

    /* Write it out */
    if (Write(OutFile, inbuff, secsize) != secsize)
    {
        Error("File write error\n");
    }
    FreeVec(inbuff);
    inbuff = NULL;
    maxsec = MAX(secsize, maxsec);
    }

    /* write out the last frame */
    if (inbuff)
    {
    ULONG *end = (ULONG *)(inbuff + read);

    /* End of the DLTA contains the data for the next frame */
    *(end++) = 0;
    *(end++) = 0;

    Write(OutFile, inbuff, secsize);
    FreeVec(inbuff);
    inbuff = NULL;
    }

    /* write out the max buffer size */
    Close(OutFile);
    OutFile = NULL;
    if (OutFile = Open(to, MODE_READWRITE))
    {
    maxsec = BYTES_TO_SECTORS(maxsec);
    printf("Max sector size is %ld\n", maxsec);
    Write(OutFile, &maxsec, sizeof(maxsec));
    }
    else
    {
    Error("Seek error\n");
    }

    CloseIFF(iff);
    Close(iff->iff_Stream);
    FreeIFF(iff);
    iff = NULL;

    CloseAll () ;
}
