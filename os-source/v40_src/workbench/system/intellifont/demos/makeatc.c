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
#include "diskfonttag.h"

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include <string.h>
#undef    strchr
#undef    strcmp
#undef    strcpy
#undef    strrchr
#undef    memset

extern struct Library *SysBase;
extern struct Library *DOSBase;

#define  TEMPLATE	"NAME/A"

#define  O_NAME		0
#define  O_COUNT	1

#define  MAXFILESIZE	4000

struct Library *GfxBase = 0;
struct Library *UtilityBase = 0;
struct RDArgs *RDArgs = 0;
BPTR OTFile = 0;
BPTR ATCFile = 0;
struct TagItem OTags[MAXFILESIZE / sizeof(struct TagItem)];

ULONG atcImage[] = {
    /* typeface ID */
    0x00000000,
    /* ECMA-94 to CG Character Code map */
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x032C0055, 0x1C5806B7, 0x064206A6, 0x00351C57,
    0x06210622, 0x06B606A8, 0x061E0625, 0x061D0628,
    0x06410638, 0x0639063A, 0x063B063C, 0x063D063E,
    0x063F0640, 0x061F0620, 0x1C6F06AB, 0x1C6E0056,
    0x1C51002F, 0x002A0025, 0x00290027, 0x002D0022,
    0x001D0023, 0x00310034, 0x0020001F, 0x001E001C,
    0x00240033, 0x0021002B, 0x001B0032, 0x00260030,
    0x002E002C, 0x00280623, 0x07230624, 0x1C941C72,
    0x1C910015, 0x0010000B, 0x000F000D, 0x00130008,
    0x00030009, 0x0017001A, 0x00060005, 0x00040002,
    0x000A0019, 0x00070011, 0x00010018, 0x000C0016,
    0x00140012, 0x000E1C52, 0x1C711C53, 0x1C951D02,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x032C0062, 0x064306A4, 0x072106BD, 0x1C7606B4,
    0x1C9312A0, 0x06AF0060, 0x1B6A0625, 0x129F1C96,
    0x06B106AA, 0x06540655, 0x1C9204DE, 0x06B50770,
    0x1C980653, 0x06B00061, 0x068A0689, 0x068B0063,
    0x02360235, 0x02370238, 0x02330234, 0x00AC0114,
    0x0227023A, 0x023B023C, 0x023D023E, 0x023F0240,
    0x016B0241, 0x02420243, 0x02390245, 0x022E06AC,
    0x01230247, 0x02460248, 0x0249016F, 0x016C00B1,
    0x00E700E8, 0x00EA00EB, 0x00E900EC, 0x00AE00EE,
    0x00EF00F0, 0x00F200F1, 0x00F500F4, 0x00F700F6,
    0x016D00F8, 0x00FA00F9, 0x00FC0102, 0x00FB06AD,
    0x00FD00FF, 0x00FE0101, 0x01000170, 0x016E02D0
};

void
endGame(format, arg1, arg2, arg3, arg4)
char *format, *arg1, *arg2, *arg3, *arg4;
{
    if (format) {
	D((format, arg1, arg2, arg3, arg4));
	printf(format, arg1, arg2, arg3, arg4);
    }
    if (OTFile)
	Close(OTFile);
    if (ATCFile)
	Close(ATCFile);
    if (RDArgs)
	FreeArgs(RDArgs);
    if (UtilityBase)
	CloseLibrary(UtilityBase);
    if (format)
	exit(5);
    exit(0);
}


void
main()
{
    ULONG *options[O_COUNT];
    char pathStore[256], *filePath, *s;
    int i;

    D(("Test\nOpenLibrarys..."));
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
    filePath = pathStore + 6;
    strcpy(filePath, FilePart((char *) options[O_NAME]));
    s = filePath + strlen(filePath);
    strcat(s, OTSUFFIX);

    D((".otag file \"%s\"\n", filePath));
    /* open .otag file */
    OTFile = Open(pathStore, MODE_OLDFILE);

    if (!OTFile) {
	*s = '\0';
	endGame("no outline font file for \"%s.font\"\n", filePath);
    }

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

    /* get the library name */
    s = (char *) GetTagData(OT_Engine, 0, OTags);
    if (!s)
	endGame("no OT_Engine tag\n");

    if (strcmp(s, "bullet"))
	endGame("OT_Engine not \"bullet\" but \"%s\"\n", s);

    /* get the typeface ID for the Compugraphic typeface */
    atcImage[0] = GetTagData(OT_Spec + 1, 0, OTags);
    if (!atcImage[0])
	endGame("no typeface ID\n");


    /* open destination */
    strcpy(pathStore, (char *) options[O_NAME]);
    strcat(pathStore, ".atc");
    ATCFile = Open(pathStore, MODE_NEWFILE);

    if (!ATCFile) {
	endGame("cannot open output file \"%s.atc\"\n", pathStore);
    }

    if (Write(ATCFile, atcImage, sizeof(atcImage)) != sizeof(atcImage))
	endGame("write error %ld for output file \"%s.atc\"\n",
		IoErr(), pathStore);

    endGame(0);
}
