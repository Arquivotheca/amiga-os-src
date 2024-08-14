
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <libraries/iffparse.h>
#include <prefs/font.h>
#include <prefs/prefhdr.h>
#include <prefs/wbpattern.h>
#include <prefs/palette.h>
#include <prefs/pointer.h>
#include <workbench/icon.h>
#include <intuition/pointerclass.h>
#include <string.h>
#include <dos.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/icon_protos.h>
#include <clib/graphics_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include "ilbm.h"
#include "updatewbfiles_rev.h"


/*****************************************************************************/


typedef BOOL (*IFFFunc)(struct IFFHandle *, struct ContextNode *);

BOOL ReadIFF(STRPTR name, ULONG *stopChunks, ULONG chunkCnt, IFFFunc readFunc);
BOOL WriteIFF(STRPTR name, IFFFunc writeFunc);
BOOL ReadFontPrefs(struct IFFHandle *iff, struct ContextNode *cn);
BOOL WriteFontPrefs(struct IFFHandle *iff, struct ContextNode *cn);
BOOL WriteWBPatternPrefs(struct IFFHandle *iff, struct ContextNode *cn);
BOOL ReadPalettePrefs(BPTR fp);
BOOL WritePalettePrefs(struct IFFHandle *iff, struct ContextNode *cn);
BOOL ReadPointerPrefs(BPTR fp);
BOOL WritePointerPrefs(struct IFFHandle *iff, struct ContextNode *cn);


/*****************************************************************************/


static struct PrefHeader IFFPrefHeader =
{
    0,
    0,
    0
};

static LONG far fontChunks[] =
{
    ID_PREF, ID_FONT
};


/*****************************************************************************/


struct WBPat
{
    ULONG Reserved[4];
    UWORD DataLength;
    BYTE  Revision;
    BYTE  Depth;
    ULONG Data[16*4];  /* 16 longwords per plane, max of 4 planes */
};

#define MAX(a,b)  ((a) > (b) ? (a) : (b))
#define MIN(a,b)  ((a) < (b) ? (a) : (b))
#define	BPR(w)	  ((w) + 15 >> 4 << 1)		/* Bytes per row */
#define PDEPTH	  4
#define PWIDTH	  16
#define PHEIGHT	  127
#define MAXCOLORS 16


/****************************************************************************/


struct Library *DOSBase;
struct Library *SysBase;
struct Library *IFFParseBase;
struct Library *IconBase;
struct Library *GfxBase;

ULONG fontType;
BOOL  doScreenFont;
BOOL  doSysFont;
BOOL  doWBFont;
BOOL  doWinPat;
BOOL  doWBPat;

struct FontPrefs      screenFont;
struct FontPrefs      sysFont;
struct FontPrefs      wbFont;
struct WBPat          wbPat;
struct WBPat          winPat;
struct BitMapHeader   bmhd;
struct ColorRegister  cmap[MAXCOLORS];
struct Point2D        grab;
struct BitMap         body;
struct PalettePrefs   palettePrefs;


/****************************************************************************/


