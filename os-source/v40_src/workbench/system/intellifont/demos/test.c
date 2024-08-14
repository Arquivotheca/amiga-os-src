#include <exec/types.h>
#include <dos/rdargs.h>
#include "diskfonttag.h"
#include "glyph.h"

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>
#include "bullet_protos.h"
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/utility_pragmas.h>
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

#define  TEMPLATE1	"NAME,CODE/N,CODE2/N,DPIX/N,DPIY/N,DOTPX/N,DOTPY/N,"
#define  TEMPLATE2	"POINTS/N,SETP/N,SHEAR/N,ROTATE/N,BOLDPX/N,BOLDPY/N,"
#define  TEMPLATE3	"AWIDTHP/N,GLYPH/S,WIDTHS/S,TKERN/S,DKERN/S"
#define  TEMPLATE	TEMPLATE1 TEMPLATE2 TEMPLATE3

#define  O_NAME		0
#define  O_CODE		1
#define  O_CODE2	2
#define  O_DPIX		3
#define  O_DPIY		4
#define  O_DOTPX	5
#define  O_DOTPY	6
#define  O_POINTS	7
#define  O_SETP		8
#define  O_SHEAR	9
#define  O_ROTATE	10
#define  O_BOLDPX	11
#define  O_BOLDPY	12
#define  O_AWIDTHP	13
#define  O_GLYPH	14
#define  O_WIDTHS	15
#define  O_TKERN	16
#define  O_DKERN	17
#define  O_COUNT	18


#define  U_CODE		0
#define  U_CODE2	1
#define  U_DPI		2
#define  U_DOTP		3
#define  U_POINTS	4
#define  U_SETP		5
#define  U_SHEARSIN	6
#define  U_SHEARCOS	7
#define  U_ROTATESIN	8
#define  U_ROTATECOS	9
#define  U_BOLDX	10
#define  U_BOLDY	11
#define  U_AWIDTH	12
#define  U_COUNT	13


#define  MAXFILESIZE	4000

struct Library *UtilityBase = 0;
struct RDArgs *RDArgs = 0;
BPTR OTFile = 0;
struct Library *BulletBase = 0;
struct GlyphEngine *GlyphEngine = 0;
struct TagItem OTags[MAXFILESIZE/sizeof(struct TagItem)];
struct TagItem UTags[U_COUNT+1] = {
    { OT_GlyphCode, 0x0042},
    { OT_GlyphCode2, 0x0000},
    { OT_DeviceDPI, 0x00480048 },
    { OT_DotSize, 0x00640064 },
    { OT_PointHeight, 0x000a0000 },
    { OT_SetFactor, 0x00010000 },
    { OT_ShearSin, 0x00000000 },
    { OT_ShearCos, 0x00010000 },
    { OT_RotateSin, 0x00000000 },
    { OT_RotateCos, 0x00010000 },
    { OT_EmboldenX, 0 },
    { OT_EmboldenY, 0 },
    { OT_GlyphWidth, 0 },
    { TAG_DONE, 0 }
};

void
endGame(format, arg1, arg2, arg3, arg4)
char *format, *arg1, *arg2, *arg3, *arg4;
{
    if (format)
	printf(format, arg1, arg2, arg3, arg4);
    if (GlyphEngine)
	CloseEngine(GlyphEngine);
    if (BulletBase)
	CloseLibrary(BulletBase);
    if (OTFile)
	Close(OTFile);
    if (RDArgs)
	FreeArgs(RDArgs);
    if (UtilityBase)
	CloseLibrary(UtilityBase);
    if (format)
	exit(5);
    exit(0);
}

ULONG
setInfo(ge, tag)
struct GlyphEngine *ge;
LONG tag;
{
    return(SetInfoA(ge, (struct TagItem *) &tag));
}

ULONG
obtainInfo(ge, tag)
struct GlyphEngine *ge;
LONG tag;
{
    return(ObtainInfoA(ge, (struct TagItem *) &tag));
}

ULONG
releaseInfo(ge, tag)
struct GlyphEngine *ge;
LONG tag;
{
    return(ReleaseInfoA(ge, (struct TagItem *) &tag));
}

