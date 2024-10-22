/*
**	$Id: fountain.h,v 37.9 92/02/07 11:55:01 davidj Exp $
**
**	Fountain/fountain.h -- program storage definitions
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
*/

/* #define  INTUITION_IOBSOLETE_H	/* don't need obsolete defines */
#include <exec/types.h>
#include <exec/lists.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <exec/initializers.h>
#include <dos/dos.h>
#include <dos/dosasl.h>
#include <dos/dosextens.h>
#include <dos/filehandler.h>
#include <dos/rdargs.h>
#include <graphics/gfx.h>
#include <graphics/rastport.h>
#include <graphics/text.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/asl.h>
#include <libraries/gadtools.h>
#include <utility/tagitem.h>
#include <workbench/workbench.h>

#undef   GLOBAL					/* from exec/types.h */
#ifndef  ID_PC_DISK
#define  ID_PC_DISK	0x4D534400		/* MSD\0 */
#endif

#include "diskfonttag.h"
#include "diskfont.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <setjmp.h>
#include "port.h"
#include "debug.h"

#undef   memset					/* from stdlib.h */
#undef   memcmp
#undef   memcpy
#undef   strlen					/* from string.h */
#undef   strcmp
#undef   strcpy

#include "dir.h"
#include "lzs.h"

#include <clib/asl_protos.h>
#include <clib/exec_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/dos_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/icon_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/asl_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/diskfont_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/utility_pragmas.h>

/* --- common used externals --- */
extern struct Library *SysBase;

/* shared metrics for window */
#define  INSIDEWIDTH	566

/* gadget constants */
#define  MAXDDIRS	99		/* max that G_DDIRNEXT can hold */
#define  DDNLABELLEN	26		/* size of G_DDIRNEXT label */

#define  G_SDIRREQ	1
#define  G_SDIR		2
#define  G_SFACE	3
#define  G_DDIRNEXT	4
#define  G_DDIRREQ	5
#define  G_DDIR		6
#define  G_DFACE	7
#define  G_INSTALL	8
#define  G_MODIFY	9

#define  G_HELP		10

#define  GLDTYPE_PCDOS		0	/* PC-DOS Distribution Disk */
#define  GLDTYPE_AMIGA		1	/* Amiga Distribution Disk */
#define  GLDTYPE_COUNT		2	/* number of GLDTYPE_s */

#define  G_MDIR		11
#define  G_MFACE	12
#define  G_MFACEACTIVE	13
#define  G_SIZE		14
#define  G_ADDSIZE	15
#define  G_DELSIZE	16
#define  G_SIZENUM	17
#define  G_CREATEBM	18
#define  G_DELETEBM	19
#define  G_DELETEFACE	20
#define  G_PERFORM	21
#define  G_CANCEL	22


/* fFlags */
#define  FFLAG_TYPE	0x01		/* assoc .type file exists */
#define  FFLAG_FONT	0x02		/* assoc .font file exists */
#define  FFLAG_OFONT	0x04		/* assoc .font file exists & OFC_ID */
#define  FFLAG_OTAG	0x08		/* assoc .otag file exists */
#define  FFLAG_COMPLETE 0x0f		/* everything in place */

#define  FFLAG_CREATE	0x10		/* this record needs to be created */
#define  FFLAG_DELETE   0x20		/* this record needs to be deleted */
#define  FFLAG_DIRTY	0x30		/* this record needs to be changed */

#define  FFLAG_TAG	0x40		/* flag used while updating */

struct TypeEntry {
    struct MinNode node;
    UWORD   bucketBits;			/* typeface bucket flags */
    LONG    typefaceID;			/* typeface id */
    LONG    complementID;		/* character complement */
    LONG    spaceReq;			/* space required for the library */
    ULONG   faceheaderOffset;		/* offset in library file */
    ULONG   faceheaderCount;		/*   and size of face header */
    UWORD   ySizeFactor;		/* ascender - descender */
    UWORD   ySizeNumerator;		/* DU/EM */
    UWORD   spaceWidth;			/* width of space in DU */
    UBYTE   serifFlag;
    UBYTE   stemWeight;
    UBYTE   slantStyle;
    UBYTE   horizStyle;
    UBYTE   isFixed;			/* non-zero for fixed width */
    UBYTE   fFlags;			/* used during list processing */
    char    family[21];			/* null terminated string */
    char    amigaName[26];		/* null terminated string */
    char    sourceFile[PATHNAMELEN];	/* source file path */
    char    typePath[PATHNAMELEN];	/* type file path */
};

struct OTagEntry {
    struct MinNode node;
    struct MinList sizes;
    struct TypeEntry *te;
    struct OTagEntry *bDef, *iDef, *biDef;
    struct OTagEntry *bRef, *iRef, *biRef;
    UWORD   symbolSet;
    UBYTE   fFlags;			/* used during list processing */
    char    amigaName[26];
    char    bRefName[26];
    char    iRefName[26];
    char    biRefName[26];
    char    otagPath[PATHNAMELEN];	/* otag file path */
};

struct FontEntry {
    struct MinNode node;
    struct MinList sizes;
    struct OTagEntry *oe;
    struct TypeEntry *te;
    UBYTE   fFlags;			/* used during list processing */
    char    amigaName[26];
    char    fontPath[PATHNAMELEN];	/* font contents file path */
};

#define  FONTFILELEN	62		/* 30 / 30 <null> */
struct SizeEntry {
    struct MinNode node;
    union {
	struct OTagEntry *oe;
	struct FontEntry *fe;
    } ref;
    UWORD   ySize;
    ULONG   dpi;
    ULONG   dotSize;
    UWORD   symbolSet;
    UBYTE   fFlags;
    char    fontFile[FONTFILELEN];	/* not 256 because Fountain created */
};


struct FontsEntry {			/* list of Fonts: assigns */
    struct MinNode node;
    struct MinList otList;		/* OTagEntry in path */
    struct MinList fcList;		/* FontEntry in path */
    struct MinList tfList;		/* TypeEntry in path */
    char    indexName[DDNLABELLEN];
    char    assignPath[PATHNAMELEN];
};


struct FaceDisplay {
    struct Node node;			/* linking, displaying */
    struct MinList sizes;
    union {
	struct FontEntry *fe;
	struct TypeEntry *te;
    } ref;
    char name[28];
};


struct SizeDisplay {
    struct Node node;
    UWORD ySize;
    UBYTE isBitmap;
    UBYTE   fFlags;
    char    name[16];			/* for displaying */
};


/* static memory pool */
union StaticPool {

/*   for FAISLoad */
#define  MAX_BUFFER                    8000	/* Must be greater or equal */
						/* to MAX_CHAR_SIZE */
    struct {
	BYTE	buffer[MAX_BUFFER];
	BYTE    buffer2[MAX_BUFFER];
    } b;

/*   for various constructions */
    struct {
	UBYTE   TempBuffer2[PATHNAMELEN];
	UBYTE   TempBuffer[1];		/* actually sizeof(fl)-PATHNAMELEN */
    } t;
};

void MakeReady (void);