LONG main(VOID)
{
LONG               failureLevel = RETURN_FAIL;
LONG               failureCode = ERROR_INVALID_RESIDENT_LIBRARY;
BPTR               in, out;
char              *line;
ULONG              len;
BPTR               cd, lock;
struct DiskObject *diskObj;
struct Process    *process;
APTR               oldWP;

    geta4();

    SysBase      = (*((struct Library **) 4));
    DOSBase      = OpenLibrary("dos.library" VERSTAG,36);
    IFFParseBase = OpenLibrary("iffparse.library",36);
    IconBase     = OpenLibrary("icon.library",36);
    GfxBase      = OpenLibrary("graphics.library",36);

    if (DOSBase && IFFParseBase && IconBase && GfxBase)
    {
        process = (struct Process *)FindTask(NULL);
        oldWP   = process->pr_WindowPtr;
        process->pr_WindowPtr = (APTR)-1;

        failureCode = ERROR_OBJECT_NOT_FOUND;
        if (cd = Lock("NEWWB:",ACCESS_READ))
        {
            failureCode = 0;

            cd = CurrentDir(cd);



            /* wb.pat win.pat -->  wbpattern.prefs */

            if (lock = Lock("Prefs/Env-Archive/Sys/wbpattern.prefs",ACCESS_READ))
            {
                UnLock(lock);
            }
            else
            {
                doWinPat = FALSE;
                doWBPat  = FALSE;

                if (in = Open("Prefs/Env-Archive/Sys/wb.pat",MODE_OLDFILE))
                {
                    doWBPat = TRUE;
                    Read(in,&wbPat,sizeof(struct WBPat));
                    Close(in);
                }

                if (in = Open("Prefs/Env-Archive/Sys/win.pat",MODE_OLDFILE))
                {
                    doWinPat = TRUE;
                    Read(in,&winPat,sizeof(struct WBPat));
                    Close(in);
                }

                if (doWinPat || doWBPat)
                    WriteIFF("Prefs/Env-Archive/Sys/wbpattern.prefs",WriteWBPatternPrefs);
            }

            DeleteFile("Prefs/Env-Archive/Sys/wb.pat");
            DeleteFile("Prefs/Env-Archive/Sys/win.pat");



            /* pointer.ilbm --> pointer.prefs   */

            if (lock = Lock("Prefs/Env-Archive/Sys/pointer.prefs",ACCESS_READ))
            {
                UnLock(lock);
            }
            else
            {
                if (in = Open("Prefs/Env-Archive/Sys/pointer.ilbm",MODE_OLDFILE))
                {
                    if (ReadPointerPrefs(in))
                        WriteIFF("Prefs/Env-Archive/Sys/pointer.prefs",WritePointerPrefs);

                    Close(in);
                }
            }

            DeleteFile("Prefs/Env-Archive/Sys/pointer.ilbm");



            /* palette.ilbm --> palette.prefs   */

            if (lock = Lock("Prefs/Env-Archive/Sys/palette.prefs",ACCESS_READ))
            {
                UnLock(lock);
            }
            else
            {
                if (in = Open("Prefs/Env-Archive/Sys/palette.ilbm",MODE_OLDFILE))
                {
                    if (ReadPalettePrefs(in))
                        WriteIFF("Prefs/Env-Archive/Sys/palette.prefs",WritePalettePrefs);

                    Close(in);
                }
            }

            DeleteFile("Prefs/Env-Archive/Sys/palette.ilbm");



            /* screenfont.prefs wbfont.prefs sysfont.prefs --> font.prefs */

            if (lock = Lock("Prefs/Env-Archive/Sys/font.prefs",ACCESS_READ))
            {
                UnLock(lock);
            }
            else
            {
                fontType     = FP_SCREENFONT;
                doScreenFont = ReadIFF("Prefs/Env-Archive/Sys/screenfont.prefs",fontChunks,1,ReadFontPrefs);

                fontType  = FP_SYSFONT;
                doSysFont = ReadIFF("Prefs/Env-Archive/Sys/sysfont.prefs",fontChunks,1,ReadFontPrefs);

                fontType = FP_WBFONT;
                doWBFont = ReadIFF("Prefs/Env-Archive/Sys/wbfont.prefs",fontChunks,1,ReadFontPrefs);

                if (doScreenFont || doSysFont || doWBFont)
                    WriteIFF("Prefs/Env-Archive/Sys/font.prefs",WriteFontPrefs);
            }

            DeleteFile("Prefs/Env-Archive/Sys/wbfont.prefs");
            DeleteFile("Prefs/Env-Archive/Sys/screenfont.prefs");
            DeleteFile("Prefs/Env-Archive/Sys/sysfont.prefs");



            /* .backdrop updating */

            if (line = AllocVec(4096,MEMF_ANY))
            {
                if (in = Open(".backdrop",MODE_OLDFILE))
                {
                    if (out = Open(".newbackdrop",MODE_NEWFILE))
                    {
                        while (FGets(in,line,4096))
                        {
                            len = strlen(line);
                            if ((len > 0) && (line[len-1] == '\n'))
                                line[--len] = 0;

                            if (diskObj = GetDiskObject(line))
                            {
                                FPuts(out,line);
                                FPuts(out,"\n");
                                FreeDiskObject(diskObj);
                            }
                        }
                        Close(out);
                    }
                    Close(in);
                }

                DeleteFile(".backdrop");
                Rename(".newbackdrop",".backdrop");
            }

            UnLock(CurrentDir(cd));
        }

        process->pr_WindowPtr = oldWP;
    }

    if (DOSBase)
    {
        if (failureCode)
            PrintFault(failureCode,NULL);

        CloseLibrary(GfxBase);
        CloseLibrary(IconBase);
        CloseLibrary(IFFParseBase);
        CloseLibrary(DOSBase);
    }

    return(failureLevel);
}


