/*
 * ISSUES:
 *   if alloc char ptr arrays here, can make diskfont romable again?
 */
/*
**	$Id: genfont.c,v 39.1 92/07/14 08:08:30 darren Exp $
**
**      diskfont.library outline font generation
**
**      (C) Copyright 1992 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
*/
#define YSIZEFACTOR
/*	debug switches */

#ifdef	DEBUG
extern void kprintf(char *,...);
#define DS(a)	kprintf a
#define	D(a)	kprintf a
#define DD(a)
#else
#define DS(a)
#define	D(a)
#define DD(a)
#endif


/*	includes */

#include <exec/types.h>
#include <exec/memory.h>

#include <string.h>
#undef    strcat
#undef    strcmp
#undef    strcpy
#undef    memcpy
#include <setjmp.h>

#include "diskfont.h"
#include <diskfont/diskfonttag.h>
#include <diskfont/glyph.h>

#include "dfdata.h"

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>

/*	exports */

/*	imports */

#define	SysBase		dfl->dfl_SysBase
#define	DOSBase		dfl->dfl_DOSBase
#define	GfxBase		dfl->dfl_GfxBase
#define	UtilityBase	dfl->dfl_UtilityBase

/*	locals */
#define  OFONT_TAGCNT	4	/* Points, DPI, DotSize, DONE */

static struct GlyphEngine *glyphEngine;

static struct GlyphMap *charMaps[257];

static UWORD ySize, spaceWidth, yBaseOrigin, loCode, destMod;
static ULONG undefLoc;
static UWORD undefCode, undefKern, undefSpace;


struct TagItem *readOTag(struct DiskfontLibrary *dfl,
	char *fontPath, char *otagName)
{
    ULONG otFile;
    struct TagItem tag, *tags;
    struct Library *glyphLibrary;
    char tempName[256], *s, failure;
    int error, i;

    tags = 0;
    failure = TRUE;

    /* generate .otag file path */
    strcpy(tempName, fontPath);

    s = FilePart(tempName);
    strcpy(s, otagName);
    strcat(s, OTSUFFIX);

    D((".otag name %s\n", tempName));

    /* open .otag file */
    if (!(otFile = Open(tempName, MODE_OLDFILE))) {
	D(("no associated .otag file\n"));
	goto OTEND;
    }

    /* read and verify the .otag */
    if (Read(otFile, &tag, sizeof(tag)) != sizeof(tag)) {
	D(("otFile read fail (8 bytes)\n"));
	goto OTEND;
    }
    if (tag.ti_Tag != OT_FileIdent) {
	D((".otag first tag $%lx, not $%lx\n", tag.ti_Tag, OT_FileIdent));
	goto OTEND;
    }

    D(("OT_FileIdent %ld\n", tag.ti_Data));

    Seek(otFile, 0, OFFSET_END);
    if (Seek(otFile, 0, OFFSET_BEGINNING) != tag.ti_Data) {
	D((".otag file size wrong\n"));
	goto OTEND;
    }
    if (!(tags = (struct TagItem *) AllocVec(tag.ti_Data, 0))) {
	D(("tags allocation failure\n"));
	goto OTEND;
    }

    /* this is a valid .otag file header */
    if (Read(otFile, tags, tag.ti_Data) != tag.ti_Data) {
	D(("tags read failure\n"));
	goto OTEND;
    }

    /* patch indirect pointers */
    for (i = 0; i < tag.ti_Data/sizeof(struct TagItem); i++) {
	if (tags[i].ti_Tag == TAG_DONE)
	    break;
	if (tags[i].ti_Tag & OT_Indirect)
	    tags[i].ti_Data += (ULONG) tags;
    }

    /* ensure the associated glyph library is open */
    /* get the library name */
    if (!(s = (char *) GetTagData(OT_Engine, 0, tags))) {
	D(("no OT_Engine tag\n"));
	goto OTEND;
    }
    if ((glyphEngine)
	    /* validate existing engine */
	    && strcmp(s, glyphEngine->gle_Name)) {
	/* different engine name */
	glyphLibrary = glyphEngine->gle_Library;
	ECloseEngine(glyphEngine);
	glyphEngine = 0;
	CloseLibrary(glyphLibrary);
    }
    if (!glyphEngine) {
	/* get new engine */
	strcpy(tempName, s);
	strcat(tempName, ".library");
	glyphLibrary = OpenLibrary(tempName, 0);
	if (!glyphLibrary) {
	    D(("OpenLibrary \"%s\" failure\n", tempName));
	    goto OTEND;
	}
	glyphEngine = (struct GlyphEngine *) EOpenEngine(glyphLibrary);
	if (!glyphEngine) {
	    D(("OpenEngine \"%s\" failure\n", tempName));
	    CloseLibrary(glyphLibrary);
	    goto OTEND;
	}
    }

    error = ESetInfo(glyphEngine, OT_OTagPath, fontPath,
	    OT_OTagList, tags, TAG_DONE);

    if (error) {
	D(("ESetInfo(OT_OTag...) failed %ld\n", error));
	glyphLibrary = glyphEngine->gle_Library;
	ECloseEngine(glyphEngine);
	CloseLibrary(glyphLibrary);
	glyphEngine = 0;
	goto OTEND;
    }

    failure = FALSE;

OTEND:
    if (otFile)
	Close(otFile);
    if (failure) {
	FreeVec(tags);
	tags = 0;
    }
    return(tags);
}


