#define  DEBUG
#ifdef   DEBUG
#define  D(a)	kprintf a
#else
#define  D(a)
#endif
#include <exec/types.h>
#include <dos/rdargs.h>
#include <graphics/rastport.h>
#include <intuition/intuition.h>
#include "diskfonttag.h"
#include "glyph.h"

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>
#include "bullet_protos.h"
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
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

#define  TEMPLATE	"NAME,DPIX/N,DPIY/N,DOTPX/N,DOTPY/N"

#define  O_NAME		0
#define  O_DPIX		1
#define  O_DPIY		2
#define  O_DOTPX	3
#define  O_DOTPY	4
#define  O_COUNT	5

#define  U_DPI		0
#define  U_DOTP		1
#define  U_COUNT	2

#define  MAXFILESIZE	4000

struct Library *GfxBase = 0;
struct Library *IntuitionBase = 0;
struct Library *UtilityBase = 0;
struct RDArgs *RDArgs = 0;
BPTR OTFile = 0;
struct Library *BulletBase = 0;
struct GlyphEngine *GlyphEngine = 0;
struct TagItem OTags[MAXFILESIZE/sizeof(struct TagItem)];
struct TagItem UTags[U_COUNT+1] = {
    { OT_DeviceDPI, 0x00480048 },
    { OT_DotSize, 0x00640064 },
    { TAG_DONE, 0 }
};

#define  X2	640
#define  Y2	400
#define  CX	(X2/2)
#define  CY	(Y2/2)
struct Window *Window = 0;
struct IntuiMessage *IM;
PLANEPTR Template = 0;

void
endGame(format, arg1, arg2, arg3, arg4)
char *format, *arg1, *arg2, *arg3, *arg4;
{
    if (format) {
	D((format, arg1, arg2, arg3, arg4));
	printf(format, arg1, arg2, arg3, arg4);
    }
    if (Template)
	FreeRaster(Template, X2, Y2);
    if (Window)
	CloseWindow(Window);
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
    if (IntuitionBase)
	CloseLibrary(IntuitionBase);
    if (GfxBase)
	CloseLibrary(GfxBase);
    if (format)
	exit(5);
    exit(0);
}


struct Window *
myOpenWindowTags(tag)
LONG tag;
{
    return(OpenWindowTagList(0, (struct TagItem *) &tag));
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
    ULONG error, *options[O_COUNT], result;
    int i, x, y, dx, dy;
    UWORD chId;
    char pathStore[256], *filePath, *s;

    D(("Test\nOpenLibrarys..."));
    GfxBase = OpenLibrary("graphics.library", 0);
    if (!GfxBase)
	endGame("ERROR: cannot open \"graphics.library\"\n");
    IntuitionBase = OpenLibrary("intuition.library", 0);
    if (!IntuitionBase)
	endGame("ERROR: cannot open \"intuition.library\"\n");
    UtilityBase = OpenLibrary("utility.library", 0);
    if (!UtilityBase)
	endGame("ERROR: cannot open \"utility.library\"\n");

    D(("ReadArgs..."));
    memset(options, 0, sizeof(options));
    RDArgs = ReadArgs(TEMPLATE, (LONG *) options, 0);
    if (!RDArgs)
	endGame("ERROR: invalid arguments\n");

    D((" .\n"));
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
    if (options[O_DPIX] && options[O_DPIY])
	UTags[U_DPI].ti_Data = (*options[O_DPIX]<<16) | *options[O_DPIY];
    if (options[O_DOTPX] && options[O_DOTPY])
	UTags[U_DOTP].ti_Data = (*options[O_DOTPX]<<16) | *options[O_DOTPY];

    if (error = SetInfoA(GlyphEngine, UTags))
	endGame("setInfo(UTags) error %ld\n", error);
    if (error = setInfo(GlyphEngine, OT_PointHeight, 0x00140000, TAG_DONE))
	endGame("setInfo(20 points) failed %ld\n", error);

    Window = myOpenWindowTags(
	    WA_Left, 0,
	    WA_Top, 0,
	    WA_Width, X2,
	    WA_Height, Y2,
	    WA_IDCMP, CLOSEWINDOW,
	    WA_CloseGadget, TRUE,
	    WA_Title, "Sprial",
	    TAG_DONE);
    
    if (!Window)
	endGame("OpenWindow failed\n");

    Template = AllocRaster(X2, Y2);
    if (!Template)
	endGame("AllocRaster failed\n");

    x = 4;
    y = 30;
    chId = 0;
    SetAPen(Window->RPort, 1);
    SetDrMd(Window->RPort, JAM1);
    do {
	do {
	    if (chId == 0x3000)
		chId = 0xe800;
	    if (error = setInfo(GlyphEngine, OT_GlyphCode, chId++, TAG_DONE))
		endGame("setInfo() failed %ld\n", error);
	    if (!obtainInfo(GlyphEngine, OT_GlyphMap, &glyph, TAG_DONE)) {
		for (i = (glyph->glm_BMRows*glyph->glm_BMModulo/4) - 1;
			i >= 0; i--)
		    ((ULONG *) Template)[i] = ((ULONG *) glyph->glm_BitMap)[i];
		dx = x - glyph->glm_X0 + glyph->glm_BlackLeft;
		dy = y - glyph->glm_Y0 + glyph->glm_BlackTop;
		BltTemplate(((char *) Template)+
			(glyph->glm_BlackTop*glyph->glm_BMModulo)+
			((glyph->glm_BlackLeft/16)*2), glyph->glm_BlackLeft&0xf,
			glyph->glm_BMModulo, Window->RPort, dx, dy,
			glyph->glm_BlackWidth, glyph->glm_BlackHeight);
		releaseInfo(GlyphEngine, OT_GlyphMap, glyph, TAG_DONE);
		x += glyph->glm_X1-glyph->glm_X0;
	    }
	}
	    while (x < 620);
	x = 4;
	y += 20;
    }
	while (y < 390);

    D(("done w/ strikefont.\n"));
    D(("UserPort $%lx\n", Window->UserPort));
    /* wait until done before returning */
    for (;;) {
	while (IM = (struct IntuiMessage *) GetMsg(Window->UserPort)) {
	    D(("IM Class $%08lx\n", IM->Class));
	    if (IM->Class & CLOSEWINDOW) {
		D(("CLOSEWINDOW\n"));
		endGame(0);
	    }
	    ReplyMsg((struct Message *) IM);
	}
	D(("WaitPort..."));
	WaitPort(Window->UserPort);
	D(("WAKE!\n"));
    }
}