/*****************************************************************************/


struct IFFHandle *GetIFF(STRPTR name, LONG mode)
{
struct IFFHandle *iff;
LONG              error;

    if (iff = AllocIFF())
    {
        if (iff->iff_Stream = (ULONG)Open(name,mode))
        {
            InitIFFasDOS(iff);

            if (!(error = OpenIFF(iff, (mode == MODE_OLDFILE)? IFFF_READ : IFFF_WRITE)))
	        return(iff);

            Close((BPTR)iff->iff_Stream);
        }
        FreeIFF(iff);
    }
    return(NULL);
}


/*****************************************************************************/


VOID ReturnIFF(struct IFFHandle *iff)
{
    CloseIFF(iff);
    Close(iff->iff_Stream);
    FreeIFF(iff);
}


/*****************************************************************************/


BOOL ReadIFF(STRPTR name, ULONG *stopChunks, ULONG chunkCnt, IFFFunc readFunc)
{
struct IFFHandle   *iff;
struct ContextNode *cn;
BOOL                result = FALSE;
LONG                error;

    if (iff = GetIFF(name,MODE_OLDFILE))
    {
        if (!ParseIFF(iff,IFFPARSE_STEP))
        {
            cn = CurrentChunk(iff);

            if (cn->cn_ID == ID_FORM)
            {
                if (!StopChunks(iff,stopChunks,chunkCnt))
                {
                    while (TRUE)
                    {
                        if (error = ParseIFF(iff,IFFPARSE_SCAN))
                        {
                            result = TRUE;
                            break;
                        }

                        cn = CurrentChunk(iff);

                        if (!(result = readFunc(iff,cn)))
                            break;
                    }
                }
            }
        }
        ReturnIFF(iff);
    }

    return(result);
}


/*****************************************************************************/


BOOL WriteIFF(STRPTR name, IFFFunc writeFunc)
{
struct IFFHandle *iff;
BOOL              result = FALSE;

    if (iff = GetIFF(name,MODE_NEWFILE))
    {
        if (!PushChunk(iff,ID_PREF,ID_FORM,IFFSIZE_UNKNOWN))
        {
            if (!PushChunk(iff,0,ID_PRHD,sizeof(struct PrefHeader)))
            {
                if (WriteChunkBytes(iff,&IFFPrefHeader,sizeof(struct PrefHeader)) == sizeof(struct PrefHeader))
                {
                    if (!PopChunk(iff))
                    {
                        if ((result = writeFunc(iff,NULL)))
                        {
		            if (PopChunk(iff))
		            {
		                result = FALSE;
		            }
                        }
                    }
                }
            }
        }
        ReturnIFF(iff);
        SetProtection(name,FIBF_EXECUTE);
    }

    return(result);
}


/*****************************************************************************/


#define NUMPROPS(array) (sizeof(array)/8)


static LONG ilbmprops[] =
{
    ID_ILBM, ID_BMHD,
    ID_ILBM, ID_CMAP,
    ID_ILBM, ID_GRAB
};


/*****************************************************************************/


BOOL GetBMHD(struct IFFHandle *iff, struct BitMapHeader *bmhd)
{
struct StoredProperty *sp;

    if (sp = FindProp(iff,ID_ILBM,ID_BMHD))
    {
	*bmhd = *((struct BitMapHeader *)sp->sp_Data);
	return (TRUE);
    }

    return (FALSE);
}


/*****************************************************************************/