UWORD imageCode(struct DiskFontHeader *dfh, UWORD code, UWORD bitstart,
	UBYTE forceWhite)
{
    struct GlyphMap *cbm;
    UWORD bitwidth;
    short kern, space;
    UWORD srcMod, *srcY, *src, srcShift, srcEnd;
    UWORD blackTop, blackHeight;
    WORD yOrigin;
    UWORD *destY, *dest, destShift;
    short x, y;

    if (cbm = charMaps[code]) {
	bitwidth = cbm->glm_BlackWidth;
	if (!bitwidth) {
	    ((ULONG *) dfh->dfh_TF.tf_CharLoc)[code-loCode] = 0;
	    ((WORD *) dfh->dfh_TF.tf_CharKern)[code-loCode] = 0;
	    ((WORD *) dfh->dfh_TF.tf_CharSpace)[code-loCode] =
		    cbm->glm_X1-cbm->glm_X0;
	    DD(("code %ld, no bits, CharSpace %ld\n",
		code, cbm->glm_X1-cbm->glm_X0));
	    return(0);
	}
	((ULONG *) dfh->dfh_TF.tf_CharLoc)[code-loCode] =
		(bitstart<<16) | bitwidth;
 	/* rounded off */
	kern = cbm->glm_BlackLeft-cbm->glm_X0;
 	space = cbm->glm_X1-cbm->glm_X0-kern;
	if (forceWhite) {
	    if (forceWhite & 0x02) {
		DD(("kern %ld -> ", kern));
		/* force left */
		if (kern <= 0)
		    kern++;
		DD(("%ld\n", kern));
	    }
	    else {
		/* force right */
		DD(("bitwidth %ld space %ld -> ", bitwidth, space));
		if (space <= bitwidth)
		    space++;
		DD(("%ld\n", space));
	    }
	}
	((WORD *) dfh->dfh_TF.tf_CharKern)[code-loCode] = kern;
	((WORD *) dfh->dfh_TF.tf_CharSpace)[code-loCode] = space;
	DD(("code %ld, CharLoc $%08lx, CharSpace %ld, CharKern %ld\n",
		code, ((ULONG *) dfh->dfh_TF.tf_CharLoc)[code-loCode],
		space, kern));

	yOrigin = cbm->glm_Y0;
	blackTop = cbm->glm_BlackTop;
	blackHeight = cbm->glm_BlackHeight;

	/* truncate to ySize */
	if (blackHeight > ySize) {
	    DS(("truncate %ld blackHeight %ld to %ld, blackTop %ld to %ld\n",
		    code, blackHeight, ySize,
		    blackTop, blackTop+(ySize-blackHeight)/2));
	    blackTop += (blackHeight-ySize)/2;
	    blackHeight = ySize;
	}
	if ((yBaseOrigin+blackTop+blackHeight-ySize) > yOrigin) {
	    D(("shift %ld yOrigin up (%ld to %ld)\n",
		    code, yOrigin, yBaseOrigin+blackTop+blackHeight-ySize));
	    yOrigin = yBaseOrigin+blackTop+blackHeight-ySize;
	}
	else if ((yBaseOrigin+blackTop) < yOrigin) {
	    D(("shift %ld yOrigin down (%ld to %ld)\n",
		    code, yOrigin, yBaseOrigin+blackTop));
	    yOrigin = yBaseOrigin+blackTop;
	}
	srcMod = cbm->glm_BMModulo>>1;
	srcY = ((UWORD *) (cbm->glm_BitMap+(cbm->glm_BMModulo*blackTop))) +
		(cbm->glm_BlackLeft>>4);
	srcShift = cbm->glm_BlackLeft&0xf;
	srcEnd = cbm->glm_BlackWidth+srcShift;
	srcShift = 0x10-srcShift;
	destY = ((UWORD *) dfh->dfh_TF.tf_CharData)+(bitstart>>4)+(destMod*
		(yBaseOrigin-(yOrigin-blackTop)));
	destShift = 0x10-(bitstart&0xf);
	DD(("  srcY $%lx, srcMod %ld, srcShift %ld, srcEnd %ld\n",
		srcY, srcMod, srcShift, srcEnd));
	DD(("  destY $%lx, destShift %ld\n",
		destY, destShift));
	for (y = blackHeight; y > 0; y--) {
	    src = srcY;
	    dest = destY;
	    for (x = srcEnd; x > 0; x-=16) {
		DD(("  dest $%04lx @ $%lx, src $%04lx-%04lx : %08lx\n",
			*dest, dest, src[0], src[1],
			(((*((ULONG *) src))>>srcShift)&0xffff)<<destShift));
		*((ULONG *) dest) |=
			(((*((ULONG *) src))>>srcShift)&0xffff)<<destShift;
		src++;
		dest++;
	    }
	    srcY += srcMod;
	    destY += destMod;
	}
	if (code == undefCode) {
	    undefLoc = (bitstart<<16) | bitwidth;
	    undefKern = kern;
	    undefSpace = space;
	}
    }
    else {
	bitwidth = 0;

	if (code == undefCode) {
	    undefLoc = 0;
	    undefKern = 0;
	    undefSpace = 0;
	}

	if ((code == ' ') || (code == 0xa0)) {
	    ((UWORD *) dfh->dfh_TF.tf_CharSpace)[code-loCode] = spaceWidth;
	}
	else {
	    ((ULONG *) dfh->dfh_TF.tf_CharLoc)[code-loCode] = undefLoc;
	    ((WORD *) dfh->dfh_TF.tf_CharKern)[code-loCode] = undefKern;
	    ((WORD *) dfh->dfh_TF.tf_CharSpace)[code-loCode] = undefSpace;
	}
	DD(("code %ld, CharLoc $%08lx, CharSpace %ld, CharKern %ld (undef)\n",
		code, ((ULONG *) dfh->dfh_TF.tf_CharLoc)[code-loCode],
		undefKern, undefSpace));
    }
    return(bitwidth);
}