void
main()
{
    struct GlyphMap *glyph;
    struct MinList *widthList;
    struct GlyphWidthEntry *widthEntry;
    double angle;
    ULONG error, *options[O_COUNT], result, l;
    short i, x, y;
    char pathStore[256], *filePath, *s;

    UtilityBase = OpenLibrary("utility.library", 0);
    if (!UtilityBase)
	endGame("ERROR: cannot open \"utility.library\"\n");

    memset(options, 0, sizeof(options));
    RDArgs = ReadArgs(TEMPLATE, (LONG *) options, 0);
    if (!RDArgs)
	endGame("ERROR: invalid arguments\n");

    /* generate .otag file path */
    strcpy(pathStore, "FONTS:");
    filePath = pathStore+6;
    if (options[0])
	strcpy(filePath, (char *) options[O_NAME]);
    else
	strcpy(filePath, "CGTimes.font");
    s = strrchr(filePath, '.');
    if ((s == 0) || (strcmp(s, ".font")))
	endGame("ERROR: NAME not .font name\n");

    strcpy(s, OTSUFFIX);

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
    for (i = 0; i < OTags[0].ti_Data/sizeof(struct TagItem); i++) {
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

    error = setInfo(GlyphEngine, OT_OTagPath, filePath, TAG_DONE);
    if (error)
	endGame("setInfo(OTagPath) error %ld\n", error);

    error = setInfo(GlyphEngine, OT_OTagList, OTags, TAG_DONE);
    if (error)
	endGame("setInfo(OTagList) error %ld\n", error);


    /* modify environment from .otag */
    if (result = GetTagData(OT_DeviceDPI, 0, OTags))
	UTags[U_DPI].ti_Data = result;
    if (result = GetTagData(OT_DotSize, 0, OTags))
	UTags[U_DOTP].ti_Data = result;

    /* modify environment from parameters */
    if (options[O_CODE])
	UTags[U_CODE].ti_Data = *options[O_CODE];
    if (options[O_CODE2])
	UTags[U_CODE2].ti_Data = *options[O_CODE2];
    if (options[O_DPIX] && options[O_DPIY])
	UTags[U_DPI].ti_Data = (*options[O_DPIX]<<16) | *options[O_DPIY];
    if (options[O_DOTPX] && options[O_DOTPY])
	UTags[U_DOTP].ti_Data = (*options[O_DOTPX]<<16) | *options[O_DOTPY];
    if (options[O_POINTS])
	UTags[U_POINTS].ti_Data = *options[O_POINTS]<<16;
    if (options[O_SETP])
	UTags[U_SETP].ti_Data = (*options[O_SETP]<<16)/100;
    if (options[O_SHEAR]) {
	angle = *options[O_SHEAR];
	angle = angle * PI / 180.0;
	if (sin(angle) < 0) {
	    UTags[U_SHEARSIN].ti_Data = ((-sin(angle)) * 65536.0);
	    UTags[U_SHEARSIN].ti_Data = -UTags[U_SHEARSIN].ti_Data;
	}
	else
	    UTags[U_SHEARSIN].ti_Data = (sin(angle) * 65536.0);
	if (cos(angle) < 0) {
	    UTags[U_SHEARCOS].ti_Data = ((-cos(angle)) * 65536.0);
	    UTags[U_SHEARCOS].ti_Data = -UTags[U_SHEARCOS].ti_Data;
	}
	else
	    UTags[U_SHEARCOS].ti_Data = (cos(angle) * 65536.0);
	printf("$%08lx $%08lx\n",
		UTags[U_SHEARSIN].ti_Data, UTags[U_SHEARCOS].ti_Data);
    }
    if (options[O_ROTATE]) {
	angle = *options[O_ROTATE];
	printf("rotate %ld %f degrees, ", angle);
	angle = angle * PI / 180.0;
	printf("%f radians, ", angle);
	if (sin(angle) < 0) {
	    UTags[U_ROTATESIN].ti_Data = ((-sin(angle)) * 65536.0);
	    UTags[U_ROTATESIN].ti_Data = -UTags[U_ROTATESIN].ti_Data;
	}
	else
	    UTags[U_ROTATESIN].ti_Data = (sin(angle) * 65536.0);
	if (cos(angle) < 0) {
	    UTags[U_ROTATECOS].ti_Data = ((-cos(angle)) * 65536.0);
	    UTags[U_ROTATECOS].ti_Data = -UTags[U_ROTATECOS].ti_Data;
	}
	else
	    UTags[U_ROTATECOS].ti_Data = (cos(angle) * 65536.0);
	printf("$%08lx $%08lx\n",
		UTags[U_ROTATESIN].ti_Data, UTags[U_ROTATECOS].ti_Data);
    }
    if (options[O_BOLDPX])
	UTags[U_BOLDX].ti_Data = (*options[O_BOLDPX]<<16)/100;
    if (options[O_BOLDPY])
	UTags[U_BOLDY].ti_Data = (*options[O_BOLDPY]<<16)/100;
    if (options[O_AWIDTHP])
	UTags[U_AWIDTH].ti_Data = (*options[O_AWIDTHP]<<16)/100;

    SetInfoA(GlyphEngine, UTags);

    if (options[O_GLYPH]) {
	error = obtainInfo(GlyphEngine, OT_GlyphMap, &glyph, TAG_DONE);
	if (error)
	    endGame("obtainInfo(Glyph) error %ld\n", error);
	printf("glyph $%lx, width %ld, height %ld, bitmap $%lx\n",
		glyph, glyph->glm_BMModulo, glyph->glm_BMRows,
		glyph->glm_BitMap);
	printf("    Black- Left %ld, Top %ld, Width %ld, Height %ld\n",
		glyph->glm_BlackLeft, glyph->glm_BlackTop,
		glyph->glm_BlackWidth, glyph->glm_BlackHeight);
	printf("    xOrigin $%08lx, yOrigin $%08lx\n",
		glyph->glm_XOrigin, glyph->glm_YOrigin);
	printf("    x0 y0 %ld %ld,    x1 y1 %ld %ld\n",
		glyph->glm_X0, glyph->glm_Y0, glyph->glm_X1, glyph->glm_Y1);
	printf("    width %ld.%04ld\n", glyph->glm_Width>>16,
		((glyph->glm_Width&0xffff)*10000)>>16);
	for (y = 0; y < glyph->glm_BMRows; y++) {
	    for (x = 0; x < glyph->glm_BMModulo; x++) {
		for (i = 0x80; i; i>>=1) {
		    if (((char *)(glyph->glm_BitMap))[(y*glyph->glm_BMModulo)+x]
			    & i)
			printf("*");
		    else
			printf("-");
		}
	    }
	    printf("\n");
	}
	releaseInfo(GlyphEngine, OT_GlyphMap, glyph, TAG_DONE);
    }
    if (options[O_WIDTHS]) {
	error = obtainInfo(GlyphEngine, OT_WidthList, &widthList, TAG_DONE);
	if (error)
	    endGame("obtainInfo(WidthList) error %ld\n", error);
	printf("widthList $%lx...\n", widthList);
	for (widthEntry = (struct GlyphWidthEntry *) widthList->mlh_Head;
		widthEntry->gwe_Node.mln_Succ;
		widthEntry = (struct GlyphWidthEntry *)
		widthEntry->gwe_Node.mln_Succ) {
	    printf("$%04lx: %ld.%04ld\n",
		    widthEntry->gwe_Code,
		    widthEntry->gwe_Width>>16,
		    ((widthEntry->gwe_Width&0xffff)*10000)>>16);
	}
	releaseInfo(GlyphEngine, OT_WidthList, widthList, TAG_DONE);
    }
    if (options[O_TKERN]) {
	error = obtainInfo(GlyphEngine, OT_TextKernPair, &l, TAG_DONE);
	if (error)
	    endGame("obtainInfo(TextKernPair) error %ld\n", error);
	printf("Text Kern $%08lx:$%08lx %ld.%04ld\n", UTags[U_CODE].ti_Data,
		UTags[U_CODE2].ti_Data, l>>16, ((l&0xffff)*10000)>>16);
    }
    if (options[O_DKERN]) {
	error = obtainInfo(GlyphEngine, OT_DesignKernPair, &l, TAG_DONE);
	if (error)
	    endGame("obtainInfo(DesignKernPair) error %ld\n", error);
	printf("Design Kern $%08lx:$%08lx %ld.%04ld\n", UTags[U_CODE].ti_Data,
		UTags[U_CODE2].ti_Data, l>>16, ((l&0xffff)*10000)>>16);
    }

    endGame(0);
}
