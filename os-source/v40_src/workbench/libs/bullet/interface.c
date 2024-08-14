/*
 *	$Id: interface.c,v 9.4 92/07/13 11:26:05 darren Exp $
 *
 *	(C) Copyright 1991 Robert R. Burns
 *	    All Rights Reserved
 *
 *	$Log:	interface.c,v $
 * Revision 9.4  92/07/13  11:26:05  darren
 * Clean up spelling errors, reference to OTAG_ID, and reference to
 * struct Glyph (change to struct GlyphMap)
 * 
 * Revision 9.3  92/06/19  11:18:17  darren
 * change all refs of libraries/ to diskfont/
 * 
 * Revision 9.2  92/05/21  17:03:02  davidj
 * GetGlyphMap now returns error.
 * 
 * Revision 9.1  92/02/03  12:05:52  davidj
 * turned autodoc comments into real autodocs
 *
 * Revision 9.0  91/04/09  19:59:07  kodiak
 * GlyphMap escapement is now like width -- an em fraction
 *
 * Revision 8.1  91/03/28  15:06:38  kodiak
 * fix OTagPath typeface file path creation
 *
 * Revision 8.0  91/03/24  12:16:59  kodiak
 * Phase 2 Beta 1 (38.2)
 *
 * Revision 7.0  91/03/19  18:36:22  kodiak
 * after Amigaizing memory, lists, and adding UniCode.  Before cache work.
 *
 * Revision 5.1  91/03/18  08:57:14  kodiak
 * eliminate old style symbol table lookup
 *
 * Revision 5.0  91/02/26  10:52:01  kodiak
 * Phase 2 Alpha 1
 *
 */

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <dos/dos.h>

#include "debug.h"
#include "port.h"
#include "cgif.h"
#include "cgconfig.h"
#include "diskfonttag.h"
#include "glyph.h"
#include "math.h"
#include "oterrors.h"

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/utility_pragmas.h>

#define  BULLET_SUBDIR	"_Bullet_Outlines"
#define  CASE(id)	case (id&0xffff)

struct Library *SysBase;
struct Library *DOSBase;
struct Library *UtilityBase;
struct Library *BulletBase;

#ifdef  DEBUG
ULONG far _FPERR;
ULONG far _SIGFPE;
#endif

void  CExpunge(void);
ULONG COpenEngine(void);
void  CCloseEngine(void);
ULONG CSetInfoA(struct TagItem *tagList);
ULONG CObtainInfoA(struct TagItem *tagList);
ULONG CReleaseInfoA(struct TagItem *tagList);
ULONG CGetGlyphMap(ULONG glyphCode, struct GlyphMap **glyphMap);

/* mem.c */
void *MemAlloc(int);
void MemFree(void *);

MLOCAL void *ccBufPtr = 0;

MLOCAL UBYTE engineEntered = FALSE;
MLOCAL UBYTE shearUndefined = FALSE;
MLOCAL UBYTE rotateUndefined = FALSE;

MLOCAL FONTCONTEXT fc = {
    0, 0, 0, 0, 0, 0, 0, {0, 0x10000}, {0x10000, 0}, 0, 0,
    0x10000, 0x10000, 0, 0, 0x4C31 /* L1 */, 0<<14
};
MLOCAL IFCONFIG configBlock;

MLOCAL ULONG glyphCode = 0;
MLOCAL ULONG glyphCode2 = 0;
MLOCAL ULONG glyphWidth = 0;
MLOCAL ULONG pointHeight = 0;
MLOCAL ULONG setFactor = 0x10000;

MLOCAL struct GlyphMap *prevGlyph = 0;

MLOCAL struct MinList vecList = {
    (struct MinNode *) &vecList.mlh_Tail, 0,
    (struct MinNode *) &vecList.mlh_Head
};