BOOL GetColors(struct IFFHandle *iff, struct ColorRegister *cmap)
{
struct ColorRegister  *rgb;
struct StoredProperty *sp;
WORD                   ncolors;

    if (sp = FindProp(iff,ID_ILBM,ID_CMAP))
    {
	/* Compute the actual number of colors we need to convert. */
	ncolors = MIN(16, (sp->sp_Size / 3));

	rgb = (struct ColorRegister *)sp->sp_Data;
	while (ncolors--)
	{
	    *cmap++ = *rgb++;
	}
	return (TRUE);
    }

    return (FALSE);
}


/*****************************************************************************/


VOID GetHotSpot(struct IFFHandle *iff, struct Point2D *grab)
{
struct StoredProperty *sp;

    if (sp = FindProp(iff,ID_ILBM,ID_GRAB))
    {
	*grab = *((struct Point2D *)sp->sp_Data);
    }
    else
    {
	grab->x = 0;
	grab->y = 0;
    }

    if (grab->x >= PWIDTH)
    {
	grab->x = 0;
    }
}


/*****************************************************************************/


BOOL GetLine(struct IFFHandle *iff, APTR buf, WORD wide, WORD deep, UBYTE cmptype)
{
    if (cmptype == cmpNone)
    {	/* No compression */
	LONG big = wide * deep;

	if (ReadChunkBytes(iff, buf, big) != big)
	    return (FALSE);
    }
    else
    {
	WORD i, so_far;
	UBYTE *dest = buf;
	BYTE len;

	for (i = deep;  i--; )
	{
	    so_far = wide;
	    while (so_far > 0)
	    {
		if (ReadChunkBytes(iff, &len, 1L) != 1)
		    return (FALSE);

		if (len >= 0)
		{   /*  Literal byte copy  */
		    if ((so_far -= ++len) < 0)
			break;
		    if (ReadChunkBytes(iff, dest, (LONG) len) != len)
			return (FALSE);
		    dest += len;
		}
		else if ((UBYTE)len == 128)
		    /*  NOP  */ ;

		else if (len < 0)
		{   /*  Replication count  */
		    UBYTE byte;

		    len = -len + 1;
		    if ((so_far -= len) < 0)
			break;
		    if (ReadChunkBytes(iff, &byte, 1L) != 1)
			return (FALSE);
		    while (--len >= 0)
			*dest++ = byte;
		}
	    }
	    if (so_far)
	    {
		return (FALSE);
	    }
	}
    }
    return (TRUE);
}


/*****************************************************************************/


BOOL GetBody(struct IFFHandle *iff, struct BitMapHeader *bmhd, struct BitMap *bm)
{
BOOL status = TRUE;
WORD i, n, p;
UWORD *srcline, *destline;
struct BitMap mapcopy, *destmap;
UBYTE *linebuf, *csrcline;
WORD srcw, destw;                   /* src and dest width in BYTES */
WORD srch, desth;                   /* src and dest height (rows) */
WORD srcd, destd;                   /* src and dest depth */
WORD deep, rows, mod;

    InitBitMap(bm, MIN(PDEPTH, bmhd->nplanes), PWIDTH, MIN(PHEIGHT, bmhd->h));
    for (i = 0; i < 8; i++)
	bm->Planes[i] = NULL;
    for (i = 0; i < bm->Depth; i++)
    {
	if ((bm->Planes[i] = AllocRaster(PWIDTH, bm->Rows)) == NULL)
	    return (FALSE);
    }

    destmap = bm;

    /* Check compression type. */
    if ((bmhd->Compression != cmpNone) && (bmhd->Compression != cmpByteRun1))
    {
	return (FALSE);
    }

    CopyMem((APTR)destmap, (APTR)&mapcopy, (ULONG)sizeof(mapcopy));

    srcw  = BPR(bmhd->w);
    srch  = bmhd->h;
    srcd  = bmhd->nplanes;
    destw = mapcopy.BytesPerRow;
    desth = mapcopy.Rows;
    destd = mapcopy.Depth;
    rows  = MIN(srch, desth);
    mod	  = destw - srcw;

    if (mod < 0)
        mod = -mod;

    if (bmhd->Masking == mskHasMask)
	srcd++;

    deep = MIN(srcd, destd);
    /*
     * Allocate a one-line buffer to load imagery in.  The line is then
     * copied into the destination bitmap.  This seeming duplicity makes
     * clipping loaded images easier.
     */

    if (!(linebuf = (UBYTE *)AllocMem((LONG)srcw * srcd, 0L)))
    {
	return (FALSE);
    }

    /*
     * Load the BODY into the allocated line buffer, then copy into
     * the destination bitmap.
     */

    for (i = rows;  i--; )
    {
	if (!(status = GetLine(iff, linebuf, srcw, srcd, bmhd->Compression)))
	    break;

	srcline = (UWORD *)linebuf;
	for (p = 0;  p < deep;  p++)
	{
	    destline = (UWORD *)mapcopy.Planes[p];
	    *destline = 0xffff;
	    n = (MIN(srcw, destw)) >> 1;	/* convert #bytes to #words */

	    while (n--)
		*destline++ = *srcline++;

	    if (srcw > destw)
	    {
		csrcline = (UBYTE *)srcline;
		csrcline += mod;
		srcline = (UWORD *)csrcline;
	    }
	    mapcopy.Planes[p] += destw;
	}
    }

    if (linebuf)
	FreeMem(linebuf, (LONG)srcw * srcd);

    return (status);
}


