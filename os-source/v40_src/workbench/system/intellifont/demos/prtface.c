/*
**	$Id$
**
**      prtface typeface print utility
**
**      (C) Copyright 1991 Robert R. Burns
**          All Rights Reserved
*/
#ifdef   DEBUG
#define  D(a)	printf a
#else
#define  D(a)
#endif
#include <exec/types.h>
#include <dos/rdargs.h>
#include <graphics/rastport.h>
#include <graphics/view.h>
#include "diskfonttag.h"
#include "glyph.h"

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>
#include <clib/diskfont_protos.h>
#include "bullet_protos.h"
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/diskfont_pragmas.h>
#include "bullet_pragmas.h"

#include "SYS:include2.0strip/math.h"
#include <string.h>
#undef    strchr
#undef    strcmp
#undef    strcpy
#undef    strrchr
#undef    memset

extern struct Library *SysBase;
extern struct Library *DOSBase;

extern struct RastPort PrinterRP;
extern WORD MaxX, MaxY, DPIX, DPIY;

#define  TEMPLATE	"NAME,ALL/S"

#define  O_NAME		0
#define  O_ALL		1
#define  O_COUNT	2

#define  MAXFILESIZE	4000

struct Library *GfxBase = 0;
struct Library *UtilityBase = 0;
struct Library *DiskfontBase = 0;
struct RDArgs *RDArgs = 0;
BPTR OTFile = 0;
struct Library *BulletBase = 0;
struct GlyphEngine *GlyphEngine = 0;
struct TagItem OTags[MAXFILESIZE / sizeof(struct TagItem)];
PLANEPTR Template = 0;
struct TextFont *FFont = 0;

void
endGame(format, arg1, arg2, arg3, arg4)
char *format, *arg1, *arg2, *arg3, *arg4;
{
    if (format) {
	D((format, arg1, arg2, arg3, arg4));
	printf(format, arg1, arg2, arg3, arg4);
    }
    if (Template)
	FreeRaster(Template, DPIX, DPIY);
    if (GlyphEngine)
	CloseEngine(GlyphEngine);
    if (BulletBase)
	CloseLibrary(BulletBase);
    if (OTFile)
	Close(OTFile);
    if (RDArgs)
	FreeArgs(RDArgs);
    if (DiskfontBase)
	CloseLibrary(DiskfontBase);
    if (UtilityBase)
	CloseLibrary(UtilityBase);
    ClosePrinter();
    if (FFont)
	CloseFont(FFont);
    if (GfxBase)
	CloseLibrary(GfxBase);
    if (format)
	exit(5);
    exit(0);
}


ULONG
setInfo(ge, tag)
struct GlyphEngine *ge;
LONG tag;
{
    return (SetInfoA(ge, (struct TagItem *) & tag));
}

ULONG
obtainInfo(ge, tag)
struct GlyphEngine *ge;
LONG tag;
{
    return (ObtainInfoA(ge, (struct TagItem *) & tag));
}

ULONG
releaseInfo(ge, tag)
struct GlyphEngine *ge;
LONG tag;
{
    return (ReleaseInfoA(ge, (struct TagItem *) & tag));
}