/****** bullet.library/--background-- **************************************
*
*   AMIGA FONTS
*	The existence of a disk font is indicated by the existence of
*	its associated font contents file, whose name has the suffix
*	".font".  It is this name that is used in the actual font
*	open call of the application.  Amiga fonts are collected in
*	the directory path(s) associated with the FONTS: assign.  It
*	is this assign that is searched if no explicit path name is
*	provided in the font open call: use of an explicit path is
*	generally discouraged.  The actual bitmaps of traditional
*	Amiga fonts are stored in a directory with the name of the
*	font contents file stripped of its ".font" suffix.  This
*	directory is usually in the same directory as the font
*	contents file.  This traditional arrangement is supported by
*	the FixFonts system application.
*
*	For example:
*	    o   FONTS: is assigned to Sys:Fonts/
*	    o   Sys:Fonts/garnet.font exists and describes that the
*		font "garnet.font" exists
*	    o   Sys:Fonts/garnet/ contains the bitmap images for
*		sizes 9 and 16
*
*	Other variations of file placement may exist, but they
*	require custom tools to maintain -- tools available not from
*	Commodore, but from other sources such as Fish disks.
*
*	Font contents files are flagged with magic numbers that not
*	only verify that they are a font contents files but also what
*	variation of file structure they contain.
*
*   OUTLINE TYPEFACES
*	The existence of an outline typeface is indicated by a magic
*	number in the font contents file.  They are further described
*	in the associated outline typeface tag file, whose name is
*	the that of the font contents file with the suffix ".otag"
*	substituted for ".font".  This tag file contains a tag list
*	that is to be processed and passed to the outline engine
*	(i.e.  bullet.library) in order to select the associated
*	typeface.  It also contains information applications may use
*	to guide their use of the typeface.
*
*   OTAG SPECIFICATION EXAMPLE
*	Here are the steps necessary to go from an arbitrary font
*	name into an environment where glyphs from that font
*	can be accessed:
*	1.  Read the header from the font contents (.font) file and
*	    verify that the magic cookie fch_ID is OFCH_ID.
*	    If it is not, then this is an Amiga bitmap font,
*	    not an outline font.
*	2.  Read the associated outline tag (.otag) file into memory:
*	    a.  Validate that the OT_FileIdent exists and matches the
*		file size.
*	    b.  Allocate a memory buffer and read the file into it.
*	    c.  Resolve addresses: for each tag with the OT_Indirect
*		bit set, add the memory buffer origin to the
*		associated data.
*	3.  Find the OT_Engine tag and ensure that you have the
*	    proper engine open:
*	    a.  If you already have an engine handle for this engine
*		name, you skip these steps.
*	    b.  append the suffix ".library" to the engine name.
*		(e.g. "bullet" becomes "bullet.library").
*	    c.  use exec's OpenLibrary() to open the library.
*	    d.  use the engine's OpenEngine() to acquire an engine
*		handle.
*	4.  Pass the full path name of the .otag file to the engine
*	    with the OT_OTagPath tag using SetInfo().
*	5.  Pass the memory copy of the .otag file to the engine with
*	    the OT_OTagList tag using SetInfo().  This step may be
*	    combined with step 4, passing first the path then the
*	    list in one call.
*	The library is now ready to accept glyph metric information
*	(e.g. size and glyph code) and produce glyph bitmaps.
*
*   DISKFONT USE OF OTAG ENTRIES
*	The diskfont library uses other entries from the outline tag
*	(.otag) file.  The following are used both during inquiry of
*	what typefaces exist (AvailFonts) and during creation of an
*	Amiga TextFont (OpenDiskFont):
*	o   OT_IsFixed is used to determine whether these outlines
*	    describe a PROPORTIONAL flagged font.
*	o   OT_StemWeight is used to determine whether these outlines
*	    describe a BOLD style font.
*	o   OT_SlantStyle is used to determine whether these outlines
*	    describe an ITALIC style font.
*	o   OT_HorizStyle is used to determine whether these outlines
*	    describe an EXTENDED style font.
*	The following are used only during OpenDiskFont:
*	o   OT_YSizeFactor is used to convert the Amiga pixel height
*	    specification, which describes the distance from the
*	    lowest decender to the highest ascender, into a point
*	    size specification, which is related (via YSizeFactor)
*	    to a nominal character height.
*	o   OT_SpaceWidth is used as the width of the space character.
*	The following is used only during AvailFonts:
*	o   OT_AvailSizes is used to generate a list of sizes
*	    available for the font.
*
*************************************************************************
*
* Created: kodiak
*
*/

void *vecAlloc(size)
int size;
{
    struct MinNode *node;

    node = MemAlloc(size+sizeof(struct MinNode));
    if (node) {
	DBG4("  vecList @ $%lx: $%lx > $%lx < $%lx\n", &vecList,
		vecList.mlh_Head, vecList.mlh_Tail, vecList.mlh_TailPred);
	AddTail((struct List *) &vecList, (struct Node *) node);
	DBG4("  vecList @ $%lx: $%lx > $%lx < $%lx\n", &vecList,
		vecList.mlh_Head, vecList.mlh_Tail, vecList.mlh_TailPred);
	node++;
    }
    DBG2("vecAlloc(%ld): $%lx\n", size, node);
    return(node);
}

void vecFree(mem)
void *mem;
{
    struct MinNode *node;

    if (mem) {
	node = ((struct MinNode *) mem) - 1;
	DBG2("vecFree($%lx) node @ $%lx\n", mem, node);
	DBG4("  vecList @ $%lx: $%lx > $%lx < $%lx\n", &vecList,
		vecList.mlh_Head, vecList.mlh_Tail, vecList.mlh_TailPred);
	Remove((struct Node *) node);
	DBG4("  vecList @ $%lx: $%lx > $%lx < $%lx\n", &vecList,
		vecList.mlh_Head, vecList.mlh_Tail, vecList.mlh_TailPred);
	MemFree(node);
    }
}

void  CExpunge()
{
    /* no debug statements nor reference to C globals allowed here */
}


ULONG COpenEngine()
{
#if CACHE
    int cacheSize;
#endif
    int status;

    DBG3("SysBase 0x%lx, DOSBase 0x%lx, UtilityBase 0x%lx\n", SysBase,
	    DOSBase, UtilityBase);

    DBG4("vecList @ $%lx: $%lx > $%lx < $%lx\n", &vecList,
	    vecList.mlh_Head, vecList.mlh_Tail, vecList.mlh_TailPred);

    DBG("call CGIFinit...\n");
    if (status = CGIFinit()) {
	DBG1("CGIFinit() error: %ld\n", status);
	return(OTERR_Failure);
    }

    DBG1("fill configBlock @ $%lx\n", &configBlock);
    configBlock.max_char_size = 500;
    configBlock.cc_buf_ptr = ccBufPtr = (UBYTE *) vecAlloc(16000);
    if (!configBlock.cc_buf_ptr) {
	DBG("MemAlloc(16000) compound character buffer failed\n");
	CCloseEngine();
	return(OTERR_Failure);
    }

    configBlock.cc_buf_size = 16000;
    configBlock.num_files = 5;
    strcpy(configBlock.typePath, "Fonts:" BULLET_SUBDIR);
    DBG1("    \"%s\"\ncalling CGIFconfig...\n", configBlock.typePath);
    if (status = CGIFconfig(&configBlock)) {
	DBG1("CGIFconfig() error: %ld\n", status);
	CCloseEngine();
	return(OTERR_Failure);
    }

    DBG("calling CGIFenter...\n");
    if (status = CGIFenter()) {
	DBG1("CGIFenter() error: %ld\n",status);
	CCloseEngine();
	return(OTERR_Failure);
    }
    engineEntered = TRUE;

    return(SUCCESS);
}