/*****************************************************************************/


BOOL GetILBM(struct IFFHandle *iff)
{
    if (GetBMHD(iff,&bmhd))
    {
	if (GetColors(iff,cmap))
	{
	    GetHotSpot(iff,&grab);

	    if (GetBody(iff,&bmhd,&body))
	    {
		return(TRUE);
	    }
	}
    }

    return(FALSE);
}


/*****************************************************************************/


BOOL ReadILBM(BPTR fp)
{
struct IFFHandle   *iff;
struct ContextNode *cn;
BOOL                result = FALSE;
BOOL                bodyFlag = FALSE;
LONG                error;

    if (iff = AllocIFF())
    {
        iff->iff_Stream = (ULONG)fp;
        InitIFFasDOS(iff);

        if (!OpenIFF(iff,IFFF_READ))
        {
            if (!PropChunks(iff,ilbmprops,NUMPROPS(ilbmprops)))
            {
                if (!StopChunk(iff,ID_ILBM,ID_BODY))
		{
                    if (!StopOnExit(iff,ID_ILBM,ID_FORM))
		    {
			if (!ParseIFF(iff,IFFPARSE_STEP))
			{
			    cn = CurrentChunk(iff);

			    if (cn->cn_ID == ID_FORM && cn->cn_Type == ID_ILBM)
			    {
			        while (TRUE)
                                {
                                    error = ParseIFF(iff,IFFPARSE_SCAN);

                                    if ((error == IFFERR_EOC) || (error == IFFERR_EOF))
                                    {
                                        result = (bodyFlag);
                                        break;
                                    }
                                    else if (error)
                                    {
                                        break;
                                    }
                                    else
                                    {
                                        if (!GetILBM(iff))
                                            break;
                                        bodyFlag = TRUE;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            CloseIFF(iff);
        }
        FreeIFF(iff);
    }

    return (result);
}


/*****************************************************************************/


BOOL ReadFontPrefs(struct IFFHandle *iff, struct ContextNode *cn)
{
struct FontPrefs  tmp;
struct FontPrefs *fp;

    if (ReadChunkBytes(iff,&tmp,sizeof(struct FontPrefs)) == sizeof(struct FontPrefs))
    {
        if (fontType == FP_WBFONT)
            fp = &wbFont;

        else if (fontType == FP_SYSFONT)
            fp = &sysFont;

        else if (fontType == FP_SCREENFONT)
            fp = &screenFont;

        tmp.fp_Type = fontType;
        *fp = tmp;

        return(TRUE);
    }

    return(FALSE);
}


/*****************************************************************************/


BOOL WriteFontPrefs(struct IFFHandle *iff, struct ContextNode *cn)
{
    if (doWBFont)
        if (!PushChunk(iff,0,ID_FONT,sizeof(struct FontPrefs)))
            if (WriteChunkBytes(iff,&wbFont,sizeof(struct FontPrefs)) == sizeof(struct FontPrefs))
                PopChunk(iff);

    if (doScreenFont)
        if (!PushChunk(iff,0,ID_FONT,sizeof(struct FontPrefs)))
            if (WriteChunkBytes(iff,&screenFont,sizeof(struct FontPrefs)) == sizeof(struct FontPrefs))
                PopChunk(iff);

    if (doSysFont)
        if (!PushChunk(iff,0,ID_FONT,sizeof(struct FontPrefs)))
            if (WriteChunkBytes(iff,&sysFont,sizeof(struct FontPrefs)) == sizeof(struct FontPrefs))
                PopChunk(iff);

    return(TRUE);
}


/*****************************************************************************/


struct ExtWBPatternPrefs
{
    struct WBPatternPrefs WBPrefs;
    ULONG                 Data[16*4];
};

BOOL WriteWBPatternPrefs(struct IFFHandle *iff, struct ContextNode *cn)
{
struct ExtWBPatternPrefs wbp;
ULONG                    len;

    wbp.WBPrefs.wbp_Reserved[0] = 0;
    wbp.WBPrefs.wbp_Reserved[1] = 0;
    wbp.WBPrefs.wbp_Reserved[2] = 0;
    wbp.WBPrefs.wbp_Reserved[3] = 0;
    wbp.WBPrefs.wbp_Revision    = 0;
    wbp.WBPrefs.wbp_Flags       = WBPF_PATTERN;

    if (doWBPat)
    {
        wbp.WBPrefs.wbp_Which      = WBP_ROOT;
        wbp.WBPrefs.wbp_Depth      = wbPat.Depth;
        wbp.WBPrefs.wbp_DataLength = wbPat.DataLength;
        CopyMem(wbPat.Data,wbp.Data,wbPat.DataLength);
        len = sizeof(struct WBPatternPrefs) + wbPat.DataLength;

        if (!PushChunk(iff,0,ID_PTRN,len))
            if (WriteChunkBytes(iff,&wbp,len) == len)
                PopChunk(iff);
    }

    if (doWinPat)
    {
        wbp.WBPrefs.wbp_Which      = WBP_DRAWER;
        wbp.WBPrefs.wbp_Depth      = winPat.Depth;
        wbp.WBPrefs.wbp_DataLength = winPat.DataLength;
        CopyMem(winPat.Data,wbp.Data,winPat.DataLength);
        len = sizeof(struct WBPatternPrefs) + winPat.DataLength;

        if (!PushChunk(iff,0,ID_PTRN,len))
            if (WriteChunkBytes(iff,&wbp,len) == len)
                PopChunk(iff);
    }

    return(TRUE);
}


/*****************************************************************************/


BOOL ReadPointerPrefs(BPTR fp)
{
WORD i;

    /* Clear color map, just to be sure... */
    for (i = 0; i < MAXCOLORS; i++)
    {
	cmap[i].red   = 0;
	cmap[i].green = 0;
	cmap[i].blue  = 0;
    }

    if (!ReadILBM(fp))
        return(FALSE);

    return(TRUE);
}


/*****************************************************************************/


BOOL WritePointerPrefs(struct IFFHandle *iff, struct ContextNode *cn)
{
struct PointerPrefs pointerPrefs;
UWORD               plane, i, msize;

    pointerPrefs.pp_Reserved[0] = 0;
    pointerPrefs.pp_Reserved[1] = 0;
    pointerPrefs.pp_Reserved[2] = 0;
    pointerPrefs.pp_Reserved[3] = 0;
    pointerPrefs.pp_Which       = WBP_NORMAL;
    pointerPrefs.pp_Size        = POINTERXRESN_DEFAULT;
    pointerPrefs.pp_Width       = 16;
    pointerPrefs.pp_Height      = body.Rows;
    pointerPrefs.pp_Depth       = body.Depth;
    pointerPrefs.pp_YSize       = 16;				/* YSize */
    pointerPrefs.pp_X           = -grab.x;
    pointerPrefs.pp_Y           = -grab.y;

    if (!PushChunk(iff,0,ID_PNTR,IFFSIZE_UNKNOWN))
    {
        WriteChunkBytes(iff,&pointerPrefs,sizeof(struct PointerPrefs));
        WriteChunkBytes(iff,&cmap[1],sizeof(struct ColorRegister));
        WriteChunkBytes(iff,&cmap[2],sizeof(struct ColorRegister));
        WriteChunkBytes(iff,&cmap[3],sizeof(struct ColorRegister));

        msize = body.Rows * body.BytesPerRow;
        for (i = 0; i < body.Depth; i++)
            WriteChunkBytes(iff, body.Planes[i], msize);

        PopChunk(iff);
    }

    /* De-allocate BitMap planes */
    for (plane = 0; plane < body.Depth; plane++)
    {
        if (body.Planes[plane])
            FreeRaster(body.Planes[plane], PWIDTH, body.Rows);
    }

    return(TRUE);
}


/*****************************************************************************/


#define NUM_KNOWNPENS 12
UBYTE DefaultPens[] =
{
    0,	/* DETAILPEN		 */
    1,	/* BLOCKPEN		 */
    1,	/* TEXTPEN		 */
    2,	/* SHINEPEN		 */
    1,	/* SHADOWPEN		 */
    3,	/* FILLPEN		 */
    1,	/* FILLTEXTPEN		 */
    0,	/* BACKGROUNDPEN	 */
    2,	/* HIGHLIGHTTEXTPEN 	 */
    1,	/* BARDETAILPEN	 	 */
    2,	/* BARBLOCKPEN	 	 */
    1	/* BARTRIMPEN	 	 */
};

#define Make32(sixteen) ((((ULONG)sixteen)<<16) | (ULONG)sixteen)
ULONG Make16(ULONG value, ULONG bits)
{
ULONG result;
ULONG mask;

    if (bits == 16)
        return(value);

    mask   = value << (16 - bits);
    result = 0;
    while (mask)
    {
        result |= mask;
        mask    = mask >> bits;
    }

    return(result);
}

BOOL ReadPalettePrefs(BPTR fp)
{
struct IFFHandle *iff;
BOOL              status = FALSE;
UWORD             i;
LONG              error;
struct ColorRegister creg;

    if (iff = AllocIFF())
    {
        iff->iff_Stream = (ULONG)fp;
        InitIFFasDOS(iff);

        if (!OpenIFF(iff,IFFF_READ))
        {
            if (!StopChunk(iff,ID_ILBM,ID_CMAP))
            {
                if (!ParseIFF(iff, IFFPARSE_SCAN))
                {
                    i = 0;
                    while ((i < MAXCOLORS) && ((error = ReadChunkBytes(iff,&creg,sizeof(struct ColorRegister))) == sizeof(struct ColorRegister)))
                    {
                        palettePrefs.pap_Colors[i].ColorIndex = i;
                        palettePrefs.pap_Colors[i].Red        = Make32(Make16(creg.red,8));
                        palettePrefs.pap_Colors[i].Green      = Make32(Make16(creg.green,8));
                        palettePrefs.pap_Colors[i].Blue       = Make32(Make16(creg.blue,8));
                        i++;
                    }
                    palettePrefs.pap_Colors[i].ColorIndex = -1;

                    status = TRUE;

                    for (i = 0; i < NUM_KNOWNPENS; i++)
                    {
                        palettePrefs.pap_4ColorPens[i] = DefaultPens[i];
                        palettePrefs.pap_8ColorPens[i] = DefaultPens[i];
                    }
                    palettePrefs.pap_4ColorPens[NUM_KNOWNPENS] = ~0;
                    palettePrefs.pap_8ColorPens[NUM_KNOWNPENS] = ~0;
                }
            }
            CloseIFF(iff);
        }
	FreeIFF(iff);
    }

    return(status);
}


/*****************************************************************************/


BOOL WritePalettePrefs(struct IFFHandle *iff, struct ContextNode *cn)
{
    if (!PushChunk(iff,0,ID_PALT,sizeof(struct PalettePrefs)))
        if (WriteChunkBytes(iff,&palettePrefs,sizeof(struct PalettePrefs)) == sizeof(struct PalettePrefs))
            PopChunk(iff);

    return(TRUE);
}