void
main()
{
    struct GlyphMap *glyph;
    ULONG result, *options[O_COUNT];
    int i, x, ux, uy, dx, dy, pageNo;
    ULONG pointHeight;
    ULONG chId;
    char pathStore[256], *filePath, *s, pageFilled, colFilled, legend[8];
    struct TTextAttr tta;
    struct TagItem tags[3];

    D(("Test\nOpenLibrarys..."));
    GfxBase = OpenLibrary("graphics.library", 0);
    if (!GfxBase)
	endGame("ERROR: cannot open \"graphics.library\"\n");
    if (OpenPrinter())
	endGame("ERROR: cannot open \"printer.device\"\n");
    UtilityBase = OpenLibrary("utility.library", 0);
    if (!UtilityBase)
	endGame("ERROR: cannot open \"utility.library\"\n");
    DiskfontBase = OpenLibrary("diskfont.library", 0);
    if (!DiskfontBase)
	endGame("ERROR: cannot open \"diskfont.library\"\n");
    D(("ReadArgs..."));
    memset(options, 0, sizeof(options));
    RDArgs = ReadArgs(TEMPLATE, (LONG *) options, 0);
    if (!RDArgs)
	endGame("ERROR: invalid arguments\n");

    D((" .\n"));
    /* generate .otag file path */
    strcpy(pathStore, "FONTS:");
    filePath = pathStore + 6;
    if (options[0])
	strcpy(filePath, (char *) options[O_NAME]);
    else
	strcpy(filePath, "LetterGothic.font");
    s = strrchr(filePath, '.');
    if ((s == 0) || (strcmp(s, ".font")))
	endGame("ERROR: NAME not .font name\n");

    strcpy(s, OTSUFFIX);

    D((".otag file \"%s\"\n", filePath));
    /* open .otag file */
    OTFile = Open(filePath, MODE_OLDFILE);
    if (!OTFile) {
	if (!strchr(filePath, ':')) {
	    filePath = pathStore;
	    OTFile = Open(filePath, MODE_OLDFILE);
	}
    }

    if (!OTFile)
	endGame("no font file \"%s\"\n", filePath);

    strcpy(s, ".font");
    D((".font file \"%s\"\n", filePath));

    /* read and verify the .otag */
    if (Read(OTFile, OTags, sizeof(struct TagItem)) != sizeof(struct TagItem))
	endGame("OTFile read fail (8 bytes)\n");

    if (OTags[0].ti_Tag != OT_FileIdent)
	endGame(".otag first tag $%lx, not $%lx\n", OTags[0].ti_Tag,
		OT_FileIdent);

    Seek(OTFile, 0, OFFSET_END);
    if (Seek(OTFile, 0, OFFSET_BEGINNING) != OTags[0].ti_Data)
	endGame(".otag file size wrong\n");

    if (OTags[0].ti_Data > MAXFILESIZE)
	endGame(".otag file size %ld larger than program maximum %ld\n",
		OTags[0].ti_Data, MAXFILESIZE);

    /* this is a valid .otag file header, read it */
    if (Read(OTFile, OTags, OTags[0].ti_Data) != OTags[0].ti_Data)
	endGame(".otag read failure\n");

    /* patch indirect pointers */
    for (i = 0; i < OTags[0].ti_Data / sizeof(struct TagItem); i++) {
	if (OTags[i].ti_Tag == TAG_DONE)
	    break;
	if (OTags[i].ti_Tag & OT_Indirect)
	    OTags[i].ti_Data += (ULONG) OTags;
    }

    /* ensure the associated glyph library is open */
    /* get the library name */
    s = (char *) GetTagData(OT_Engine, 0, OTags);
    if (!s)
	endGame("no OT_Engine tag\n");

    if (strcmp(s, "bullet"))
	endGame("OT_Engine not \"bullet\" but \"%s\"\n", s);

    BulletBase = OpenLibrary("bullet.library", 0);
    if (!BulletBase)
	endGame("OpenLibrary \"bullet.library\" failed\n");

    GlyphEngine = OpenEngine();

    if (!GlyphEngine)
	endGame("OpenEngine failed\n");

    result = setInfo(GlyphEngine, OT_OTagPath, filePath, TAG_DONE);
    if (result)
	endGame("setInfo(OTagPath) result %ld\n", result);

    result = setInfo(GlyphEngine, OT_OTagList, OTags, TAG_DONE);
    if (result)
	endGame("setInfo(OTagList) result %ld\n", result);

    uy = (MaxY - 1) / 17;		/* 17 vertical box units */
    ux = uy * DPIX / DPIY;		/* ? horizontal box units */
    pointHeight = ((uy * 36) << 16) / DPIY;	/* 1/2 vertical box height */

    D(("ux %ld uy %ld, pointHeight $%08lx\n", uy, ux, pointHeight));

    if (result = setInfo(GlyphEngine, OT_PointHeight, pointHeight,
		    OT_DeviceDPI, (DPIX << 16) | DPIY, TAG_DONE))
	endGame("setInfo(20 points) failed %ld\n", result);

    /* get labeling font */
    tags[0].ti_Tag = OT_DeviceDPI;
    tags[0].ti_Data = (DPIX << 16) | DPIY;
    tags[1].ti_Tag = OT_PointHeight;
    tags[1].ti_Data = pointHeight / 2;
    tags[2].ti_Tag = TAG_DONE;
    tta.tta_Name = "LetterGothic.font";
    tta.tta_YSize = 0;
    tta.tta_Flags = 0;
    tta.tta_Style = FSF_TAGGED;
    tta.tta_Tags = tags;
    FFont = OpenDiskFont((struct TextAttr *) & tta);
    if (!FFont)
	endGame("OpenDiskFont failed\n");
    SetFont(&PrinterRP, FFont);

    Template = AllocRaster(DPIX, DPIY);
    if (!Template)
	endGame("AllocRaster failed\n");

    *strrchr(filePath, '.') = '\0';	/* get rid of .font */
    filePath = FilePart(filePath);
    pageNo = 0;
    chId = 0;
    SetAPen(&PrinterRP, 1);
    SetDrMd(&PrinterRP, JAM1);
    do {
	pageFilled = 0;
	x = ux / 2;
	SetRast(&PrinterRP, 0);
	do {
	    colFilled = 0;
	    do {
		if (chId == 0x3000)
		    chId = 0xe800;
		if (result = setInfo(GlyphEngine, OT_GlyphCode, chId, TAG_DONE))
		    endGame("setInfo() failed %ld\n", result);
		if (!obtainInfo(GlyphEngine, OT_GlyphMap, &glyph, TAG_DONE)) {
		    dx = x + ux / 4 +
			    (glyph->glm_XOrigin >> 16) + glyph->glm_BlackLeft;
		    dy = ((chId & 0xf) + 2) * uy - ux / 4 -
			    (glyph->glm_YOrigin >> 16) + glyph->glm_BlackTop;
		    for (i = (glyph->glm_BMRows * glyph->glm_BMModulo / 4) - 1;
			    i >= 0; i--)
			((ULONG *) Template)[i] =
				((ULONG *) glyph->glm_BitMap)[i];
		    BltTemplate(((char *) Template) +
			    (glyph->glm_BlackTop * glyph->glm_BMModulo) +
			    ((glyph->glm_BlackLeft / 16) * 2),
			    glyph->glm_BlackLeft & 0xf,
			    glyph->glm_BMModulo, &PrinterRP, dx, dy,
			    glyph->glm_BlackWidth, glyph->glm_BlackHeight);
		    releaseInfo(GlyphEngine, OT_GlyphMap, glyph, TAG_DONE);
		    colFilled = 1;
		}
		chId++;
	    }
	    while (((!colFilled) || (chId & 0xf)) &&
		    (chId < (options[O_ALL] ? 0x10000 : 0x0370)));
	    if (colFilled) {
		/* fill in the top of this column */
		Move(&PrinterRP, x + ((ux - 4 * FFont->tf_XSize) / 2),
			uy / 2 + (uy / 2 - FFont->tf_YSize) / 2 +
			FFont->tf_Baseline);
		sprintf(legend, "%03X", (chId - 1) >> 4);
		D(("colFilled %s_\n", legend));
		legend[3] = '_';
		Text(&PrinterRP, legend, 4);
		/* draw vertical left hash mark */
		if (pageFilled)
		    Move(&PrinterRP, x, uy / 2);
		else
		    Move(&PrinterRP, x, 0);
		Draw(&PrinterRP, x, MaxY - 1);
		pageFilled = 1;
		x += ux;
	    }
	}
	while (((x + ux) < (MaxX - 1)) &&
		(chId < (options[O_ALL] ? 0x10000 : 0x0370)));

	if (pageFilled) {
	    D(("pageFilled\n"));
	    /* draw last vertical right hash mark */
	    Move(&PrinterRP, x, 0);
	    Draw(&PrinterRP, x, MaxY - 1);

	    for (i = 0; i < 16; i++) {
		/* fill in left legends */
		Move(&PrinterRP, 0, (i + 1) * uy +
			(uy - FFont->tf_YSize) / 2 + FFont->tf_Baseline);
		sprintf(legend, "%X", i);
		Text(&PrinterRP, legend, 1);
		/* fill in horizontal top hash marks */
		Move(&PrinterRP, ux / 2, (i + 1) * uy);
		Draw(&PrinterRP, x, (i + 1) * uy);
	    }

	    /* fill in last horizontal bottom hash marks */
	    Move(&PrinterRP, ux / 2, 17 * uy);
	    Draw(&PrinterRP, x, 17 * uy);

	    /* label the typeface */
	    Move(&PrinterRP, ux,
		    (uy / 2 - FFont->tf_YSize) / 2 + FFont->tf_Baseline);
	    Text(&PrinterRP, filePath, strlen(filePath));

	    sprintf(legend, "Page %2d", ++pageNo);
	    Move(&PrinterRP, x - ux / 2 -
		    TextLength(&PrinterRP, legend, strlen(legend)),
		    (uy / 2 - FFont->tf_YSize) / 2 + FFont->tf_Baseline);
	    Text(&PrinterRP, legend, strlen(legend));
	    /* print this page */
	    Print();
	}
    }
    while (chId < (options[O_ALL] ? 0x10000 : 0x0370));
    endGame(0);
}