void CCloseEngine()
{
    struct Node *n;

    DBG("CCloseEngine()\n");
    if (engineEntered) {
	DBG("CGIFexit...\n");
	CGIFexit();
	DBG("... CGIFexit done.\n");
    }
    DBG4("vecList @ $%lx: $%lx > $%lx < $%lx\n", &vecList, vecList.mlh_Head,
	    vecList.mlh_Tail, vecList.mlh_TailPred);
    while (n = RemHead((struct List *) &vecList)) {
	MemFree(n);
    }
    DBG("CCloseEngine done.\n");
}


ULONG
CSetInfoA(struct TagItem *tagList)
{
    struct TagItem *tag;
    int status;
    char fcFlag;

    fcFlag = FALSE;
    while ((tag = NextTagItem(&tagList)) != TAG_DONE) {
	switch ((UWORD) tag->ti_Tag) {
	  CASE(OT_DeviceDPI):
	    DBG2("set OT_DeviceDPI %ld %ld\n",
		    tag->ti_Data>>16, tag->ti_Data&0xffff);
	    fc.xres = (tag->ti_Data>>16)    * 3937 / 100;
	    fc.yres = (tag->ti_Data&0xffff) * 3937 / 100;
	    fcFlag = TRUE;
	    break;

	  CASE(OT_DotSize):
	    DBG2("set OT_DotSize %ld %ld\n",
		    tag->ti_Data>>16, tag->ti_Data&0xffff);
	    fc.xspot = (tag->ti_Data&0xffff0000)   / 100;
	    fc.yspot = ((tag->ti_Data&0xffff)<<16) / 100;
	    fcFlag = TRUE;
	    break;

	  CASE(OT_PointHeight):
	    DBG2("set OT_PointHeight %ld.%04ld\n",
		    tag->ti_Data>>16, ((tag->ti_Data&0xffff)*10000)>>16);
	    pointHeight = tag->ti_Data;
	    fc.point_size = pointHeight>>13;
	    fc.set_size = DLoToL(DShift(LMul(pointHeight, setFactor),-29));
	    fcFlag = TRUE;
	    break;

	  CASE(OT_SetFactor):
	    DBG2("set OT_SetFactor %ld.%04ld\n",
		    tag->ti_Data>>16, ((tag->ti_Data&0xffff)*10000)>>16);
	    setFactor = tag->ti_Data;
	    fc.set_size = DLoToL(DShift(LMul(pointHeight, setFactor),-29));
	    break;

	  CASE(OT_ShearSin):
	    DBG2("set OT_ShearSin %ld.%04ld\n",
		    tag->ti_Data>>16, ((tag->ti_Data&0xffff)*10000)>>16);
	    fc.shear.x = tag->ti_Data;
	    shearUndefined = TRUE;
	    break;

	  CASE(OT_ShearCos):
	    DBG2("set OT_ShearCos %ld.%04ld\n",
		    tag->ti_Data>>16, ((tag->ti_Data&0xffff)*10000)>>16);
	    fc.shear.y = tag->ti_Data;
	    shearUndefined = FALSE;
	    fcFlag = TRUE;
	    break;

	  CASE(OT_RotateSin):
	    DBG2("set OT_RotateSin %ld.%04ld\n",
		    tag->ti_Data>>16, ((tag->ti_Data&0xffff)*10000)>>16);
	    fc.rotate.y = tag->ti_Data;
	    rotateUndefined = TRUE;
	    break;

	  CASE(OT_RotateCos):
	    DBG2("set OT_RotateCos %ld.%04ld\n",
		    tag->ti_Data>>16, ((tag->ti_Data&0xffff)*10000)>>16);
	    fc.rotate.x = tag->ti_Data;
	    rotateUndefined = FALSE;
	    fcFlag = TRUE;
	    break;

	  CASE(OT_EmboldenX):
	    DBG2("set OT_EmboldenX %ld.%04ld\n",
		    tag->ti_Data>>16, ((tag->ti_Data&0xffff)*10000)>>16);
	    fc.xbold = (tag->ti_Data*8782)>>16;
	    fcFlag = TRUE;
	    break;

	  CASE(OT_EmboldenY):
	    DBG2("set OT_EmboldenY %ld.%04ld\n",
		    tag->ti_Data>>16, ((tag->ti_Data&0xffff)*10000)>>16);
	    fc.ybold = (tag->ti_Data*8782)>>16;
	    fcFlag = TRUE;
	    break;

	  CASE(OT_PointSize):
	    DBG2("set OT_PointSize %ld.%04ld\n",
		    tag->ti_Data>>4, ((tag->ti_Data&0xf)*10000)>>4);
	    pointHeight = tag->ti_Data<<12;
	    fc.point_size = tag->ti_Data>>1;
	    fc.set_size = DLoToL(DShift(LMul(pointHeight, setFactor),-29));
	    fcFlag = TRUE;
	    break;

	  CASE(OT_GlyphCode):
	    DBG1("set OT_GlyphCode $%lx\n", tag->ti_Data);
	    glyphCode = tag->ti_Data;
	    break;

	  CASE(OT_GlyphCode2):
	    DBG1("set OT_GlyphCode2 $%lx\n", tag->ti_Data);
	    glyphCode2 = tag->ti_Data;
	    break;

	  CASE(OT_GlyphWidth):
	    DBG2("set OT_GlyphWidth %ld.%04ld\n", tag->ti_Data>>16,
		    ((tag->ti_Data&0xffff)*10000)>>16);
	    glyphWidth = DLoToL(DShift(LMul(tag->ti_Data, 8782),-16));
	    break;

	  CASE(OT_OTagPath):
	    DBG1("set OT_OTagPath \"%s\"\n", tag->ti_Data);
	    strcpy(configBlock.typePath, tag->ti_Data);
	    *PathPart(configBlock.typePath) = '\0';
	    if (!AddPart(configBlock.typePath, BULLET_SUBDIR, 256)) {
		DBG("AddPart failure\n");
		return(OTERR_BadData);
	    }
	    DBG1("    \"%s\"\ncalling CGIFconfig...\n", configBlock.typePath);
	    if (status = CGIFconfig(&configBlock)) {
		DBG1("CGIFconfig() error: %ld\n", status);
		return(OTERR_BadData);
	    }
	    break;

	  CASE(OT_OTagList):
#ifdef  DEBUG
	    DBG("set OT_OTagList\n");
	    for (status = 0; status < 12; status++) {
		DBG4("    $%08lx $%08lx $%08lx $%08lx\n",
			((ULONG *)(tag->ti_Data))[status*4],
			((ULONG *)(tag->ti_Data))[status*4+1],
			((ULONG *)(tag->ti_Data))[status*4+2],
			((ULONG *)(tag->ti_Data))[status*4+3]);
	    }
#endif
	    fc.font_id = GetTagData(OT_Spec+1, 0,
		    (struct TagItem *) (tag->ti_Data));
	    fc.name = (char *) GetTagData((OT_Spec+2)|OT_Indirect, 0,
		    (struct TagItem *) (tag->ti_Data));
	    fc.fhoff = GetTagData(OT_Spec+3, 0xffffffff,
		    (struct TagItem *) (tag->ti_Data));
	    fc.fhcount = GetTagData(OT_Spec+4, 0,
		    (struct TagItem *) (tag->ti_Data));
	    fc.bucket_num = GetTagData(OT_Spec+5, 0xffff,
		    (struct TagItem *) (tag->ti_Data));
	    DBG5("  id %ld, name \"%s\", "
		    "fhoff $%ld, fhcount $%ld, bucket $%lx\n",
		    fc.font_id, fc.name, fc.fhoff, fc.fhcount,
		    (UWORD) fc.bucket_num);
	    if ((fc.font_id == 0) || (fc.name == 0) ||
		    (fc.fhoff == 0xffffffff) || (fc.fhcount == 0) ||
		    (fc.bucket_num == 0xffff))
		return(OTERR_BadFace);
	    fcFlag = TRUE;
	    break;

	  CASE(OT_SymbolSet):
	    DBG1("set OT_SymbolSet $%04lx\n", tag->ti_Data);
	    fc.ssnum = tag->ti_Data;
	    fcFlag = TRUE;
	    break;

	  CASE(OT_GlyphMap):
	    DBG("set OT_GlyphMap!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_WidthList):
	    DBG("set OT_WidthList!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_TextKernPair):
	    DBG("set OT_TextKernPair!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_DesignKernPair):
	    DBG("set OT_DesignKernPair!\n");
	    return(OTERR_BadTag);

	  default:
	    DBG1("set ??? 0x%08lx\n", tag->ti_Tag);
	    return(OTERR_UnknownTag);
	}
    }
    if (fcFlag) {
	DBG("FontContext\n");
	DBG5("  font_id %ld, point_size %ld %ld/8, set_size %ld %ld/8\n",
		fc.font_id, fc.point_size/8, fc.point_size%8,
		fc.set_size/8, fc.set_size%8);
	DBG4("  shear %ld.%04ld %ld.%04ld\n",
		DBINT(fc.shear.x), DBFRAC(fc.shear.x),
		DBINT(fc.shear.y), DBFRAC(fc.shear.y));
	DBG4("  rotate %ld.%04ld %ld.%04ld\n", DBINT(fc.rotate.x),
		DBFRAC(fc.rotate.x), DBINT(fc.rotate.y), DBFRAC(fc.rotate.y));
	DBG2("  xres %ld, yres %ld\n", fc.xres, fc.yres);
	DBG4("  xspot %ld.%04ld, yspot %ld.%04ld\n", DBINT(fc.xspot),
		DBFRAC(fc.xspot), DBINT(fc.yspot), DBFRAC(fc.yspot));
	DBG4("  xbold %ld, ybold %ld, ssnum $%04lx, format $%04lx\n",
		fc.xbold, fc.ybold, fc.ssnum, fc.format);
	if (shearUndefined) {
	    DBG("shear undefined\n");
	    return(OTERR_NoShear);
	}
	if (rotateUndefined) {
	    DBG("rotate undefined\n");
	    return(OTERR_NoRotate);
	}
	if (status = CGIFfont(&fc)) {
	    DBG1("CGIFfont() error: %ld\n",status);
	    return(OTERR_BadData);
	}
    }
    return(OTERR_Success);
}


ULONG
CObtainInfoA(struct TagItem *tagList)
{
    struct TagItem *tag;
#if CACHE
    IFBITMAP *bm;
#endif
    IFBITMAP *result, *bold1, *bold2;
    struct MinList *widthList;
    struct GlyphWidthEntry *widthEntry;
    FIXED width;
    KERN_PAIR kernPair;
    UWORD size;
    int status, i;

    while ((tag = NextTagItem(&tagList)) != TAG_DONE) {
	switch ((UWORD) tag->ti_Tag) {
	  CASE(OT_DeviceDPI):
	    DBG("obtain OT_DeviceDPI!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_DotSize):
	    DBG("obtain OT_DotSize!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_PointHeight):
	    DBG("obtain OT_PointHeight!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_SetFactor):
	    DBG("obtain OT_SetFactor!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_ShearSin):
	    DBG("obtain OT_ShearSin!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_ShearCos):
	    DBG("obtain OT_ShearCos!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_RotateSin):
	    DBG("obtain OT_RotateSin!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_RotateCos):
	    DBG("obtain OT_RotateCos!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_EmboldenX):
	    DBG("obtain OT_EmboldenX!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_EmboldenY):
	    DBG("obtain OT_EmboldenY!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_PointSize):
	    DBG("obtain OT_PointSize!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_GlyphCode):
	    DBG("obtain OT_GlyphCode!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_GlyphCode2):
	    DBG("obtain OT_GlyphCode2!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_GlyphWidth):
	    DBG("obtain OT_GlyphWidth!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_OTagPath):
	    DBG("obtain OT_OTagPath!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_OTagList):
	    DBG("obtain OT_OTagList!\n");
	    return(OTERR_BadTag);

	  CASE(OT_GlyphMap):
	    DBG("obtain OT_GlyphMap\n");
	    DBG1("CGIFchar($%lx, ...)\n", glyphCode);
#if CACHE
	    if (status = CGIFchar(glyphCode, &bm, glyphWidth)) {
		DBG1("CGIFchar failed %ld\n", status);
		return(OTERR_Failure);
	    }
	    size = bm->width*bm->depth+(sizeof(*bm)-sizeof(bm->bm));
#else
	    if (status = CGIFchar_size(glyphCode, &size, glyphWidth)) {
		DBG1("CGIFchar_size failed %ld\n", status);
		return(OTERR_Failure);
	    }
#endif

	    if (!(result = vecAlloc(size))) {
		DBG1("MemAlloc %ld for IFBITMAP failed\n", size);
		return(OTERR_Failure);
	    }
	    if (fc.xbold || fc.ybold) {
		bold1 = vecAlloc(size);
		bold2 = vecAlloc(size);
		if ((!bold1) || (!bold2)) {
		    vecFree(result);
		    vecFree(bold1);
		    vecFree(bold2);
		    DBG1("MemAlloc %ld for bold IFBITMAP failed\n", size);
		    return(OTERR_Failure);
		}
	    }
	    else {
		bold1 = bold2 = 0;
	    }

#if CACHE
	    memcpy(result, bm, size);
#else
	    if (status = MAKbitMap(result, bold1, bold2)) {
		DBG1("MAKbitMap failed %ld\n", status);
		vecFree(result);
		vecFree(bold1);
		vecFree(bold2);
		return(OTERR_Failure);
	    }
#endif
	    DBG1("MAKbitMap result: $%lx\n", result);
	    result->xorigin <<= 12;
	    result->yorigin <<= 12;
	    result->escapement = (result->escapement << 16) / 8782;
	    result->bmp = result->bm;
	    *((void **) tag->ti_Data) = (void *) (&result->width);
	    break;

	  CASE(OT_WidthList):
	    DBG("obtain OT_WidthList\n");
	    if (glyphCode > glyphCode2)
		return(OTERR_BadGlyph);
	    widthList = vecAlloc(sizeof(struct MinList));
	    if (!widthList)
		return(OTERR_NoMemory);
	    NewList((struct List *) widthList);
	    status = CGIFwidthsetup();
	    if (status) {
		DBG1("CGIFwidthsetup failed %ld\n", status);
		vecFree(widthList);
		return(OTERR_Failure);
	    }
	    for (i = glyphCode; i <= glyphCode2; i++) {
		width = MAKchar_width(i);
		if (width != 0xffffffff) {
		    widthEntry = vecAlloc(sizeof(struct GlyphWidthEntry));
		    if (!widthEntry) {
			while (widthEntry = (struct GlyphWidthEntry *)
				RemHead((struct List *) widthList))
			    vecFree(widthEntry);
			vecFree(widthList);
		    }
		    widthEntry->gwe_Code = i;
		    /* convert design units to em fractions */
		    widthEntry->gwe_Width = (width << 16) / 8782;
		    AddTail((struct List *) widthList,
			    (struct Node *) widthEntry);
		}
	    }
	    *((struct MinList **) tag->ti_Data) = widthList;
	    break;

	  CASE(OT_TextKernPair):
	    DBG("obtain OT_TextKernPair\n");
	    kernPair.chId0 = glyphCode;
	    kernPair.chId1 = glyphCode2;
	    status = CGIFkern(TEXT_KERN, 1, &kernPair);
	    if (status) {
		DBG1("CGIFkern failed %ld\n", status);
		return(OTERR_Failure);
	    }
	    /* convert design units to em fractions */
	    *((ULONG *) tag->ti_Data) = (kernPair.adj << 16) / 8782;
	    break;

	  CASE(OT_DesignKernPair):
	    DBG("obtain OT_DesignKernPair\n");
	    kernPair.chId0 = glyphCode;
	    kernPair.chId1 = glyphCode2;
	    status = CGIFkern(DESIGN_KERN, 1, &kernPair);
	    if (status) {
		DBG1("CGIFkern failed %ld\n", status);
		return(OTERR_Failure);
	    }
	    /* convert design units to em fractions */
	    *((ULONG *) tag->ti_Data) = (kernPair.adj << 16) / 8782;
	    break;

	  default:
	    return(OTERR_UnknownTag);
	}
    }
    return(OTERR_Success);
}

ULONG
CReleaseInfoA(struct TagItem *tagList)
{
    struct TagItem *tag;
    struct Node *node;
    IFBITMAP *ibm;

    while ((tag = NextTagItem(&tagList)) != TAG_DONE) {
	switch ((UWORD) tag->ti_Tag) {
	  CASE(OT_DeviceDPI):
	    DBG("release OT_DeviceDPI!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_DotSize):
	    DBG("release OT_DotSize!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_PointHeight):
	    DBG("release OT_PointHeight!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_SetFactor):
	    DBG("release OT_SetFactor!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_ShearSin):
	    DBG("release OT_ShearSin!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_ShearCos):
	    DBG("release OT_ShearCos!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_RotateSin):
	    DBG("release OT_RotateSin!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_RotateCos):
	    DBG("release OT_RotateCos!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_EmboldenX):
	    DBG("release OT_EmboldenX!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_EmboldenY):
	    DBG("release OT_EmboldenY!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_PointSize):
	    DBG("release OT_PointSize!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_GlyphCode):
	    DBG("release OT_GlyphCode!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_GlyphCode2):
	    DBG("release OT_GlyphCode2!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_GlyphWidth):
	    DBG("release OT_GlyphWidth!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_OTagPath):
	    DBG("release OT_OTagPath!\n");
#ifdef DEBUG
	    return(OTERR_BadTag);
#endif
	  CASE(OT_OTagList):
	    DBG("release OT_OTagList!\n");
	    return(OTERR_BadTag);

	  CASE(OT_GlyphMap):
	    DBG1("release OT_GlyphMap $%lx\n", tag->ti_Data);
	    if (tag->ti_Data) {
		ibm = (IFBITMAP *) (tag->ti_Data-
			((ULONG) &(((IFBITMAP *) 0)->width)));
		vecFree(ibm);
	    }
	    break;

	  CASE(OT_WidthList):
	    DBG("release OT_WidthList\n");
	    if (tag->ti_Data) {
		while (node = RemHead((struct List *) tag->ti_Data))
		    vecFree(node);
		vecFree(tag->ti_Data);
	    }
	    break;

	  CASE(OT_TextKernPair):
	    DBG("release OT_TextKernPair\n");
	    break;
	  CASE(OT_DesignKernPair):
	    DBG("release OT_DesignKernPair\n");
	    break;

	  default:
	    return(OTERR_UnknownTag);
	}
    }
    return(OTERR_Success);
}

ULONG CGetGlyphMap(ULONG glyphCode, struct GlyphMap **glyphMap)
{
#if 1
    return(OTERR_Failure);
#else
    struct TagItem tags[2];
    ULONG status;

    tags[0].ti_Tag = OT_GlyphMap;
    tags[1].ti_Tag = TAG_DONE;
    if (prevGlyph) {
	tags[0].ti_Data = (ULONG) prevGlyph;
	status = CReleaseInfoA(tags);
	if (status)
	    return(status);
    }
    prevGlyph = 0;
    tags[0].ti_Tag = OT_GlyphCode;
    tags[0].ti_Data = glyphCode;
    status = CSetInfoA(tags);
    if (status)
	return(status);

    tags[0].ti_Tag = OT_GlyphMap;
    tags[0].ti_Data = (ULONG) &prevGlyph;
    status = CSetInfoA(tags);
    *glyphMap = prevGlyph;
    return(status);
#endif
}


/****** bullet.library/CloseEngine *****************************************
*
*    NAME
*	CloseEngine -- Release an engine handle
*
*    SYNOPSIS
*	CloseEngine(engineHandle)
*	            A0
*
*	void CloseEngine(struct GlyphEngine *);
*
*    FUNCTION
*	This function releases the engine handle acquired with
*	OpenEngine.  It first releases any data acquired with
*	ObtainInfoA associated with the engineHandle that has not yet
*	been released.
*
*    INPUTS
*	engineHandle -- the handle acquired via OpenEngine.  If zero,
*		no operation is performed.
*
*    RESULT
*	This function has no result.  The only error that can occur is
*	when the when an invalid engineHandle is supplied: the
*	application is assumed not to do that.
*
*    EXAMPLE
*	EndGame(code, arg1, arg2, arg3, arg3)
*	{
*	    ...
*	    CloseEngine(EngineHandle);
*	    ...
*	}
*
*    SEE ALSO
*	OpenEngine()
*
****************************************************************************
*
*
*/

/****i* bullet.library/GetGlyphMap *****************************************
*
*    NAME
*	GetGlyphMap -- Acquire a glyph bitmap.
*
*    SYNOPSIS
*	error = GetGlyphMap(engineHandle, glyphCode, glyph)
*	                    A0            D0         A1
*
*	ULONG GetGlyphMap(struct GlyphEngine *, int, struct Glyph **);
*
*    FUNCTION
*	This function provides a shortcut way to access a glyph
*	bitmap.  It does the equivalant of the following function
*	sequence:
*	1.  If a previous GetGlyphMap has been performed with this
*	    engine handle: ReleaseInfoA the previous glyph.
*	2.  SetInfoA the OT_GlyphCode to glyphCode.
*	2.  ObtainInfoA the OT_Glyph into this glyph.
*
*    INPUTS
*	engineHandle -- the handle acquired via OpenEngine.
*	glyphCode -- the glyph code whose image is to be acquired.
*	glyph -- a pointer to store the pointer to the Glyph structure
*		that contains the glyph metrics and bitmap.
*
*    RESULT
*	This function returns a zero success indication, or a non-zero
*	error code.
*
*    EXAMPLE
*	    struct GlyphMap *glyph;
*
*	    for (code = 32; code <= 255; code++) {
*		if (code == 128)
*		    code = 160;
*		if (!GetGlyphMap(EngineHandle, 0x32323200 | code, &glyph)) {
*		    ...
*		}
*	    }
*
*    SEE ALSO
*	ObtainInfoA(), ReleaseInfoA(), SetInfoA(), diskfont/oterrors.h
*
****************************************************************************
*
*
*/

/****** bullet.library/ObtainInfoA *****************************************
*
*    NAME
*	ObtainInfoA -- Inquire tagged font and/or glyph metrics
*	ObtainInfo -- varargs form of ObtainInfoA
*
*    SYNOPSIS
*	error = ObtainInfoA(engineHandle, tagList)
*	                    A0            A1
*
*	ULONG ObtainInfoA(struct GlyphEngine *, struct TagItem *);
*
*	error = ObtainInfo(engineHandle, firstTag, ...)
*
*	ULONG ObtainInfo(struct GlyphEngine *, Tag, ...);
*
*    FUNCTION
*	This function accepts a tagList whose tag field elements are
*	valid for inquiry, and whose associated data fields are
*	pointers to the destination in which to place the requested
*	data.
*
*	Tag items that refer to data indirectly (OT_Indirect is set)
*	return pointers that may be allocated or cached by the
*	library.  This data must be treated as read-only data.  When
*	the application is done with the data acquired via ObtainInfoA,
*	it must perform a ReleaseInfoA to allow the library to release
*	the data.
*
*    INPUTS
*	engineHandle -- the handle acquired via OpenEngine.
*	tagList -- a tagList containing OT_ tags valid for inquiry
*		paired with the destination pointers for the inquiry
*		results.  All destinations are longwords, whether they
*		are pointers or values, and regardless of whether the
*		value could fit in a smaller variable.
*
*    RESULT
*	This function returns a zero success indication, or a non-zero
*	error code.
*
*    EXAMPLE
*	    ULONG pointSize;
*	    struct GlyphMap *glyph;
*	    ...
*	    if (!ObtainInfo(EngineHandle, OT_Glyph, &glyph, TAG_DONE)) {
*		...
*		ReleaseInfo(EngineHandle, OT_Glyph, glyph, TAG_DONE);
*	    }
*
*    SEE ALSO
*	ReleaseInfoA(), diskfont/diskfonttag.h, diskfont/oterrors.h
*
****************************************************************************
*
*
*/

/****** bullet.library/OpenEngine ******************************************
*
*    NAME
*	OpenEngine -- Acquire engine handle
*
*    SYNOPSIS
*	engineHandle = OpenEngine()
*
*	struct GlyphEngine *OpenEngine(void)
*
*    FUNCTION
*	This function establishes a context for access to the bullet
*	library.  This context remains valid until it is closed via
*	CloseEngine.  Each specific context isolates the specification
*	of the various font attributes from other contexts concurrently
*	accessing the bullet library.  A context can be shared among
*	different tasks.
*
*    RESULT
*	This function returns an engineHandle, or NULL if for some
*	reason no engineHandle can be created.
*
*    EXAMPLE
*	    BulletBase = OpenLibrary("bullet.library", 0);
*	    if (!BulletBase)
*		EndGame(ERROR_LibOpen, "bullet.library", 0);
*	    EngineHandle = OpenEngine();
*	    if (!EngineHandle)
*		EndGame(ERROR_InternalCall, "OpenEngine");
*
*    SEE ALSO
*	CloseEngine()
*
****************************************************************************
*
*
*/

/****i* bullet.library/ReleaseInfoA ****************************************
*
*    NAME
*	ReleaseInfoA -- Release data obtained with ObtainInfoA
*	ReleaseInfo -- varargs form of ReleaseInfoA
*
*    SYNOPSIS
*	error = ReleaseInfoA(engineHandle, tagList)
*	                     A0            A1
*
*	ULONG ReleaseInfoA(struct GlyphEngine *, struct TagItem *);
*
*	error = ReleaseInfo(engineHandle, firstTag, ...)
*
*	ULONG ReleaseInfo(struct GlyphEngine *, Tag, ...);
*
*    FUNCTION
*	This function releases the data obtained with ObtainInfoA.
*	Data associated with tags that are not indirect, i.e. for which
*	OT_Indirect is not set, need not be released, but it is not an
*	error to do so.  Released data may be immediately freed or may
*	become a candidate to be expunged from memory when the system
*	reaches a low memory condition, depending on the library's
*	internal implementation.
*
*	Each ReleaseInfoA tag item must be associated with a prior
*	ObtainInfoA.  It is inappropriate to explicitly ReleaseInfoA a
*	glyph that has only been implicitly obtained via GetGlyphMap --
*	the glyph obtained via GetGlyphMap must only be released via a
*	subsequent GetGlyphMap or via CloseEngine.
*
*    INPUTS
*	engineHandle -- the handle acquired via OpenEngine.
*	tagList -- a tagList containing OT_ tags valid for inquiry
*		paired with the data previously acquired for them with
*		ObtainInfoA.  Null pointers quietly accepted and
*		ignored for indirect data.
*
*    RESULT
*	This function has no result.  The only error that can occur is
*	when the Obtain and Release pairs are mismatched: the
*	application is assumed not to do that.
*
*    EXAMPLE
*	    ULONG pointSize;
*	    struct GlyphMap *glyph;
*	    ...
*	    error = ObtainInfo(EngineHandle, OT_Glyph, &glyph, TAG_DONE);
*	    ...
*	    ReleaseInfo(EngineHandle, OT_Glyph, glyph, TAG_DONE);
*
*    SEE ALSO
*	ReleaseInfoA(), diskfont/diskfonttag.h, diskfont/oterrors.h
*
****************************************************************************
*
*
*/

/****** bullet.library/ReleaseInfoA ****************************************
*
*    NAME
*	ReleaseInfoA -- Release data obtained with ObtainInfoA
*	ReleaseInfo -- varargs form of ReleaseInfoA
*
*    SYNOPSIS
*	error = ReleaseInfoA(engineHandle, tagList)
*	                     A0            A1
*
*	ULONG ReleaseInfoA(struct GlyphEngine *, struct TagItem *);
*
*	error = ReleaseInfo(engineHandle, firstTag, ...)
*
*	ULONG ReleaseInfo(struct GlyphEngine *, Tag, ...);
*
*    FUNCTION
*	This function releases the data obtained with ObtainInfoA.
*	Data associated with tags that are not indirect, i.e. for which
*	OT_Indirect is not set, need not be released, but it is not an
*	error to do so.  Released data may be immediately freed or may
*	become a candidate to be expunged from memory when the system
*	reaches a low memory condition, depending on the library's
*	internal implementation.
*
*	Each ReleaseInfoA tag item must be associated with a prior
*	ObtainInfoA.
*
*    INPUTS
*	engineHandle -- the handle acquired via OpenEngine.
*	tagList -- a tagList containing OT_ tags valid for inquiry
*		paired with the data previously acquired for them with
*		ObtainInfoA.  Null pointers quietly accepted and
*		ignored for indirect data.
*
*    RESULT
*	This function has no result.  The only error that can occur is
*	when the Obtain and Release pairs are mismatched: the
*	application is assumed not to do that.
*
*    EXAMPLE
*	    ULONG pointSize;
*	    struct GlyphMap *glyph;
*	    ...
*	    error = ObtainInfo(EngineHandle, OT_Glyph, &glyph, TAG_DONE);
*	    ...
*	    ReleaseInfo(EngineHandle, OT_Glyph, glyph, TAG_DONE);
*
*    SEE ALSO
*	ReleaseInfoA(), diskfont/diskfonttag.h, diskfont/oterrors.h
*
****************************************************************************
*
*
*/

/****** bullet.library/SetInfoA ********************************************
*
*    NAME
*	SetInfoA -- Set font and/or glyph metrics
*	SetInfo -- varargs form of SetInfoA
*
*    SYNOPSIS
*	error = SetInfoA(engineHandle, tagList)
*	                 A0            A1
*
*	ULONG SetInfoA(struct GlyphEngine *, struct TagItem *);
*
*	error = SetInfo(engineHandle, firstTag, ...)
*
*	ULONG SetInfo(struct GlyphEngine *, Tag, ...);
*
*    FUNCTION
*	This function accepts a tagList whose tag field elements are
*	valid for specification, and whose associated data fields are
*	used to supply the specified data.
*
*	Data that is supplied via an indirect pointer (OT_Indirect) to
*	an array or structure is copied from that array or structure
*	into the internal memory of the library.  Changes to the data
*	after this call do not affect the engine.
*
*    INPUTS
*	engineHandle -- the handle acquired via OpenEngine.
*	tagList -- a tagList containing OT_ tags valid for
*		specification paired with the specification data.
*
*    RESULT
*	This function returns a zero success indication, or a non-zero
*	error code.
*
*    EXAMPLE
*	    if (!(error = SetInfo(EngineHandle, OT_PointHeight, fpoints,
*		    OT_GlyphCode, GC_daggerdbl, TAG_DONE)) {
*		error = ObtainInfo(EngineHandle, OT_Glyph, &glyph);
*		...
*		ReleaseInfo(EngineHandle, OT_Glyph, glyph);
*	    }
*
*    SEE ALSO
*	diskfont/diskfonttag.h, diskfont/oterrors.h,
*
****************************************************************************
*
*
*/