struct TextFont * __saveds GenFont(struct DiskfontLibrary *dfl, char *fontPath,
	struct TTextAttr *tta, struct FontContentsHeader *fch,
	struct TFontContents *tfc, struct TextFont *tfRam,
	WORD wRam)
{
    struct TagItem *tags, *ti;
    UWORD dpiX, dpiY, dpiX2, dpiY2;
    UWORD dotX, dotY;
    FIXED points, temp;
    UWORD ySize2m, ySize2o, ySize2b;
    union {
	ULONG t;			/* tag */
	struct {
	    UWORD n;			/* numerator */
	    UWORD d;			/* denominator */
	} w;
    } ySizeFactor;
    struct TTextAttr ttaLocal;
    struct TagItem dpiTags[3];
    UWORD *sizes;
    struct DiskFontHeader *dfh;
    struct TextFont *tf;
    struct Library *glyphLibrary;
    struct GlyphMap *cbm;
    ULONG *p, allocSize;
    char *s;
    short code, hiCode, ascent, descent, yOrigin;
    short whiteLeft, whiteRight, whiteBoth;
    UWORD w, bitwidth, bitstart;
    int i, cnt, x, y;
    short xSize, width, width2, kern, space;
    long error;
    char otName[32];
    UBYTE tfStyle;

    D(("GenFont(,\"%s\", { \"%s\", %ld, $%lx, $%lx, $%lx }, ) \n",
	    fontPath, tta->tta_Name, tta->tta_YSize, tta->tta_Style,
	    tta->tta_Flags, tta->tta_Tags));

    tags = 0;
    for (code = 0; code < 257; code++) {
	charMaps[code] = 0;
    }
    glyphEngine = 0;
    dfh = 0;
    tf = 0;

    if (tta->tta_Flags & FPF_REVPATH)
	/* cannot do REVPATH work here */
	goto gfEndgame;

    /* get associated .otag file */
    strcpy(otName, tta->tta_Name);
    if (!(s = strrchr(otName, '.'))) {
	D(("no '.' in fontPath name\n"));
	goto gfEndgame;
    }
    *s = '\0';
    tags = readOTag(dfl, fontPath, otName);

    if (!tags) {
	D(("no valid .otag file\n"));
	goto gfEndgame;
    }

    tfStyle = FSF_TAGGED;
    if (GetTagData(OT_StemWeight, OTS_Medium, tags) >=
	    ((OTS_Medium+OTS_SemiBold)/2))
	tfStyle |= FSF_BOLD;
    if (GetTagData(OT_SlantStyle, OTS_Upright, tags) != OTS_Upright)
	tfStyle |= FSF_ITALIC;
    if (GetTagData(OT_HorizStyle, OTH_Normal, tags) >=
	    ((OTH_Normal+OTH_SemiExpanded)/2))
	tfStyle |= FSF_EXTENDED;

#if 0
    /* find alternate font for style */
    s = 0;
    x = GetTagData(OT_StemWeight, OTS_Medium, tags);
    y = GetTagData(OT_SlantStyle, OTS_Upright, tags);
    if ((tta->tta_Style & (FSF_ITALIC|FSF_BOLD)) == (FSF_ITALIC|FSF_BOLD)) {
	if ((x < ((OTS_Medium+OTS_SemiBold)/2)) && (y == OTS_Upright)) {
	    s = GetTagData(dfl, OT_BIName, tags);
	    tfStyle = FSF_ITALIC|FSF_BOLD|FSF_TAGGED;
	}
    }
    if ((!s) && (tta->tta_Style & FSF_ITALIC)) {
	/* ensure italic */
	if (y == OTS_Upright) {
	    s = GetTagData(dfl, OT_IName, tags);
	    tfStyle = FSF_ITALIC|FSF_TAGGED;
	}
    }
    if ((!s) && (tta->tta_Style & FSF_BOLD)) {
	/* ensure bold */
	if (x < ((OTS_Medium+OTS_SemiBold)/2)) {
	    s = GetTagData(dfl, OT_BName, tags);
	    tfStyle = FSF_BOLD|FSF_TAGGED;
	}
    }
    if (s) {
	if (ti = readOTag(dfl, fontPath, s)) {
	    FreeVec(tags);
	    tags = ti;
	    strcpy(otName, s);
	}
	else
	    tfStyle = FSF_TAGGED;
    }
#endif

    /* get default environment */
    if (ti = FindTagItem(OT_DeviceDPI, tags)) {
	dpiX = ti->ti_Data>>16;
	dpiY = ti->ti_Data & 0xffff;
    }
    else {
	dpiX = dfl->dfl_XDPI;
	dpiY = dfl->dfl_YDPI;
    }
    if (ti = FindTagItem(OT_DotSize, tags)) {
	dotX = ti->ti_Data>>16;
	dotY = ti->ti_Data & 0xffff;
    }
    else {
	dotX = dfl->dfl_DotSizeX;
	dotY = dfl->dfl_DotSizeY;
    }

    /* determine size metrics */
    points = 0;
    D(("default: dpiX %ld, dpiY %ld, dotX %ld, dotY %ld\n",
	    dpiX, dpiY, dotX, dotY));
    if (tta->tta_Style & FSF_TAGGED) {
	D(("look at tta_Tags at $%lx\n", tta->tta_Tags));
	if (ti = FindTagItem(OT_DeviceDPI, tta->tta_Tags)) {
	    dpiX = ti->ti_Data>>16;
	    dpiY = ti->ti_Data & 0xffff;
	}
	if (ti = FindTagItem(OT_DotSize, tta->tta_Tags)) {
	    dotX = ti->ti_Data>>16;
	    dotY = ti->ti_Data & 0xffff;
	}
	points = GetTagData(OT_PointHeight, 0, tta->tta_Tags);
	if (points == 0)
	    points = GetTagData(OT_PointSize, 0, tta->tta_Tags) << 12;
	D(("after tag: dpiX %ld, dpiY %ld, dotX %ld, dotY %ld\n",
		dpiX, dpiY, dotX, dotY));
    }
    ySizeFactor.t = GetTagData(OT_YSizeFactor, 0x10001, tags);
    if (!points) {
	ySize = tta->tta_YSize;

	if (tta->tta_Flags & FPF_DESIGNED) {
	    D(("designed...\n"));
	    /* no scaling, find greatest smaller size */
	    ySize2m = 32768;
	    ySize2o = ySize2b = 0;
	    if (sizes = (UWORD *) GetTagData(OT_AvailSizes, 0, tags)) {
		cnt = *sizes++;
		D(("%ld o sizes\n", cnt));
		if (cnt) {
		    ySize2m = *sizes;
		    D(("first %ld <= %ld?\n", ySize2m, ySize));
		    if (ySize2m <= ySize) {
			for (; (cnt > 0) & (*sizes <= ySize); cnt--, sizes++)
			    D(("  %ld <= %ld?\n", *sizes, ySize));
			ySize2o = *--sizes;
			D(("  ySize2o %ld\n", ySize2o));
		    }
		}
	    }

	    for (i = 0; i < fch->fch_NumEntries; i++) {
		if (tfc[i].tfc_YSize < ySize2m)
		    ySize2m = tfc[i].tfc_YSize;
		if ((tfc[i].tfc_YSize <= ySize) &&
			(tfc[i].tfc_YSize > ySize2b)) {
		    ySize2b = tfc[i].tfc_YSize;
		    D(("  ySize2b %ld\n", ySize2b));
		}
	    }

	    ySize2o = (ySize2o > ySize2b) ? ySize2o : ySize2b;
	    if (ySize2o)
		ySize = ySize2o;
	    else if (ySize2m != 32767)
		ySize = ySize2m;
	}

	D(("  ySizeFactor %ld / %ld\n", ySizeFactor.w.n, ySizeFactor.w.d));
#ifdef  YSIZEFACTOR
	dpiX = 72 * dpiX / dpiY * ySizeFactor.w.n / ySizeFactor.w.d;
	dpiY = 72 * ySizeFactor.w.n / ySizeFactor.w.d;
#else
	dpiX = 72 * dpiX / dpiY;
	dpiY = 72;
#endif
	points = ySize<<16;
    }
    else {
#ifdef  YSIZEFACTOR
	ySize = ((points / 72 * dpiY / ySizeFactor.w.n * ySizeFactor.w.d)
		+0x8000)>>16;
#else
	ySize = (points+0x8000)>>16;
#endif
    }

    /* Determine if engine can do underlining if requested; default must
       be no underlining per docs/specs.   We prefer to have Broken
       underlining like gfx Text() would do, but will accept Solid if
       Broken is not supported.

       Assumptions:

	Underlined is not an intrinsic font style, but rather something
	generated by the engine 
    */

    if(tta->tta_Style & FSF_UNDERLINED)
    {
	D(("Underlined requested\n"));

	    /* Set in outline style, but clear in copy of TextAttr */

	    tfStyle |= FSF_UNDERLINED;
	    tta->tta_Style &= ~FSF_UNDERLINED;

	    /* Ask engine for Broken underlining, or Solid, but if neither,
	       this engine cannot underline, and we do not bother searching
	       for an underlined ram font.
	    */

	    if (error = ESetInfo(glyphEngine, OT_UnderLined, OTUL_Broken,
	    	TAG_DONE))
	    {
		if(error = ESetInfo(glyphEngine, OT_UnderLined, OTUL_Solid,
		   TAG_DONE))
		{
			D(("OTERR %ld - Underlining not supported\n",(ULONG)error));
			tfStyle &= ~FSF_UNDERLINED;
		}
	    }
    }

    /* re-look for memory match */
    s = otName + strlen(otName);
    strcpy(s, ".font");
    ttaLocal.tta_Name = otName;
    ttaLocal.tta_YSize = ySize;
    ttaLocal.tta_Style = tfStyle | tta->tta_Style;	/* tfStyle is TAGGED */
    ttaLocal.tta_Flags = 0;
    dpiTags[0].ti_Tag = TA_DeviceDPI;
    dpiTags[0].ti_Data = (dpiX<<16)|dpiY;
    dpiTags[1].ti_Tag = OT_DotSize;
    dpiTags[1].ti_Data = (dotX<<16)|dotY;
    dpiTags[2].ti_Tag = TAG_DONE;
    ttaLocal.tta_Tags = dpiTags;
    tf = OpenFont((struct TextAttr *) &ttaLocal);
    D(("OpenFont({ \"%s\", %ld, $%02lx, $%02lx, })...\n", otName, ySize,
	    ttaLocal.tta_Style, ttaLocal.tta_Flags));
    if (tf) {
	D(("  ram YSize %ld, Style 0x%02lx, Flags0x%02lx\n",
	    tf->tf_YSize, tf->tf_Style, tf->tf_Flags));
	/* get the weight for the resulting ram font */
	w = WeighTAMatch((struct TextAttr *) &ttaLocal,
		(struct TextAttr *) (((ULONG) &tf->tf_YSize)-4),
		((struct TextFontExtension *) tf->tf_Extension)->tfe_Tags);
	D(("tf 0x%lx, w %ld\n", tf, w));
        if (w == MAXFONTMATCHWEIGHT) {
            /* exact match: cannot get better from here! */
	    if (tfRam)
		CloseFont(tfRam);
	    tfRam = 0;
            goto gfEndgame;
	}
	/* not good enough */
	CloseFont(tf);
    }

    /* look for disk match */
    for (i = 0; i < fch->fch_NumEntries; i++) {
	if (tfc[i].tfc_YSize != ySize)
	    continue;
	if (tfc[i].tfc_TagCount) {
	    D(("tfc_TagCount %ld\n", tfc[i].tfc_TagCount));
	    ti = (struct TagItem *) &tfc[i].tfc_FileName[MAXFONTPATH-
		    (tfc[i].tfc_TagCount*sizeof(struct TagItem))];
	    if (ti = FindTagItem(OT_DeviceDPI, ti)) {
		dpiX2 = ti->ti_Data>>16;
		dpiY2 = ti->ti_Data & 0xffff;
		if ((dpiX * dpiY2) != (dpiX2 * dpiY))
		    continue;
	    }
	}
	/* open this disk font */
	D(("Load disk font %s path %s\n", tta->tta_Name, fontPath));
	tf = (struct TextFont *)
		loadDiskFont(dfl, tfc+i, tta->tta_Name, fontPath);
	if (tf)
	    goto gfEndgame;
	D(("  font path %s unchanged?\n", fontPath));
	break;
    }

    temp = GetTagData(OT_SpaceFactor, 0, tags);
    if (temp) {
	D(("OT_SpaceFactor $%08lx\n", temp));
	spaceWidth = (((GetTagData(OT_SpaceFactor, 0, tags) * ySize) + 0x8000) /
	    dpiY * dpiX) >> 16;
    }
    else {
	D(("No OT_SpaceFactor tag\n"));
	spaceWidth = GetTagData(OT_SpaceWidth, 0, tags) * dpiX / 254
		* (points>>12) / 40000;
    }
    if (spaceWidth == 0) {
	D(("No OT_SpaceWidth tag\n"));
	spaceWidth = ySize * dpiX / dpiY / 3;	/* thinspace */
    }
    if (spaceWidth == 0) {
	D(("spaceWidth (0!) = ySize (%ld) * dpiX (%ld) / dpiY (%ld) / 3\n",
		ySize, dpiX, dpiY));
	spaceWidth = 1;
    }
    D(("ySize %ld, points %ld.%04ld, dpiXY %ld, %ld, dotXY %ld, %ld, sp %ld\n",
	    ySize, points>>16, ((points&0xffff)*10000)>>16,
	    dpiX, dpiY, dotX, dotY, spaceWidth));

    if (error = ESetInfo(glyphEngine, OT_PointHeight, points,
	    OT_DeviceDPI, (dpiX<<16)|dpiY, OT_DotSize, (dotX<<16)|dotY,
	    TAG_DONE)) {
	D(("ESetInfo(%ld.%04ld %ld %ld...) failed %ld\n",
		points>>16, ((points&0xffff)*10000)>>16, dpiX, dpiY, error));
	goto gfEndgame;
    }

    if ((tta->tta_Style & FSF_BOLD) && (!(tfStyle & FSF_BOLD)) &&
	    (!(GetTagData(OT_InhibitAlgoStyle, 0, tags) & FSF_BOLD))) {
	D(("set bold."));
	if (!ESetInfo(glyphEngine,
		/* use same values as bullet does for screen fonts */
		OT_EmboldenX, 0x00000e75, OT_EmboldenY, 0x0000099e, TAG_DONE)) {
	    tfStyle |= FSF_BOLD;
	    D(("  successfully."));
	}
	D(("\n"));
    }
    if ((tta->tta_Style & FSF_ITALIC) && (!(tfStyle & FSF_ITALIC)) &&
	    (!(GetTagData(OT_InhibitAlgoStyle, 0, tags) & FSF_ITALIC))) {
	D(("set italic."));
	if (!ESetInfo(glyphEngine,
		/* 16 degrees: between CGTimesItalic (15.6) &
		 *   UniversMediumItalic (16.5)
		 */
		OT_ShearSin, 0x00004690, OT_ShearCos, 0x0000f615, TAG_DONE)) {
	    tfStyle |= FSF_ITALIC;
	    D(("  successfully."));
	}
	D(("\n"));
    }

    /* gather character definitions */
    ascent = descent = bitwidth = hiCode = 0;
    whiteLeft = whiteRight = whiteBoth = 0;
    loCode = ' ';			/* space is required */
    for (code = 0x00; code <= 0x100; code++) {
	DD(("  code %ld $%lx\n", code, code));
	if (code == 0x100)
	    error = ESetInfo(glyphEngine, OT_GlyphCode, 0x25a1 /* en-box */,
		    TAG_DONE);
	else
	    error = ESetInfo(glyphEngine, OT_GlyphCode, code,
		    TAG_DONE);
	if (error) {
	    D(("ESetInfo(OT_GlyphCode, ...$%lx) failed %ld\n", code, error));
	    continue;
	}
	error = EObtainInfo(glyphEngine, OT_GlyphMap, &cbm, TAG_DONE);
	if (error) {
	    D(("EObtainInfo(OT_GlyphMap) failed %ld\n", error));
	    continue;
	}

	    charMaps[code] = cbm;
	    DD(("    width %ld, height %ld\n",
		    cbm->glm_BMModulo, cbm->glm_BMRows));
	    DD(("    blackLeft %ld, blackTop %ld\n",
		    cbm->glm_BlackLeft, cbm->glm_BlackTop));
	    DD(("    blackWidth %ld, blackHeight %ld\n",
		    cbm->glm_BlackWidth, cbm->glm_BlackHeight));
	    DD(("    xOrigin %ld.%ld, yOrigin %ld.%ld\n",
		    cbm->glm_XOrigin>>16,
		    (((cbm->glm_XOrigin<0)?(((~cbm->glm_XOrigin)&0xffff))-1:
		    cbm->glm_XOrigin&0xffff)*10000)>>16,
		    cbm->glm_YOrigin>>16,
		    (((cbm->glm_YOrigin<0)?(((~cbm->glm_YOrigin)&0xffff))-1:
		    cbm->glm_YOrigin&0xffff)*10000)>>16));
	    DD(("    escapement %ld.%ld\n",
		    cbm->glm_Escapement>>16,
		    (((cbm->glm_Escapement<0)?
		    (((~cbm->glm_Escapement)&0xffff))-1:
		    cbm->glm_Escapement&0xffff)*10000)>>16));
#ifdef  DEBUG
	    for (y = 0; y < cbm->glm_BMRows; y++) {
		for (x = 0; x < cbm->glm_BMModulo; x++) {
		    UBYTE bits;

		    bits = cbm->glm_BitMap[y*cbm->glm_BMModulo+x];
		    for (i = 7; i >= 0; i--) {
			if (bits & (1<<i))
			    DD(("*"));
			else
			    DD(("-"));
		    }
		}
		DD(("\n"));
	    }
#endif

	    /* min & max */
	    if (cbm->glm_BlackWidth) {
	    yOrigin = cbm->glm_Y0;
	    if (ascent < (yOrigin-cbm->glm_BlackTop)) {
		ascent = yOrigin-cbm->glm_BlackTop;
		D(("code %ld, ascent now %ld\n", code, ascent));
	    }
	    if (descent < (cbm->glm_BlackTop+cbm->glm_BlackHeight-yOrigin)) {
		descent = cbm->glm_BlackTop+cbm->glm_BlackHeight-yOrigin;
		D(("code %ld, descent now %ld\n", code, descent));
	    }
	    DD(("code %ld x0 %ld x1 %ld bl %ld bw %ld",
		     code, cbm->glm_X0, cbm->glm_X1,
		     cbm->glm_BlackLeft, cbm->glm_BlackWidth));
	if ((code >= 'a') && (code <= 'z')) {
	    if ((cbm->glm_BlackLeft+cbm->glm_BlackWidth) < cbm->glm_X1) {
		/* there is whiteRight */
		if (cbm->glm_X0 < cbm->glm_BlackLeft) {
		    DD((" wl. wb."));
		    whiteLeft++;
		    whiteBoth++;
		}
		DD((" wr."));
		whiteRight++;
	    }
	    else {
		if (cbm->glm_X0 < cbm->glm_BlackLeft) {
		    DD((" wl."));
		    whiteLeft++;
		}
	    }
	    DD(("\n"));
	}
	    bitwidth += cbm->glm_BlackWidth;
	    }
	    if (code < loCode) {
		loCode = code;
	    }
	    if ((code > hiCode) && (code != 0x100)) {
		hiCode = code;
	    }
    }

    /* generate diskfont */
    code = hiCode-loCode+2;
    yBaseOrigin = ascent;
    if ((ascent+descent) > ySize) {
	yBaseOrigin -= (ascent+descent-ySize+1)*2/3;
    }
    D(("code %ld..%ld, bitwidth %ld, ascent %ld descent %ld, yBaseOrig %ld\n",
	    loCode, hiCode, bitwidth, ascent, descent, yBaseOrigin));
    D(("whiteLeft %ld, whiteRight %ld, whiteBoth %ld\n",
	    whiteLeft, whiteRight, whiteBoth));
    if (code < 0)
	goto gfEndgame;
    destMod = ((bitwidth+15)/16)*2;
    allocSize = ((12+sizeof(struct DiskFontHeader)+(OFONT_TAGCNT*8)+7)+
	    (16*code)+(ySize*destMod))
	    & 0xfffffffc;		/* must be even longword size */
    if (!(p = (ULONG *) AllocMem(allocSize, MEMF_CLEAR))) {
	D(("AllocMem %ld failure (%ld+(16*%ld)+(%ld*%ld)\n", allocSize,
		12+sizeof(struct DiskFontHeader)+(OFONT_TAGCNT*8)+7,
		code, ySize, destMod));
	goto gfEndgame;
    }

    p[0] = allocSize;			/* set alloc size as LoadSeg would */
    p[2] = 0x70644e75;			/* moveq #100,d0; rts; */
    dfh = (struct DiskFontHeader *) (p+3);
    dfh->dfh_FileID = DFH_ID;
    strcpy(dfh->dfh_Name, tta->tta_Name);
    dfh->dfh_TF.tf_YSize = ySize;
    dfh->dfh_TF.tf_Style = tfStyle;
    dfh->dfh_TF.tf_Flags = FPF_PROPORTIONAL;
    dfh->dfh_TF.tf_Baseline = yBaseOrigin-1;
    dfh->dfh_TF.tf_BoldSmear = 1;
    dfh->dfh_TF.tf_LoChar = loCode;
    dfh->dfh_TF.tf_HiChar = hiCode;
    dfh->dfh_TF.tf_Modulo = destMod;
    dfh->dfh_Segment = (LONG)(dfh+1);
    dfh->dfh_TF.tf_CharLoc = (APTR) (dfh->dfh_Segment+(OFONT_TAGCNT*8));
    dfh->dfh_TF.tf_CharSpace = (APTR) (((ULONG) dfh->dfh_TF.tf_CharLoc)+
	    (code*4));
    dfh->dfh_TF.tf_CharKern = (APTR) (((ULONG) dfh->dfh_TF.tf_CharSpace)+
	    (code*2));
    dfh->dfh_TF.tf_CharData = (APTR) (((ULONG) dfh->dfh_TF.tf_CharKern)+
	    (code*2));

    ti = (struct TagItem *) dfh->dfh_Segment;
    ti->ti_Tag = OT_PointHeight;
    ti->ti_Data = points;
    ti++;
    ti->ti_Tag = OT_DeviceDPI;
    ti->ti_Data = (dpiX<<16)|dpiY;
    ti++;
    ti->ti_Tag = OT_DotSize;
    ti->ti_Data = (dotX<<16)|dotY;
    ti++;
    ti->ti_Tag = TAG_DONE;
    ti->ti_Data = 0;

    D(("YSize %ld, Baseline %ld, Modulo %ld\n", dfh->dfh_TF.tf_YSize,
	     dfh->dfh_TF.tf_Baseline, dfh->dfh_TF.tf_Modulo));
    DD(("tf_CharData $%lx\n", dfh->dfh_TF.tf_CharData));

    destMod /= 2;

    undefCode = hiCode+1;
    charMaps[undefCode] = charMaps[0x100];
    x = 0x00;				/* guess no adjustment needed */
    y = 26-ySize;
    if ((whiteBoth < y) && (whiteLeft < 24) && (whiteRight < 24)) {
	if (whiteLeft < whiteRight)
	    x = 0x02;			/* ensure white left bearing */
	else
	    x = 0x01;			/* ensure white right bearing */
    }
    DS(("size %ld White Left %ld Right %ld Both %ld Neither %ld force %ld\n",
	    dfh->dfh_TF.tf_YSize, whiteLeft, whiteRight, whiteBoth,
	    whiteBoth-whiteLeft-whiteRight+26, x));
    bitstart = imageCode(dfh, undefCode, 0, x);	/* image undefined character */
    for (code = loCode; code <= hiCode; code++) {
	bitstart += imageCode(dfh, code, bitstart, x);
    }

    if (GetTagData(OT_IsFixed, 0, tags)) {
	/* force a fixed width typeface */
	D(("IsFixed: spaceWidth %ld\n", spaceWidth));
	xSize = spaceWidth;
	for (code = hiCode-loCode+1; code >= 0; code--) {
	    width = (((WORD *) dfh->dfh_TF.tf_CharKern)[code]+
		    ((WORD *) dfh->dfh_TF.tf_CharSpace)[code]);
	    if (xSize < width) {
		xSize = width;
		if (xSize > (spaceWidth+2)) {
		    xSize = spaceWidth+2;	/* that's the limit */
		    break;
		}
	    }
	}

	/* force everything to xSize */
	dfh->dfh_TF.tf_XSize = xSize;
	dfh->dfh_TF.tf_Flags = 0;		/* not PROPORTIONAL */
	for (code = hiCode-loCode+1; code >= 0; code--) {
	    kern = ((WORD *) dfh->dfh_TF.tf_CharKern)[code];
	    space = ((WORD *) dfh->dfh_TF.tf_CharSpace)[code];
	    bitwidth = ((ULONG *) dfh->dfh_TF.tf_CharLoc)[code]&0xffff;
	    width = xSize - kern - space;	/* width adjustment */
	    width2 = width/2;
	    D(("c $%02lx w %ld k %ld s %ld b %ld ? %ld, ", code, width, kern,
		    space, bitwidth, (space-bitwidth)));
	    if ((short)(space-bitwidth) >= kern) {
		/* more whitespace at right than at left */
		D(("+L "));
		kern += width-width2;
		space += width2;
	    }
	    else {
		/* more whitespace at left than at right */
		D(("+R "));
		kern += width2;
		space += width-width2;
	    }
	    if ((kern < 0) && ((short)(space-bitwidth) > 0)) {
		/* pull in kern to center */
		if (-kern > (short)(space-bitwidth)) {
		    kern += (short)(space-bitwidth);
		    space = bitwidth;
		}
		else {
		    space += kern;
		    kern = 0;
		}
	    }
	    else if ((kern > 0) && ((short)(space-bitwidth) < 0)) {
		/* pull in kern to center */
		if (kern > -(short)(space-bitwidth)) {
		    kern += (short)(space-bitwidth);
		    space = bitwidth;
		}
		else {
		    space += kern;
		    kern = 0;
		}
	    }
	    D(("k2 %ld s2 %ld\n", kern, space));
	    ((WORD *) dfh->dfh_TF.tf_CharKern)[code] = kern;
	    ((WORD *) dfh->dfh_TF.tf_CharSpace)[code] = space;
	}
    }
    else {
	if ((loCode <= 'n') && ('n' <= hiCode))
	    spaceWidth = ((WORD *) dfh->dfh_TF.tf_CharKern)['n'-loCode] +
		    ((WORD *) dfh->dfh_TF.tf_CharSpace)['n'-loCode];
	dfh->dfh_TF.tf_XSize = spaceWidth;
    }
    D(("XSize %ld\n", dfh->dfh_TF.tf_XSize));

gfEndgame:
    D(("gfEndGame:\n"));
    if (glyphEngine) {
	glyphLibrary = glyphEngine->gle_Library;
	D(("ECloseEngine...\n"));
	ECloseEngine(glyphEngine);
	CloseLibrary(glyphLibrary);
    }
    D(("FreeVec things...\n"));
    if (tags)
	FreeVec(tags);
    if (dfh) {
	D(("try to patch dfh\n"));
	if (!(tf = (struct TextFont *) PatchDiskFont(dfl, dfh))) {
	    D(("  failed, UnLoadSeg\n"));
	    UnLoadSeg(dfh->dfh_Segment);
	}
    }
    if (!tf) {
	D(("OpenWeighedFont...\n"));
	tf = (struct TextFont *)
		OpenWeighedFont(dfl, fontPath, tta, fch, tfc, tfRam, wRam);
    }

    if ((tfRam) && (tf != tfRam)) {
	D(("Close tfRam font\n"));
	CloseFont(tfRam);
    }

    D(("GenFont result $%lx { \"%s\", %ld, $%lx, $%lx }\n",
	    tf, tf->tf_Message.mn_Node.ln_Name,
	    tf->tf_YSize, tf->tf_Style, tf->tf_Flags));
    return(tf);
}
