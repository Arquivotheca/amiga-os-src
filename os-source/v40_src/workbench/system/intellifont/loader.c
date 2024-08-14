/*
**	$Id: loader.c,v 37.11 92/05/26 11:39:35 darren Exp $
**
**	Fountain/loader.c -- load FAIS data
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
**
**	based on licensed code...
**
**	Intellifont Loader Version 2.1 Release 07-Oct-89
**	Copyright (c) 1988 Compugraphic Corporation ALL RIGHTS RESERVED
**
**	Copyright (C) 1987,1988, All Rights Reserved, by
**	Compugraphic Corporation, Wilmington, Ma.
**
**	This software is furnished under a license and may be used and copied
**	only in accordance with the terms of such license and with the
**	inclusion of the above copyright notice. This software or any other
**	copies thereof may not be provided or otherwise made available to any
**	other person. No title to and ownership of the software is hereby
**	transferred.
**
**	The information in this software is subject to change without notice
**	and should not be construed as a commitment by Compugraphic
**	Corporation.
**
*/
/*
*  History:
*  ---------
*  06-Oct-89 Created.
** File: LIB_LOAD.C
** Current Status:
**   1. Can not add characters to an exisiting library face.
**   2. Can not handle multi-diskette FAIS volumes.
*/

/*  02-03-92 - TnC (Agfa) - Fixed non-Intellifont data bug and Fontalias swap */

#include  "fountain.h"

extern struct Library *DOSBase;

extern union StaticPool S;

#define  MIN_CHAR_SIZE                   12
#define  MAX_CHAR_SIZE                 2000
#define  MAX_FONT_FILES                  32
#define  MAX_BUFFER                    8000	/* Must be greater or equal */
 /* to MAX_CHAR_SIZE */
#define  MAX_CHARS                      600
#define  MAX_KEYS                        84	/* max number of keys for */
 /* file headers and hiqdata1 */
#define  MAX_HEADER        6 * MAX_KEYS + 8
#define  MAX_INDEX                      256
#define  MAX_LIBBUF                    2000


/*  global defines */
#define  HIQDATA1                       502
#define  GLOBALCHDATA                  5001
#define  CGCHARNUMS                    5002
#define  GLOBINTDATA                   5003
#define  DISPLAYHEADER                  301
#define  CHARWIDTH                      201
#define  ATTRIBUTEHEADER                102
#define  CCHARMETRICS                   233
#define  COMPLEMENTHEADER               104
#define  COMPOUNDCHAR                   231
#define  CCID                           234
#define  DESIGNKERN                     212
#define  FONTALIAS                      107
#define  FONTHEADER                     101
#define  TEXTKERN                       211
#define  TRACKKERN                      213
#define  TYPEFACEHEADER                 103
#define  FILE_HEADER_SIZE                32
#define  FACE_HEADER_SIZE                32
#define  FILE_DIRECTORY_SIZE            128
#define  DISK_ALIGN                      16
#define  COPYRIGHTSIZE                   62
#define  RASTERPARAMSIZE                 10
#define  OK                               0

#define  fileheader_key                   1
#define  filedirectory_key                2
#define  faceglobal_key                  10
#define  globalintellifont_key          100
#define  trackkern_key                  101
#define  textkern_key                   102
#define  designkern_key                 103
#define  charwidth_key                  104
#define  attributeheader_key            105
#define  rasterparam_key                106
#define  typefaceheader_key             107
#define  compoundchar_key               108
#define  displayheader_key              109
#define  fontalias_key                  110
#define  copyright_key                  111
#define  cchartextkern_key              112
#define  cchardesignkern_key            113
#define  endofdir_key             MAX_UWORD
#define  longendofdir_key               -1l
#define  error_return                    -1
#define  updmask                 0x80000000
#define  limmask                 0x40000000
#define  unvmask                 0x20000000
#define  normask                 0x00000000
#define  YLINES                           0
#define  YCLASS                           1
#define  YDIM                             2
#define  XDIM                             3
#define  YDIMFLAGS                        0
#define  XDIMFLAGS                        1
#define  HQ2FLAG                          2

/*  structure definitions */
union PointerUnion
{
    BYTE *b;
    WORD *w;
    ULONG *l;
};

typedef struct
{
    UWORD key;
    ULONG offset;
} keyoffset_type;

typedef struct
{
    UWORD fileId;
    ULONG fileLength;
    UWORD NKEYS;
    keyoffset_type keyOffsets[1];
} fileheader_type;

typedef struct
{
    UWORD blockID;
    ULONG blockLength;
    UWORD NBKEYS;
    keyoffset_type keyOffsets[1];
} hiqdata1_type;

typedef struct
{
    UWORD NCHAR;
    WORD fontSlant;
    ULONG offsets[1];
} globalchdata_type;

typedef UWORD cgcharnums_type;

typedef struct
{
    UWORD cg_num;
    ULONG offset;
    UWORD size;
} char_type;

typedef struct
{
    ULONG faceglobal_offset;
    UWORD faceglobal_size;
    char_type chars[1];
} faceheader_type;

typedef struct
{
    UWORD key;
    ULONG offset;
    UWORD size;
} seg_dir_type;

typedef struct
{
    ULONG face;
    ULONG offset;
    UWORD size;
} file_dir_type;

typedef struct
{
    UWORD NCHAR;
    BYTE binaryFileName[12];
    UWORD fontLimits[4];
    UWORD reverseVideoLimits[4];
    UWORD leftReference;
    UWORD baselinePosition;
    UWORD minimumPointSize;
    UWORD maximumPointSize;
    UWORD minimumSetSize;
    UWORD maximumSetSize;
    UBYTE controlCode[4];
    UWORD masterPointSize;
    UWORD scanDirection;
    WORD italicAngle;
    WORD xHeight;
    UWORD scanResolutionY;
    UWORD scanResolutionX;
    UWORD outputEnable;
} displayheader_type;

typedef struct
{
    UWORD charWidth;
    UWORD charFlags;
} charwidth_type;

typedef struct
{
    UBYTE data[4];
} kerndata_type;

typedef struct
{
    WORD kernSign;
    UWORD kernUnit;
    UWORD NSECT;
    kerndata_type character[1];
} packedkern_type;

typedef struct
{
    UWORD data[8];
} unpkerndata_type;

typedef struct
{
    WORD kernSign;
    UWORD kernUnit;
    UWORD NSECT;
    unpkerndata_type character[1];
} unpackedkern_type;

typedef struct
{
    UBYTE languageType;
    UBYTE fontUsage;
    UBYTE isFixedPitch;
    UBYTE escapeUnit;
    UWORD scaleFactor;
    UWORD fixedSpaceRelWidths[3];
    UWORD leftReference;
    UWORD baselinePosition;
    WORD windowTop;
    WORD windowBottom;
    struct
    {
	WORD zeroPoint;
	WORD variablePoint;
    } autoVarComp[3];
    WORD ascender;
    WORD descender;
    WORD capHeight;
    WORD xHeight;
    WORD lcAccenHeight;
    WORD ucAccentHeight;
    UWORD charPica;
    WORD leftAlign;
    WORD rightAlign;
    WORD uscoreDepth;
    UWORD uscoreThickness;
    WORD windowLeft;
    WORD windowRight;
    UWORD spaceBand;
} attributeheader_type;

typedef struct
{
    UWORD NFACES;
    BYTE typeFaceName[50];
    BYTE familyName[20];
    BYTE weight[20];
    LONG typeFaceSet[3];
} typefaceheader_type;

typedef struct
{
    UWORD key;
    ULONG size;
    seg_dir_type seg_dir;
} faceglobal_type;

typedef struct
{
    UWORD cg_num;
    WORD xoffset;
    WORD yoffset;
} libpart_type;

typedef struct
{
    UWORD cg_num;
    WORD horiz_esc;
    WORD vert_esc;
    WORD NPCC;
    libpart_type parts[1];
} libcompoundchar_type;

typedef struct
{
    UWORD ccCharCode;
    WORDVECTOR offsets;
} part_type;

typedef struct
{
    UWORD ccCharCode;
    UWORD NPCC;
    part_type parts[1];
} compoundchar_type;

typedef struct
{
    UWORD isoCode;
    LONG typeFace;
    UWORD cgCode;
} isocodetable_type;


typedef struct
{
    BYTE typeFaceId[6];
    BYTE complementId[6];
    BYTE characterPlane[50];
    UWORD substituteCode;
    isocodetable_type table[1];
} complementheader_type;

typedef struct
{
    UWORD cg_num;
    LONG face;
} ccid_type;

typedef struct
{
    WORD escapementX;
    WORD escapementY;
    BYTE amplified;
    BYTE correction;
    WORD leftExtent;
    WORD rightExtent;
    WORD ascent;
    WORD descent;
} ccharmetrics_type;

/*
**  Definition of Font Index File
*/
struct
{
    UWORD volId;
    UWORD nVols;
    UWORD volNo;
    LONG volLength;
    UWORD nFiles;
    struct
    {
	BYTE face[6];
	BYTE comp[6];
	BYTE style[2];
	BYTE pointSize[4];
	BYTE setSize[4];
	BYTE res[4];
	BYTE orient[4];
	BYTE fileId[5];
	BYTE fileType[2];
	BYTE fill[5];
	UWORD dupCode;
	UWORD nVol;
	UWORD iVol;
	BYTE fileName[8];
    } font[MAX_FONT_FILES];
} index_file;


struct FaisLocals
{
    ULONG typeface;
    ULONG complement;
    LONG faceheaderoffset;
    LONG globintdataoffset;
    ULONG hiqdataoffset;
    LONG globintdatasize;
    LONG faceheadersizeoffset;
    ULONG ySizeNumerator;
    ULONG ySizeDenominator;
    char *faisPath;
    char *libName;
    char *fontName;
    FILE *FONTDISPLAY;
    FILE *FONTATTRIBUTE;
    FILE *LIB;
    BYTE *updcompoundcharptr;
    UWORD orThreshold;
    UWORD nchar_in_lib;		/* number of valid characters to be put in lib */
    UWORD nchar_in_font;	/* number of chars in font including missing and pi etc. */
    UWORD faceheadersize;
    WORD fontSlant;
    UWORD spaceBand;
    UBYTE isFixed;
    char displayFile[9];
    char attributeFile[9];
};

/* global buffers */
BYTE faceheader[8 * MAX_CHARS + 14];
BYTE designkern[16 * MAX_CHARS + 6];
BYTE textkern[16 * MAX_CHARS + 6];
BYTE dfileheader[MAX_HEADER];
BYTE afileheader[MAX_HEADER];
BYTE hiqdata[MAX_HEADER];
BYTE updindextables[5][MAX_INDEX];
BYTE limindextables[5][MAX_INDEX];
BYTE unvindextables[5][MAX_INDEX];


/* define Loader error code */
#define ld_err_mem     100	/* Insufficient memory */
#define ld_err_4       104	/* Maximum files limit exceeded */
#define ld_err_5       105	/* Typeface/Complement number not found */
#define ld_err_9       109	/* Global Character Data segment not found */
#define ld_err_10      110	/* Cg Character Number Data segment not found */
#define ld_err_11      111	/* Can't open Attribute file */
#define ld_err_13      113	/* Font Header segment not found */
#define ld_err_15      115	/* Character Width segment not found */

/* define Loader warning code */
#define ld_wrn_1      1001	/* Display Header segment not found */
#define ld_wrn_4      1004	/* Attribute Header segment not found */
#define ld_wrn_8      1008	/* Typeface Header segment not found */

#define  TRUE	1
#define  FALSE	0

UWORD lib_load (WORD, BYTE **);
ULONG align (ULONG);
WORD directory_setup (struct FaisLocals *);
WORD face_header_setup (struct FaisLocals *);
WORD library_header_setup (struct FaisLocals *);
WORD global_segment_setup (struct FaisLocals *);
WORD write_characters (struct FaisLocals *);
void sort (struct FaisLocals *, charwidth_type *, packedkern_type *, packedkern_type *);
void pack_kerning (struct FaisLocals *, packedkern_type *, unpackedkern_type *);
WORD assemblecompoundchar (struct FaisLocals * fl, WORD *, WORD *, complementheader_type *,
			    ccharmetrics_type *, ccid_type *);
    WORD code_to_cgnum (struct FaisLocals * fl, UWORD, isocodetable_type *);
    void cleanup (struct FaisLocals *);
    void swapcharwidth (struct FaisLocals * fl, BYTE *);
    void swapattributeheader (struct FaisLocals * fl, BYTE *);
    void swapdisplayheader (BYTE *);
    void swaptypefaceheader (BYTE *);
    void swaptrackkern (BYTE *);
    void swapcharacters (BYTE *);

#ifdef	DEBUG
#undef	SWAPU
#undef  SWAPW
#define SWAPW	(WORD) SWAPU
    UWORD swapbuffer[128] =
    {0};
    int dumpuctr = 0;
    UWORD SWAPU (w)
    UWORD w;
{
    UWORD r;
    short i;

    r = (UWORD) (((UWORD) w >> 8) + ((((UWORD) w & 0xff) << 8)));

    for (i = 127; i > 0; i--)
	swapbuffer[i] = swapbuffer[i - 1];
    swapbuffer[0] = r;

    if ((r == 0x2a71) || (r == 0x712a) || (r == 0x2975) || (r == 7529))
    {
	DBG ("vvv\n");
	for (i = 127 - dumpuctr; i > 0; i--)
	    DBG2 ("SWAPU 0x%04lx 0x%04lx\n",
		  ((UWORD) swapbuffer[i] >> 8) + (((UWORD) swapbuffer[i] & 0xff) << 8),
		  swapbuffer[i]);
	dumpuctr = 128;
    }
    if (dumpuctr)
    {
	if (dumpuctr == 128)
	{
	    DBG2 ("SWAPU 0x%04lx 0x%04lx <<<\n", w, r);
	}
	else
	{
	    DBG2 ("SWAPU 0x%04lx 0x%04lx\n", w, r);
	}
	dumpuctr--;
	if (!dumpuctr)
	    DBG ("^^^\n");
    }
    return (r);
}

#endif

int FAISLoad (typeEntry, libNameArg, orThreshArg)
    struct TypeEntry *typeEntry;
    char *libNameArg;
    UWORD orThreshArg;
{
    struct FaisLocals fl;	/* shared local data */

    DBG1 ("FAISLoad \"%s\"\n", libNameArg);
    fl.faisPath = typeEntry->sourceFile;
    fl.libName = libNameArg;
    fl.fontName = typeEntry->amigaName;
    fl.typeface = typeEntry->typefaceID;
    fl.complement = typeEntry->complementID;
    fl.orThreshold = orThreshArg;

    fl.globintdataoffset = -1;
    fl.updcompoundcharptr = NULL;
    fl.nchar_in_lib = 0;
    fl.nchar_in_font = 0;
    fl.FONTDISPLAY = 0;
    fl.FONTATTRIBUTE = 0;
    fl.LIB = 0;

/*  Process 1.2.2   Diskette Directory Setup */
    DBG ("directory_setup...\n");
    if (directory_setup (&fl))
    {
	cleanup (&fl);
	return (FALSE);
    }

/*  Process 1.2.3   Face Header Setup */
    DBG ("face_header_setup...\n");
    if (face_header_setup (&fl))
    {
	cleanup (&fl);
	return (FALSE);
    }

/*  Process 1.2.4   Library Header Setup */
    DBG ("library_header_setup...\n");
    if (library_header_setup (&fl))
    {
	cleanup (&fl);
	return (FALSE);
    }


/*   Process 1.2.6   Global Segment Setup */
    DBG ("global_segment_setup...\n");
    if (global_segment_setup (&fl))
    {
	cleanup (&fl);
	return (FALSE);
    }

/*   Process 1.2.7   Write Characters */
    DBG ("write_characters...\n");
    if (write_characters (&fl))
    {
	cleanup (&fl);
	return (FALSE);
    }

    DBG ("cleanup...\n");
    cleanup (&fl);
    typeEntry->faceheaderOffset = fl.faceheaderoffset;
    typeEntry->faceheaderCount = fl.faceheadersize;
    typeEntry->ySizeNumerator = fl.ySizeNumerator;
    typeEntry->ySizeFactor = fl.ySizeDenominator;
    typeEntry->spaceWidth = fl.spaceBand;
    typeEntry->isFixed = fl.isFixed;
    return (TRUE);
}

BOOL faisRead (struct FaisLocals * fl, void *data, int length, BOOL equ, FILE * f)
{
    int actual;
    char string[256];

    if ((actual = fread ((char *) data, 1, length, f)) != length)
    {
	if ((!equ) && (actual > 0))
	    return (SUCCESS);
	if (f == fl->FONTDISPLAY)
	    buildpath (string, fl->faisPath, fl->displayFile);
	else
	    buildpath (string, fl->faisPath, fl->attributeFile);
	ErrRequester (ERROR_IFileRead, fl->fontName, string, length,
		      actual, ferror (f));
	return (FAILURE);
    }
    return (SUCCESS);
}

BOOL faisWrite (struct FaisLocals * fl, void *data, int length)
{
    int actual;

    if ((actual = fwrite ((char *) data, 1, length, fl->LIB)) != length)
    {
	ErrRequester (ERROR_IFileWrite, fl->fontName, fl->libName, length,
		      actual, ferror (fl->LIB));
	fclose (fl->LIB);
	fl->LIB = 0;
	DeleteFile (fl->libName);
	return (FAILURE);
    }
    return (SUCCESS);
}


/*
**  Process 1.2.2
*/
WORD
directory_setup (fl)
    struct FaisLocals *fl;
{
    BYTE string[256];
    WORD i, found;
    LONG face;
    LONG comp;
    FILE *FONTINDEX;
    int l;

/*
 *  Verify Font Disk on user-specified or default drive:
 */
    buildpath (string, fl->faisPath, "FONTINDX.FI");
    if ((FONTINDEX = fopen (string, "r")) == NULL)
    {
	ErrRequester (ERROR_IOpenFail, fl->fontName, string);
	return (FAILURE);
    }
/*
 *  Read Font Index File
 */
    if ((l = fread ((char *) &index_file, 1, sizeof (index_file), FONTINDEX))
	== -1)
    {
	ErrRequester (ERROR_IFileRead, fl->fontName, string, sizeof (index_file),
		      l, ferror (fl->LIB));
	return (FAILURE);
    }
/*
**  Validate Font Diskette
*/
    fclose (FONTINDEX);
    if (SWAPU (index_file.nFiles) > MAX_FONT_FILES)
    {
	ErrRequester (ERROR_IInternalFAIS, fl->fontName, ld_err_4);
	DBG ("Maximum files limit exceeded\n");
	return (FAILURE);
    }

/*
 *  Verify Font in Font Disk
 */
    found = FALSE;
    for (i = 0; i < SWAPU (index_file.nFiles); i++)
    {
	if ((index_file.font[i].fileType[0] == 'F') &&
	    (index_file.font[i].fileType[1] == 'D'))
	{
	    strncpy (string, index_file.font[i].face, 6);
	    string[6] = '\0';
	    face = atol (string);
	    strncpy (string, index_file.font[i].comp, 6);
	    string[6] = '\0';
	    comp = atol (string);
	    if ((fl->typeface == face) && (fl->complement == comp))
	    {
		found = TRUE;
		break;
	    }
	}
    }
    if (!found)
    {
	DBG ("Typeface/Complement number not found\n");
	ErrRequester (ERROR_IInternalFAIS, fl->fontName, ld_err_5);
	return (ld_err_5);
    }
/*
**  Open Font Display File
*/
    getString (fl->displayFile, index_file.font[i].fileName, 8);
    buildpath (string, fl->faisPath, fl->displayFile);
    if ((fl->FONTDISPLAY = fopen (string, "r")) == NULL)
    {
	ErrRequester (ERROR_IOpenFail, fl->fontName, string);
	return (FAILURE);
    }

/*
 *  Locate Attribute File on Font Disk
 */
    found = FALSE;
    for (i = 0; i < SWAPU (index_file.nFiles); i++)
    {
	if ((index_file.font[i].fileType[0] == 'F') &&
	    (index_file.font[i].fileType[1] == 'A'))
	{
	    strncpy (string, index_file.font[i].face, 6);
	    string[6] = '\0';
	    fl->typeface = atol (string);
	    strncpy (string, index_file.font[i].comp, 6);
	    string[6] = '\0';
	    fl->complement = atol (string);
	    if ((fl->typeface == face) && (fl->complement == comp))
	    {
		found = TRUE;
		break;
	    }
	}
    }
    if (!found)
    {
	DBG ("Typeface/Complement number not found\n");
	ErrRequester (ERROR_IInternalFAIS, fl->fontName, ld_err_5);
	return (ld_err_5);
    }
/*
**  Open Font Attribute File
*/
    getString (fl->attributeFile, index_file.font[i].fileName, 8);
    buildpath (string, fl->faisPath, fl->attributeFile);
    if ((fl->FONTATTRIBUTE = fopen (string, "r")) == NULL)
    {
	ErrRequester (ERROR_IOpenFail, fl->fontName, string);
	return (FAILURE);
    }
    return (OK);
}

WORD
face_header_setup (fl)
    struct FaisLocals *fl;
{
    keyoffset_type *ptr;
    UWORD i;
    ULONG cgcharnumsoffset, globalchdataoffset;
    globalchdata_type *globalchdata;
    cgcharnums_type *cgcharnums;
    char_type *charptr;
    LONG *offsets, *nextoffset;
    faceheader_type *faceheaderptr;
    UWORD nkeys;

    /* read display file header */
    if (faisRead (fl, dfileheader, MAX_HEADER, FALSE, fl->FONTDISPLAY))
	return (FAILURE);

    /* locate hiqdata1 */
    fl->hiqdataoffset = MAX_LONG;
    ptr = (keyoffset_type *) (((fileheader_type *) dfileheader)->keyOffsets);
    nkeys = SWAPU (((fileheader_type *) dfileheader)->NKEYS);
    if (nkeys > MAX_KEYS)
    {
	DBG2 ("nkeys truncated from %ld to %ld\n", nkeys, MAX_KEYS);
	nkeys = MAX_KEYS;
    }
    for (i = 0; i < nkeys; i++)
    {
	if (SWAPU (ptr->key) == HIQDATA1)
	{
	    fl->hiqdataoffset = SWAPL (ptr->offset);
	    break;
	}
	ptr++;
    }

    if (fl->hiqdataoffset == MAX_LONG)
    {
	DBG ("Error reading HIQDATA1 segment\n");
	ErrRequester (ERROR_IInternalFAIS, fl->fontName, ld_err_9);
	return (ld_err_9);
    }

    /* read hiqdata1 segment */
    fseek (fl->FONTDISPLAY, fl->hiqdataoffset, 0);
    if (faisRead (fl, hiqdata, MAX_HEADER, FALSE, fl->FONTDISPLAY))
    {
	DBG ("Error reading HIQDATA1 segment\n");
	return (FAILURE);
    }

    /* locate cgcharnums and globalchdata */
    cgcharnumsoffset = MAX_LONG;
    globalchdataoffset = MAX_LONG;
    fl->globintdataoffset = MAX_LONG;
    ptr = (keyoffset_type *) (((hiqdata1_type *) hiqdata)->keyOffsets);
    nkeys = SWAPU (((hiqdata1_type *) hiqdata)->NBKEYS);
    if (nkeys > MAX_KEYS)
    {
	DBG2 ("hiq nkeys truncated from %ld to %ld\n", nkeys, MAX_KEYS);
	nkeys = MAX_KEYS;
    }
    for (i = 0; i < nkeys; i++)
    {
	if (SWAPU (ptr->key) == CGCHARNUMS)
	    cgcharnumsoffset = fl->hiqdataoffset + SWAPL (ptr->offset);
	if (SWAPU (ptr->key) == GLOBALCHDATA)
	    globalchdataoffset = fl->hiqdataoffset + SWAPL (ptr->offset);
	if (SWAPU (ptr->key) == GLOBINTDATA)
	{			/* will need it in global_segment_setup() */
	    fl->globintdataoffset = fl->hiqdataoffset + SWAPL (ptr->offset);
	    fl->globintdatasize = SWAPL ((ptr + 1)->offset) - SWAPL (ptr->offset);
	}

	ptr++;
    }
    if (cgcharnumsoffset == MAX_LONG)
    {
	DBG ("Error reading CGCHARNUMS segment.\n");
	ErrRequester (ERROR_IInternalFAIS, fl->fontName, ld_err_10);
	return (ld_err_10);
    }
    if (globalchdataoffset == MAX_LONG)
    {
	DBG ("Error reading GLOBALCHDATA segment.\n");
	ErrRequester (ERROR_IInternalFAIS, fl->fontName, ld_err_9);
	return (ld_err_9);
    }

    /* read globalchdata into buffer */
    globalchdata = (globalchdata_type *) S.b.buffer;
    fseek (fl->FONTDISPLAY, globalchdataoffset, 0);
    if (faisRead (fl, (char *) globalchdata, 4 * MAX_CHARS + 10, FALSE,
		  fl->FONTDISPLAY))
    {
	DBG (" Error reading GLOBALCHDATA segment.\n");
	return (FAILURE);
    }

    /* read cgcharnums into buffer */
    cgcharnums = (cgcharnums_type *) (S.b.buffer + 4 * MAX_CHARS + 10);
    fseek (fl->FONTDISPLAY, cgcharnumsoffset, 0);
    if (faisRead (fl, (char *) cgcharnums, 2 * MAX_CHARS + 4, FALSE,
		  fl->FONTDISPLAY))
    {
	DBG ("Error reading GLOBALCHDATA segment.\n");
	return (FAILURE);
    }

    fl->fontSlant = SWAPW (globalchdata->fontSlant);	/* need for rasterparam segment */

    /* setup face header structure including all entries but keeping */
    /* count of the number of actual characters in library */
    faceheaderptr = (faceheader_type *) faceheader;
    charptr = faceheaderptr->chars;
    offsets = (LONG *) globalchdata->offsets;
    fl->nchar_in_font = SWAPU (globalchdata->NCHAR);

    for (i = 0; i < fl->nchar_in_font; i++)
    {
	/* get size of ith character */
	nextoffset = offsets + 1;
	while (SWAPL (*nextoffset) == -1)
	    nextoffset++;
	charptr->size = (UWORD) (SWAPL (*nextoffset) - SWAPL (*offsets));

	/* get offset of ith character; */
	charptr->offset = SWAPL (*offsets);

	/* if this is a valid character add cgnumber otherwise */
	/* set cgnumber to MAX_UWORD */

	/* character is invalid if it is too large, too small or */
	/* has a negative cgnumber (PI character) */
	if (charptr->size >= MIN_CHAR_SIZE &&
	    charptr->size <= MAX_CHAR_SIZE &&
	    (SWAPU (*cgcharnums) & 0x8000) == 0)
	{
	    /* this is a valid character */
	    fl->nchar_in_lib++;
	    charptr->cg_num = SWAPU (*cgcharnums);
	}
	else
	{
	    if ((SWAPU (*cgcharnums) & 0x8000) == 0)
		DBG1 (" Missing character number %ld\n", *cgcharnums);
	    charptr->cg_num = MAX_UWORD;
	}

	/* increment pointers for next time around */
	charptr++;
	cgcharnums++;
	offsets++;
    }
    DBG1 ("nchar_in_lib %ld\n", fl->nchar_in_lib);

    return (OK);
}






/*
**  Process 1.2.5  Library Header Setup
*/
WORD
library_header_setup (fl)
    struct FaisLocals *fl;
{
    union PointerUnion libptr;

    libptr.b = S.b.buffer;

    /* new file */
    if (!(fl->LIB = fopen (fl->libName, "w")))
    {
	ErrRequester (ERROR_ICreateFail, fl->fontName, fl->libName);
	return (FAILURE);
    }
    memset (libptr.w, 0,
	    FILE_HEADER_SIZE + FACE_HEADER_SIZE + FILE_DIRECTORY_SIZE);

    *libptr.w++ = 'D';		/* type of library */

    *libptr.w++ = fileheader_key;	/* key for file header segment */
    *libptr.l++ = FILE_HEADER_SIZE;	/* offset to file header segment */
    libptr.w++;			/* leave size 0 for file header */

    *libptr.w++ = filedirectory_key;	/* key for file directory segment */
    *libptr.l++ = FILE_HEADER_SIZE + FACE_HEADER_SIZE;
    /* offset to file directory */
    *libptr.w++ = 20;		/* size of file directory */
    *libptr.w = endofdir_key;

    libptr.b = S.b.buffer + FILE_HEADER_SIZE + FACE_HEADER_SIZE;
    /* File directory segment */
    *libptr.l++ = fl->typeface;	/* set typeface */
    *libptr.l++ = (ULONG)
      (FILE_HEADER_SIZE + FACE_HEADER_SIZE + FILE_DIRECTORY_SIZE);
    /* offset to face header */
    /* save offset to faceheadersize since it may change */
    fl->faceheadersizeoffset = libptr.b - S.b.buffer;
    *libptr.w++ = fl->faceheadersize = 6 + 8 * (fl->nchar_in_lib + 1);

    *libptr.l = longendofdir_key;
    if (faisWrite (fl, S.b.buffer,
		   FILE_HEADER_SIZE + FACE_HEADER_SIZE + FILE_DIRECTORY_SIZE))
	return (FAILURE);
    return (SUCCESS);
}


WORD global_segment_setup (fl)
    struct FaisLocals *fl;
{

    ULONG displayheaderoffset = MAX_LONG;
    ULONG fontheaderoffset = MAX_LONG;

#if 0
    ULONG fontheadersize;

#endif
    ULONG charwidthoffset = MAX_LONG, charwidthsize;
    ULONG attributeheaderoffset = MAX_LONG, attributeheadersize;
    ULONG trackkernoffset = MAX_LONG, trackkernsize;
    ULONG textkernoffset = MAX_LONG, textkernsize;
    ULONG designkernoffset = MAX_LONG, designkernsize;
    ULONG typefaceheaderoffset = MAX_LONG, typefaceheadersize;
    ULONG compoundcharoffset = MAX_LONG, compoundcharsize;
    ULONG ccharmetricsoffset = MAX_LONG, ccharmetricssize;
    ULONG ccidoffset, ccidsize = MAX_LONG;
    ULONG complementheaderoffset = MAX_LONG, complementheadersize;
    ULONG fontaliasoffset = MAX_LONG, fontaliassize;

    UWORD numsegments;
    ULONG offset;
    union
    {
	BYTE *b;
	UWORD *w;
	LONG *l;
    } ptr;
    BYTE *tempptr;
    seg_dir_type *seg_dir;
    keyoffset_type *keyptr;
    faceglobal_type *faceglobal;
    compoundchar_type *compoundchar;
    complementheader_type *complementheader;
    ccharmetrics_type *ccharmetrics;
    ccid_type *ccid;

    UWORD NYLN, NGCD, NGYD, NGXD, NSDM, NFATS, NXREF;
    UWORD i, j, nkeys;

    /* get number of segments and offsets to segments */
    numsegments = 2;		/* always have rasterparam segment and end of dir */
    if (fl->globintdataoffset != MAX_LONG)
	numsegments++;

    /* displayheader is in display file */
    keyptr = (keyoffset_type *) (((fileheader_type *) dfileheader)->keyOffsets);
    for (i = 0; i < SWAPU (((fileheader_type *) dfileheader)->NKEYS); i++)
    {
	if (SWAPU (keyptr->key) == DISPLAYHEADER)
	{
	    displayheaderoffset = SWAPL (keyptr->offset);
	    numsegments++;
	    break;
	}
	keyptr++;
    }

    /* all the other segments are in the attribute file */
    /* read attribute file header */
    if (faisRead (fl, afileheader, MAX_HEADER, FALSE, fl->FONTATTRIBUTE))
	return (FAILURE);

    /* locate segments */
    keyptr = (keyoffset_type *) (((fileheader_type *) afileheader)->keyOffsets);
    nkeys = SWAPU (((fileheader_type *) afileheader)->NKEYS);
    if (nkeys > MAX_KEYS)
    {
	DBG2 ("attr file nkeys truncated from %ld to %ld\n", nkeys, MAX_KEYS);
	nkeys = MAX_KEYS;
    }
    for (i = 0; i < nkeys; i++)
    {
	DBG1 ("attribute key %ld\n", SWAPU (keyptr->key));
	switch (SWAPU (keyptr->key))
	{
	    case FONTHEADER:
		fontheaderoffset = SWAPL (keyptr->offset);
#if 0
		fontheadersize = SWAPL ((keyptr + 1)->offset) -
		  SWAPL (keyptr->offset);
#endif
		numsegments++;
		break;

	    case CHARWIDTH:
		charwidthoffset = SWAPL (keyptr->offset);
		charwidthsize = SWAPL ((keyptr + 1)->offset) - SWAPL (keyptr->offset);
		numsegments++;
		break;

	    case ATTRIBUTEHEADER:
		attributeheaderoffset = SWAPL (keyptr->offset);
		attributeheadersize = SWAPL ((keyptr + 1)->offset) - SWAPL (keyptr->offset);
		numsegments++;
		break;

	    case TRACKKERN:
		trackkernoffset = SWAPL (keyptr->offset);
		trackkernsize = SWAPL ((keyptr + 1)->offset) - SWAPL (keyptr->offset);
		numsegments++;
		break;

	    case TEXTKERN:
		textkernoffset = SWAPL (keyptr->offset);
		textkernsize = SWAPL ((keyptr + 1)->offset) - SWAPL (keyptr->offset);
		numsegments++;
		break;

	    case DESIGNKERN:
		designkernoffset = SWAPL (keyptr->offset);
		designkernsize = SWAPL ((keyptr + 1)->offset) - SWAPL (keyptr->offset);
		numsegments++;
		break;

	    case TYPEFACEHEADER:
		typefaceheaderoffset = SWAPL (keyptr->offset);
		typefaceheadersize = SWAPL ((keyptr + 1)->offset) - SWAPL (keyptr->offset);
		numsegments++;
		break;

	    case COMPOUNDCHAR:
		compoundcharoffset = SWAPL (keyptr->offset);
		compoundcharsize = SWAPL ((keyptr + 1)->offset) - SWAPL (keyptr->offset);
		numsegments++;
		break;

	    case CCHARMETRICS:
		ccharmetricsoffset = SWAPL (keyptr->offset);
		ccharmetricssize = SWAPL ((keyptr + 1)->offset) - SWAPL (keyptr->offset);
		DBG2 ("CCHARMETRICS $%lx %ld\n", ccharmetricsoffset,
		      ccharmetricssize);
		break;

	    case CCID:
		ccidoffset = SWAPL (keyptr->offset);
		ccidsize = SWAPL ((keyptr + 1)->offset) - SWAPL (keyptr->offset);
		DBG2 ("CCID $%lx %ld\n", ccidoffset, ccidsize);
		break;

	    case COMPLEMENTHEADER:
		complementheaderoffset = SWAPL (keyptr->offset);
		complementheadersize = SWAPL ((keyptr + 1)->offset) - SWAPL (keyptr->offset);
		break;

	    case FONTALIAS:
		fontaliasoffset = SWAPL (keyptr->offset);
		fontaliassize = SWAPL ((keyptr + 1)->offset) - SWAPL (keyptr->offset);
		numsegments++;
		break;

	}
	keyptr++;
    }

    if (fontheaderoffset == MAX_LONG)
    {
	DBG ("Font Header Segment not found.\n");
	ErrRequester (ERROR_IInternalFAIS, fl->fontName, ld_err_13);
	return (ld_err_13);
    }
    if (charwidthoffset == MAX_LONG)
    {
	DBG ("Character Width Segment not found.\n");
	ErrRequester (ERROR_IInternalFAIS, fl->fontName, ld_err_15);
	return (ld_err_15);
    }
    if (attributeheaderoffset == MAX_LONG)
    {
	DBG ("Attribute Header Segment not found.\n");
	ErrRequester (ERROR_IInternalFAIS, fl->fontName, ld_wrn_4);
	return (ld_wrn_4);
    }
    if (typefaceheaderoffset == MAX_LONG)
    {
	DBG ("Typeface Header Segment not found.\n");
	ErrRequester (ERROR_IInternalFAIS, fl->fontName, ld_wrn_8);
	return (ld_wrn_8);
    }
    if (displayheaderoffset == MAX_LONG)
    {
	DBG ("Display Header Segment not found.\n");
	ErrRequester (ERROR_IInternalFAIS, fl->fontName, ld_wrn_1);
	return (ld_wrn_1);
    }

    if (trackkernoffset == MAX_LONG)
	DBG ("Track Kerning Segment not found.\n");
    if (textkernoffset == MAX_LONG)
	DBG ("Text Kerning Segment not found.\n");
    if (designkernoffset == MAX_LONG)
	DBG ("Design Kerning Segment not found.\n");
    if (compoundcharoffset == MAX_LONG)
	DBG ("Compound Character Segment not found.\n");
    if (ccharmetricsoffset == MAX_LONG)
	DBG ("Compound Character Metrics Segment not found.\n");
    if (ccidoffset == MAX_LONG)
	DBG ("Compound Character ID Segment not found.\n");
    if (complementheaderoffset == MAX_LONG)
	DBG ("Complement Header Segment not found.\n");
    if (fontaliasoffset == MAX_LONG)
	DBG ("Font Alias Segment not found.\n");

    faceglobal = (faceglobal_type *) S.b.buffer;

    seg_dir = &faceglobal->seg_dir;

    offset = align (numsegments * (LONG) sizeof (seg_dir_type) + 6);

    ptr.b = (BYTE *) faceglobal + offset;
    if (offset + COPYRIGHTSIZE > MAX_BUFFER)
    {
	DBG ("Error buffer overflow.");
	ErrRequester (ERROR_IInternalFAIS, fl->fontName, ld_err_mem);
	return (ld_err_mem);
    }

    /* add copyright */
    seg_dir->key = copyright_key;
    seg_dir->size = COPYRIGHTSIZE;
    seg_dir->offset = offset;

    fseek (fl->FONTATTRIBUTE, fontheaderoffset, 0);
    if (faisRead (fl, ptr.b, COPYRIGHTSIZE, TRUE, fl->FONTATTRIBUTE))
	return (FAILURE);

    seg_dir++;
    offset += align ((ULONG) COPYRIGHTSIZE);
    ptr.b += align ((ULONG) COPYRIGHTSIZE);


    /* add global intellifont segment if it exists */
    if (fl->globintdataoffset != MAX_LONG)
    {

	if (offset + fl->globintdatasize > MAX_BUFFER)
	{
	    DBG ("Error buffer overflow.");
	    ErrRequester (ERROR_IInternalFAIS, fl->fontName, ld_err_mem);
	    return (ld_err_mem);
	}

	fseek (fl->FONTDISPLAY, fl->globintdataoffset, 0);
	tempptr = textkern;

	/* get actual size of global intellifont data */
	if (faisRead (fl, ptr.b, 2, TRUE, fl->FONTDISPLAY))
	    return (FAILURE);
	*ptr.w = NYLN = SWAPU (*ptr.w);
	ptr.w++;
	if (faisRead (fl, ptr.b, NYLN * 2, TRUE, fl->FONTDISPLAY))
	    return (FAILURE);
	for (i = 0; i < NYLN; i++)
	    ptr.w[i] = SWAPU (ptr.w[i]);
	ptr.b += NYLN * 2;

	if (faisRead (fl, ptr.b, 2, TRUE, fl->FONTDISPLAY))
	    return (FAILURE);
	*ptr.w = NGCD = SWAPU (*ptr.w);
	if (faisRead (fl, ptr.b + 2, NGCD, TRUE, fl->FONTDISPLAY))
	    return (FAILURE);
	if (faisRead (fl, tempptr, NGCD, TRUE, fl->FONTDISPLAY))
	    return (FAILURE);
	memcpy (ptr.b + 2 + NGCD, tempptr, NGCD);
	ptr.b += NGCD * 2 + 2;

	if (faisRead (fl, ptr.b, 2, TRUE, fl->FONTDISPLAY))
	    return (FAILURE);
	*ptr.w = NGYD = SWAPU (*ptr.w);
	if (faisRead (fl, ptr.b + 2, NGYD * 2, TRUE, fl->FONTDISPLAY))
	    return (FAILURE);
	for (i = 1; i <= NGYD; i++)
	    ptr.w[i] = SWAPU (ptr.w[i]);
	if (faisRead (fl, tempptr, (NGYD + 1) & 0xfffe, TRUE, fl->FONTDISPLAY))
	    return (FAILURE);
	memcpy (ptr.b + 2 + 2 * NGYD, tempptr, (NGYD + 1) & 0xfffe);
	ptr.b += NGYD * 3 + 2 + (NGYD & 1);

	if (faisRead (fl, ptr.b, 2, TRUE, fl->FONTDISPLAY))
	    return (FAILURE);
	*ptr.w = NGXD = SWAPU (*ptr.w);
	if (faisRead (fl, ptr.b + 2, NGXD * 2, TRUE, fl->FONTDISPLAY))
	    return (FAILURE);
	for (i = 1; i <= NGXD; i++)
	    ptr.w[i] = SWAPU (ptr.w[i]);

	if (faisRead (fl, tempptr, (NGXD + 1) & 0xfffe, TRUE, fl->FONTDISPLAY))
	    return (FAILURE);
	memcpy (ptr.b + 2 + 2 * NGXD, tempptr, (NGXD + 1) & 0xfffe);
	ptr.b += NGXD * 3 + 2 + (NGXD & 1);

	if (faisRead (fl, ptr.b, 2, TRUE, fl->FONTDISPLAY))
	    return (FAILURE);
	*ptr.w = NSDM = SWAPU (*ptr.w);
	/* NSDM is now used as orThreshold, assume NSDM <= 10 is an */
	/* old font and NSDM > 10 is a new font */
	if (NSDM <= 10)
	{
	    /* old style font, need to convert to new style */
	    *ptr.w = 210;
	    /* set orThreshold if no value was entered */
	    if (fl->orThreshold == 0)
		fl->orThreshold = 210;
	    if (NSDM > 0)
	    {
		if (faisRead (fl, ptr.b + 2, NSDM * 12, TRUE, fl->FONTDISPLAY))
		    return (FAILURE);
	    }
	}
	else
	{
	    /* set orThreshold if no value was entered */
	    if (fl->orThreshold == 0)
		fl->orThreshold = NSDM;
	}
	if (faisRead (fl, ptr.b + 2, 14 * 14, FALSE, fl->FONTDISPLAY))
	    return (FAILURE);
	ptr.w[1] = SWAPU (ptr.w[1]);
	ptr.w[2] = SWAPU (ptr.w[2]);
	ptr.w[3] = SWAPU (ptr.w[3]);

	seg_dir->key = globalintellifont_key;
	seg_dir->size = NYLN * 2 + NGCD * 2 + NGYD * 3 + NGXD * 3
	  + 24 + (NGYD & 1) + (NGXD & 1);
	seg_dir->offset = offset;

	offset += align ((ULONG) seg_dir->size);
	seg_dir++;
	ptr.b = S.b.buffer + offset;

    }

    /* read charwidth segment into table */

    if (offset + fl->nchar_in_font * sizeof (charwidth_type) + 6 > MAX_BUFFER)
    {
	DBG ("Error buffer overflow.");
	ErrRequester (ERROR_IInternalFAIS, fl->fontName, ld_err_mem);
	return (ld_err_mem);
    }

    *ptr.w = charwidth_key;

    fseek (fl->FONTATTRIBUTE, charwidthoffset, 0);
    if (faisRead (fl, ptr.b + 6, (UWORD) charwidthsize, TRUE, fl->FONTATTRIBUTE))
	return (FAILURE);
    swapcharwidth (fl, ptr.b + 6);


    /* need to read in textkern and designkern segments and */
    /* sort them with the face header to insure that the */
    /* characters are in ascending order of cgnumber and that */
    /* the charwiths and kerning data stays in the same order. */

    if (textkernoffset != MAX_LONG)
    {
	fseek (fl->FONTATTRIBUTE, textkernoffset, 0);
	if (faisRead (fl, textkern, (UWORD) textkernsize, TRUE,
		      fl->FONTATTRIBUTE))
	    return (FAILURE);
    }

    /* pack kerning from words into nibbles */
    pack_kerning (fl, (packedkern_type *) textkern, (unpackedkern_type *) textkern);

    if (designkernoffset != MAX_LONG)
    {
	fseek (fl->FONTATTRIBUTE, designkernoffset, 0);
	if (faisRead (fl, designkern, (UWORD) designkernsize, TRUE,
		      fl->FONTATTRIBUTE))
	    return (FAILURE);
    }

    /* pack kerning from words into nibbles */
    pack_kerning (fl, (packedkern_type *) designkern, (unpackedkern_type *) designkern);

    seg_dir->key = charwidth_key;
    seg_dir->offset = offset;

    sort (fl, (charwidth_type *) (ptr.b + 6), (packedkern_type *) textkern,
	  (packedkern_type *) designkern);

    /* sort determines nchar_in_lib */
    seg_dir->size = fl->nchar_in_lib * sizeof (charwidth_type) + 6;
    *((LONG *) (ptr.b + 2)) = (LONG) (fl->nchar_in_lib * sizeof (charwidth_type) + 6);
    offset += align ((ULONG) seg_dir->size);
    seg_dir++;
    ptr.b = S.b.buffer + offset;


    /* add raster parameter segment */

    if (offset + RASTERPARAMSIZE > MAX_BUFFER)
    {
	DBG ("Error buffer overflow.");
	ErrRequester (ERROR_IInternalFAIS, fl->fontName, ld_err_mem);
	return (ld_err_mem);
    }

    seg_dir->offset = offset;
    *ptr.w++ = seg_dir->key = rasterparam_key;
    *ptr.l++ = seg_dir->size = (LONG) RASTERPARAMSIZE;
    *ptr.w++ = fl->orThreshold;
    *ptr.w++ = fl->fontSlant;

    offset += align ((ULONG) seg_dir->size);
    seg_dir++;
    ptr.b = S.b.buffer + offset;

    /* add attribute header */

    if (offset + attributeheadersize + 6 > MAX_BUFFER)
    {
	DBG ("Error buffer overflow.");
	ErrRequester (ERROR_IInternalFAIS, fl->fontName, ld_err_mem);
	return (ld_err_mem);
    }

    *ptr.w++ = seg_dir->key = attributeheader_key;
    *(ptr.l)++ = seg_dir->size = sizeof (attributeheader_type) + 6;
    seg_dir->offset = offset;

    fseek (fl->FONTATTRIBUTE, attributeheaderoffset, 0);
    if (faisRead (fl, ptr.b, seg_dir->size, TRUE, fl->FONTATTRIBUTE))
	return (FAILURE);
    swapattributeheader (fl, ptr.b);

    offset += align ((ULONG) seg_dir->size);
    seg_dir++;
    ptr.b = S.b.buffer + offset;

    /* add track kern if present */
    if (trackkernoffset != MAX_LONG)
    {

	if (offset + trackkernsize + 6 > MAX_BUFFER)
	{
	    DBG ("Error buffer overflow.");
	    ErrRequester (ERROR_IInternalFAIS, fl->fontName, ld_err_mem);
	    return (ld_err_mem);
	}

	seg_dir->offset = offset;
	*ptr.w++ = seg_dir->key = trackkern_key;

	fseek (fl->FONTATTRIBUTE, trackkernoffset, 0);
	if (faisRead (fl, ptr.b + 4, (UWORD) trackkernsize, TRUE,
		      fl->FONTATTRIBUTE))
	    return (FAILURE);
	swaptrackkern (ptr.b + 4);

	*ptr.l = seg_dir->size = *ptr.w * 10 + 8;

	offset += align ((ULONG) seg_dir->size);
	seg_dir++;
	ptr.b = S.b.buffer + offset;
    }

    /* add text kern */
    if (textkernoffset != MAX_LONG)
    {

	if (offset + fl->nchar_in_lib * 4 + 6 + 6 > MAX_BUFFER)
	{
	    DBG ("Error buffer overflow.");
	    ErrRequester (ERROR_IInternalFAIS, fl->fontName, ld_err_mem);
	    return (ld_err_mem);
	}

	*ptr.w++ = seg_dir->key = textkern_key;
	*ptr.l++ = seg_dir->size = fl->nchar_in_lib * 4 + 6 + 6;
	seg_dir->offset = offset;

	/* kerning has already been sorted and packed copy it into segment */
	memcpy (ptr.b, textkern, seg_dir->size);

	offset += align ((ULONG) seg_dir->size);
	seg_dir++;
	ptr.b = S.b.buffer + offset;

    }

    /* add design kern if present */
    if (designkernoffset != MAX_LONG)
    {

	if (offset + fl->nchar_in_lib * 4 + 6 + 6 > MAX_BUFFER)
	{
	    DBG ("Error buffer overflow.");
	    ErrRequester (ERROR_IInternalFAIS, fl->fontName, ld_err_mem);
	    return (ld_err_mem);
	}

	*ptr.w++ = seg_dir->key = designkern_key;
	*ptr.l++ = seg_dir->size = fl->nchar_in_lib * 4 + 6 + 6;
	seg_dir->offset = offset;

	/* kerning has already been sorted and packed copy it into segment */
	memcpy (ptr.b, designkern, seg_dir->size);

	offset += align ((ULONG) seg_dir->size);
	seg_dir++;
	ptr.b = S.b.buffer + offset;

    }

    /* add typeface header */

    if (offset + typefaceheadersize + 6 > MAX_BUFFER)
    {
	DBG ("Error buffer overflow.");
	ErrRequester (ERROR_IInternalFAIS, fl->fontName, ld_err_mem);
	return (ld_err_mem);
    }

    *ptr.w++ = seg_dir->key = typefaceheader_key;
    seg_dir->offset = offset;

    fseek (fl->FONTATTRIBUTE, typefaceheaderoffset, 0);
    if (faisRead (fl, ptr.b + 4, (UWORD) typefaceheadersize, TRUE,
		  fl->FONTATTRIBUTE))
    {
	DBG ("typefaceheader read failure\n");
	return (FAILURE);
    }
    swaptypefaceheader (ptr.b + 4);

    *ptr.l = seg_dir->size = *(UWORD *) (ptr.b + 4) * 12 + 92 + 6;

    offset += align ((ULONG) seg_dir->size);
    seg_dir++;
    ptr.b = S.b.buffer + offset;

    /* get compound character segment */
    /* need to assemble it from many parts */

    DBG5 ("compound character parts $%lx $%lx $%lx $%lx ?= $%lx\n",
	  compoundcharoffset, ccharmetricsoffset, ccidoffset,
	  complementheaderoffset, MAX_LONG);
    if (compoundcharoffset != MAX_LONG &&
	ccharmetricsoffset != MAX_LONG &&
	ccidoffset != MAX_LONG &&
	complementheaderoffset != MAX_LONG)
    {

	/* estimate size of final cchar buffer as 1.25 * cchar size in fais */
	if (offset + compoundcharsize + (compoundcharsize >> 2) > MAX_BUFFER)
	{
	    DBG ("Error buffer overflow.");
	    ErrRequester (ERROR_IInternalFAIS, fl->fontName, ld_err_mem);
	    return (ld_err_mem);
	}

	/* reuse kerning buffers */
	if (compoundcharsize > 16 * MAX_CHARS + 6)
	{
	    DBG (" Error compound char Segment too big.\n");
	    ErrRequester (ERROR_IInternalFAIS, fl->fontName, ld_err_mem);
	    return (ld_err_mem);
	}

	if (complementheadersize + ccharmetricssize + ccidsize
	    > 16 * MAX_CHARS + 6)
	{
	    DBG (" Error compound char Segment too big.\n");
	    ErrRequester (ERROR_IInternalFAIS, fl->fontName, ld_err_mem);
	    return (ld_err_mem);
	}

	compoundchar = (compoundchar_type *) textkern;
	complementheader = (complementheader_type *) designkern;
	ccharmetrics = (ccharmetrics_type *) ((BYTE *) designkern
					      + complementheadersize);
	ccid = (ccid_type *) ((BYTE *) ccharmetrics + ccharmetricssize);

	fseek (fl->FONTATTRIBUTE, compoundcharoffset, 0);
	if (faisRead (fl, (char *) compoundchar, (UWORD) compoundcharsize, TRUE,
		      fl->FONTATTRIBUTE))
	{
	    DBG ("compoundchar read failure\n");
	    return (FAILURE);
	}
	fseek (fl->FONTATTRIBUTE, complementheaderoffset, 0);
	if (faisRead (fl, (char *) complementheader,
		      (UWORD) complementheadersize, TRUE, fl->FONTATTRIBUTE))
	{
	    DBG ("complementheader read failure\n");
	    return (FAILURE);
	}

	fseek (fl->FONTATTRIBUTE, ccharmetricsoffset, 0);
	if (faisRead (fl, (char *) ccharmetrics, (UWORD) ccharmetricssize, TRUE,
		      fl->FONTATTRIBUTE))
	{
	    DBG ("ccharmetrics read failure\n");
	    return (FAILURE);
	}

	fseek (fl->FONTATTRIBUTE, ccidoffset, 0);
	if (faisRead (fl, (char *) ccid, (UWORD) ccidsize, TRUE,
		      fl->FONTATTRIBUTE))
	{
	    DBG ("ccid read failure\n");
	    return (FAILURE);
	}

	seg_dir->key = *ptr.w++ = compoundchar_key;
	seg_dir->offset = offset;
	seg_dir->size = (UWORD) (*ptr.l = 6 +
				 assemblecompoundchar (fl, (WORD *) (ptr.b + 4),
						       (WORD *) compoundchar, complementheader, ccharmetrics, ccid));

	offset += align ((ULONG) seg_dir->size);
	seg_dir++;
	ptr.b = S.b.buffer + offset;
    }

    /* add display header */
    if (offset + sizeof (displayheader_type) + 6 > MAX_BUFFER)
    {
	DBG ("Error buffer overflow.");
	ErrRequester (ERROR_IInternalFAIS, fl->fontName, ld_err_mem);
	return (ld_err_mem);
    }

    *ptr.w++ = seg_dir->key = displayheader_key;
    *ptr.l++ = seg_dir->size = sizeof (displayheader_type) + 6;
    seg_dir->offset = offset;

    fseek (fl->FONTDISPLAY, displayheaderoffset, 0);
    if (faisRead (fl, ptr.b, seg_dir->size, TRUE, fl->FONTDISPLAY))
	return (FAILURE);
    swapdisplayheader (ptr.b);


    offset += align ((ULONG) seg_dir->size);
    seg_dir++;
    ptr.b = S.b.buffer + offset;

    /* add font alias table */
    if (fontaliasoffset != MAX_LONG)
    {

	if (offset + fontaliassize + 6 > MAX_BUFFER)
	{
	    DBG ("Error buffer overflow.");
	    ErrRequester (ERROR_IInternalFAIS, fl->fontName, ld_err_mem);
	    return (ld_err_mem);
	}

	seg_dir->key = fontalias_key;
	seg_dir->offset = offset;
	ptr.b += 6;

	fseek (fl->FONTATTRIBUTE, fontaliasoffset, 0);
	if (faisRead (fl, ptr.b, (UWORD) fontaliassize, TRUE, fl->FONTATTRIBUTE))
	    return (FAILURE);

	seg_dir->size = 6;



	/* count length of segment */
	*ptr.w = NFATS = SWAPU (*ptr.w);
	seg_dir->size += 2;
	ptr.b += 2;

	for (j = 0; j < NFATS; j++)
	{
	    *ptr.w = NXREF = SWAPU (*(UWORD *) (ptr.b + j * 104 + 20));
	    for (i = 0; i < NFATS; i++)
		*(LONG *) (ptr.b + j * 104 + 122) =
		  SWAPL (*(LONG *) (ptr.b + j * 104 + 122));
	    seg_dir->size += NXREF * 104 + 22;
	    ptr.b += NXREF * 104 + 22;
	}

	/* reset pointer to begining of fontalias segment */
	ptr.b = S.b.buffer + offset;
	seg_dir->key = *ptr.w++ = fontalias_key;
	*(ULONG *) ptr.b = seg_dir->size;
	seg_dir->offset = offset;

	/* offset is the size of the face global segment */
	offset += align ((ULONG) seg_dir->size);


    }
    seg_dir++;
    seg_dir->key = endofdir_key;
    seg_dir->size = 0;
    seg_dir->offset = (LONG) 0;


    /* face global immediately follows faceheader */
    ((faceheader_type *) faceheader)->faceglobal_size = (UWORD) offset;
    fl->faceheaderoffset = ftell (fl->LIB);
    fl->faceheadersize = (UWORD) align ((ULONG) fl->faceheadersize);
    ((faceheader_type *) faceheader)->faceglobal_offset =
      fl->faceheaderoffset + (LONG) fl->faceheadersize;

    /* rewrite size of face header since it may have changed */
    fseek (fl->LIB, fl->faceheadersizeoffset, 0);
    if (faisWrite (fl, (char *) &fl->faceheadersize, 2))
	return (FAILURE);
    fseek (fl->LIB, fl->faceheaderoffset, 0);

    /* write face header */
    if (faisWrite (fl, faceheader, fl->faceheadersize))
	return (FAILURE);

    /* set face global key & size */
    faceglobal->key = faceglobal_key;
    faceglobal->size = offset;

    /* write face global */
    if (faisWrite (fl, S.b.buffer, (UWORD) offset))
	return (FAILURE);

    return (OK);

}

WORD
write_characters (fl)
    struct FaisLocals *fl;
{

    char_type *charptr;
    UWORD i, charsize;

    charptr = ((faceheader_type *) faceheader)->chars;

    DBG1 ("write_characters ... %ld\n", fl->nchar_in_lib);
    for (i = 0; i < fl->nchar_in_lib; i++)
    {
	if ((charptr->offset & 0xe0000000) == normask)
	{
	    fseek (fl->FONTDISPLAY, charptr->offset + fl->hiqdataoffset, 0);
	    if (faisRead (fl, S.b.buffer, charptr->size, TRUE,
			  fl->FONTDISPLAY))
	    {
		DBG ("Error reading characters.\n");
		return (FAILURE);
	    }
	}
	else
	{
	    DBG1 ("Error: Bad Mask $%lx\n", charptr->offset);
	    return (-1);
	}

	swapcharacters (S.b.buffer);

	*(UWORD *) S.b.buffer = charptr->size;

	charptr->offset = ftell (fl->LIB);
	charsize = (UWORD) align ((LONG) charptr->size);

	if (faisWrite (fl, S.b.buffer, charsize))
	    return (FAILURE);

	charptr++;
    }

    /* rewrite face header with new offsets */
    fseek (fl->LIB, fl->faceheaderoffset, 0);
    if (faisWrite (fl, faceheader, fl->faceheadersize))
	return (FAILURE);

    return (SUCCESS);
}


/*
**  return n aligned to next mod boundary
*/
ULONG
align (n)
    ULONG n;
{
    ULONG x;

    if (n % DISK_ALIGN == 0)
	x = n;
    else
	x = (n + DISK_ALIGN - n % DISK_ALIGN);
    return (x);
}

void
 sort (fl, charwidth, textkernptr, designkernptr)
    struct FaisLocals *fl;
    charwidth_type *charwidth;
    packedkern_type *textkernptr;
    packedkern_type *designkernptr;
{
    char_type *charptr, *lowcharptr, *swpcharptr;
    kerndata_type *textptr, *lowtextptr, *swptextptr;
    kerndata_type *designptr, *lowdesignptr, *swpdesignptr;
    charwidth_type *charwidthptr, *lowcharwidthptr, *swpcharwidthptr;
    charwidth_type charwidthbuf;
    char_type charbuf;
    UWORD i, j, k;
    UBYTE temp, *ptr1, *ptr2;
    UWORD tot_chars, num_chars;


    /* sort on cg_num in faceheader in ascending order */
    swpcharptr = ((faceheader_type *) faceheader)->chars;
    swptextptr = textkernptr->character;
    swpdesignptr = designkernptr->character;
    swpcharwidthptr = charwidth;

    tot_chars = fl->nchar_in_font;
    num_chars = fl->nchar_in_lib;

    for (i = 0; i < tot_chars - 1; i++)
    {
/*    if(i > num_chars) break; */
	lowcharptr = swpcharptr;
	lowtextptr = swptextptr;
	lowdesignptr = swpdesignptr;
	lowcharwidthptr = swpcharwidthptr;

	charptr = lowcharptr + 1;
	textptr = lowtextptr + 1;
	designptr = lowdesignptr + 1;
	charwidthptr = lowcharwidthptr + 1;

	for (j = i + 1; j < tot_chars; j++)
	{
	    if (charptr->cg_num < lowcharptr->cg_num)
	    {
		lowcharptr = charptr;
		lowtextptr = textptr;
		lowdesignptr = designptr;
		lowcharwidthptr = charwidthptr;

	    }
	    else if (charptr->cg_num == lowcharptr->cg_num)
	    {
		if (charptr->cg_num != MAX_UWORD)
		{
		    num_chars--;
		}
		lowcharptr->cg_num = MAX_UWORD;
		lowcharptr = charptr;
		lowtextptr = textptr;
		lowdesignptr = designptr;
		lowcharwidthptr = charwidthptr;
	    }

	    charptr++;
	    charwidthptr++;
	    textptr++;
	    designptr++;
	}

	if (lowcharptr != swpcharptr)
	{
	    /* need to swap all four blocks */
	    charbuf.cg_num = swpcharptr->cg_num;
	    charbuf.offset = swpcharptr->offset;
	    charbuf.size = swpcharptr->size;
	    swpcharptr->cg_num = lowcharptr->cg_num;
	    swpcharptr->offset = lowcharptr->offset;
	    swpcharptr->size = lowcharptr->size;
	    lowcharptr->cg_num = charbuf.cg_num;
	    lowcharptr->offset = charbuf.offset;
	    lowcharptr->size = charbuf.size;


	    charwidthbuf.charWidth = swpcharwidthptr->charWidth;
	    charwidthbuf.charFlags = swpcharwidthptr->charFlags;
	    swpcharwidthptr->charWidth = lowcharwidthptr->charWidth;
	    swpcharwidthptr->charFlags = lowcharwidthptr->charFlags;
	    lowcharwidthptr->charWidth = charwidthbuf.charWidth;
	    lowcharwidthptr->charFlags = charwidthbuf.charFlags;

	    ptr1 = (UBYTE *) swptextptr;
	    ptr2 = (UBYTE *) lowtextptr;
	    for (k = 0; k < 4; k++)
	    {
		temp = *ptr1;
		*ptr1++ = *ptr2;
		*ptr2++ = temp;
	    }

	    ptr1 = (UBYTE *) swpdesignptr;
	    ptr2 = (UBYTE *) lowdesignptr;
	    for (k = 0; k < 4; k++)
	    {
		temp = *ptr1;
		*ptr1++ = *ptr2;
		*ptr2++ = temp;
	    }
	}



	swpcharptr++;
	swpcharwidthptr++;
	swptextptr++;
	swpdesignptr++;
    }
    /* set end of character */
    fl->nchar_in_lib = num_chars;
    fl->faceheadersize = 6 + 8 * (fl->nchar_in_lib + 1);
    charptr = (char_type *) ((BYTE *) (((faceheader_type *) faceheader)->chars)
			     + fl->nchar_in_lib * sizeof (char_type));
    charptr->cg_num = endofdir_key;
}

void
 pack_kerning (fl, packed, unpacked)
    struct FaisLocals *fl;
    packedkern_type *packed;
    unpackedkern_type *unpacked;
{
    UWORD i, j, k, m1, m2;

    /* copy header information */
    packed->kernSign = SWAPW (unpacked->kernSign);
    packed->kernUnit = SWAPU (unpacked->kernUnit);
    packed->NSECT = SWAPU (unpacked->NSECT);

    for (i = 0; i < fl->nchar_in_lib; i++)
    {
	for (j = k = 0; j < 8; j += 2, k++)
	{
	    m1 = (SWAPU (unpacked->character[i].data[j]) << 4) & 0x00F0;
	    m2 = (SWAPU (unpacked->character[i].data[j + 1])) & 0x000F;
	    m1 = m1 + m2;
	    packed->character[i].data[k] = (UBYTE) m1;
	}
    }
}

WORD
assemblecompoundchar (fl, libcompoundchar, compoundchar, complementheader,
		      ccharmetrics, ccid)
    struct FaisLocals *fl;
    WORD *libcompoundchar;
    WORD *compoundchar;
    complementheader_type *complementheader;
    ccharmetrics_type *ccharmetrics;
    ccid_type *ccid;
{
    libcompoundchar_type *libccharptr;
    compoundchar_type *ccharptr;
    ccharmetrics_type *ccmetricsptr;
    ccid_type *ccidptr;
    isocodetable_type *tableptr;
    libcompoundchar_type *ptr, *nextptr, *tempptr;
    part_type *partptr;
    libpart_type *libpartptr, *nextpartptr, *tpartptr;
    BYTE tempbuffer[68];
    UWORD compoundcharsize, tempsize;
    UWORD i, j, k;
    UWORD NCCHAR, NCCHAR2;
    UWORD numparts;

    /* first word of compoundchar and libcompoundchar is cchar count */
    NCCHAR = *libcompoundchar = SWAPU (*compoundchar);

    libccharptr = (libcompoundchar_type *) (libcompoundchar + 1);
    ccharptr = (compoundchar_type *) (compoundchar + 1);
    ccmetricsptr = (ccharmetrics_type *) ccharmetrics;
    ccidptr = (ccid_type *) ccid;
    tempptr = (libcompoundchar_type *) tempbuffer;
    tableptr = complementheader->table;
    compoundcharsize = 2;	/* include the character count */

    DBG1 ("assemblecompoundchar %ld\n", NCCHAR);
    /* copy entries into libcompoundchar format */
    for (i = 0; i < NCCHAR; i++)
    {
	DBG1 ("cc id %ld\n", SWAPU (ccidptr->cg_num));
	libccharptr->cg_num = SWAPU (ccidptr->cg_num);
	libccharptr->horiz_esc = SWAPU (ccmetricsptr->escapementX);
	libccharptr->vert_esc = SWAPU (ccmetricsptr->escapementY);
	libccharptr->NPCC = SWAPU (ccharptr->NPCC);
	compoundcharsize += 8 + sizeof (libpart_type) * SWAPU (ccharptr->NPCC);
	partptr = ccharptr->parts;
	libpartptr = libccharptr->parts;

	for (j = 0; j < SWAPU (ccharptr->NPCC); j++)
	{
	    libpartptr->cg_num = code_to_cgnum (fl, SWAPU (partptr->ccCharCode),
						tableptr);
	    libpartptr->xoffset = SWAPU (partptr->offsets.x);
	    libpartptr->yoffset = SWAPU (partptr->offsets.y);
	    partptr++;
	    libpartptr++;
	}

	libccharptr = (libcompoundchar_type *) ((BYTE *) libccharptr +
						8 + sizeof (libpart_type) * libccharptr->NPCC);
	ccharptr = (compoundchar_type *) ((BYTE *) ccharptr +
					  4 + sizeof (part_type) * SWAPU (ccharptr->NPCC));
	ccmetricsptr++;
	ccidptr++;
    }

    /* add the update compound chars (plugins are not allowed cc's) */
    if (fl->updcompoundcharptr)
    {
	NCCHAR2 = *(WORD *) fl->updcompoundcharptr;
	ptr = (libcompoundchar_type *) ((WORD *) fl->updcompoundcharptr + 1);
	for (i = 0; i < NCCHAR2; i++)
	{
	    ptr->NPCC |= 0x8000;
	    ptr = (libcompoundchar_type *) ((BYTE *) ptr +
					    8 + sizeof (libpart_type) * (ptr->NPCC & 0x7fff));
	}
	tempsize = (BYTE *) ptr - (BYTE *) fl->updcompoundcharptr - 2;
	memcpy ((BYTE *) libccharptr, (BYTE *) fl->updcompoundcharptr + 2, tempsize);
	compoundcharsize += tempsize;
#if 0
	libccharptr = (libcompoundchar_type *) ((BYTE *) libccharptr + tempsize);
#endif
	NCCHAR += NCCHAR2;
    }


    /* sort the libcompoundchar structure by increasing cg numbers */
    for (i = 0; i < NCCHAR - 1; i++)
    {

	ptr = (libcompoundchar_type *) (libcompoundchar + 1);

	for (j = 0; j < NCCHAR - 1; j++)
	{

	    nextptr = (libcompoundchar_type *) ((BYTE *) ptr +
						8 + sizeof (libpart_type) * ptr->NPCC);

	    if (ptr->cg_num == nextptr->cg_num)
	    {
		NCCHAR--;
		nextptr->cg_num = MAX_UWORD;
		compoundcharsize -= 8 + sizeof (libpart_type) *
		  (nextptr->NPCC & 0x7fff);
	    }

	    if (ptr->cg_num > nextptr->cg_num)
	    {

		tempptr->cg_num = ptr->cg_num;
		tempptr->horiz_esc = ptr->horiz_esc;
		tempptr->vert_esc = ptr->vert_esc;
		tempptr->NPCC = ptr->NPCC;

		tpartptr = tempptr->parts;
		libpartptr = ptr->parts;
		for (k = 0; k < (ptr->NPCC & 0x7fff); k++)
		{
		    tpartptr->cg_num = libpartptr->cg_num;
		    tpartptr->xoffset = libpartptr->xoffset;
		    tpartptr->yoffset = libpartptr->yoffset;
		    tpartptr++;
		    libpartptr++;
		}

		ptr->cg_num = nextptr->cg_num;
		ptr->horiz_esc = nextptr->horiz_esc;
		ptr->vert_esc = nextptr->vert_esc;
		ptr->NPCC = nextptr->NPCC;

		libpartptr = ptr->parts;
		nextpartptr = nextptr->parts;
		numparts = nextptr->NPCC & 0x7fff;
		for (k = 0; k < numparts; k++)
		{
		    libpartptr->cg_num = nextpartptr->cg_num;
		    libpartptr->xoffset = nextpartptr->xoffset;
		    libpartptr->yoffset = nextpartptr->yoffset;
		    libpartptr++;
		    nextpartptr++;
		}

		nextptr = (libcompoundchar_type *) ((BYTE *) ptr +
						    8 + sizeof (libpart_type) * ptr->NPCC);

		nextptr->cg_num = tempptr->cg_num;
		nextptr->horiz_esc = tempptr->horiz_esc;
		nextptr->vert_esc = tempptr->vert_esc;
		nextptr->NPCC = tempptr->NPCC;

		libpartptr = nextptr->parts;
		tpartptr = tempptr->parts;
		for (k = 0; k < (tempptr->NPCC & 0x7fff); k++)
		{
		    libpartptr->cg_num = tpartptr->cg_num;
		    libpartptr->xoffset = tpartptr->xoffset;
		    libpartptr->yoffset = tpartptr->yoffset;
		    libpartptr++;
		    tpartptr++;
		}
	    }
	    ptr = nextptr;
	}
    }
    *libcompoundchar = NCCHAR;
    ptr = (libcompoundchar_type *) ((WORD *) libcompoundchar + 1);
    for (i = 0; i < NCCHAR; i++)
    {
	ptr->NPCC &= 0x7fff;
	ptr = (libcompoundchar_type *) ((BYTE *) ptr +
					8 + sizeof (libpart_type) * ptr->NPCC);
    }
    return ((WORD) compoundcharsize);
}

WORD
code_to_cgnum (fl, code, table)
    struct FaisLocals *fl;
    UWORD code;
    isocodetable_type *table;
{
    WORD i;
    isocodetable_type *tableptr;

    tableptr = table;

    for (i = 0; i < fl->nchar_in_font; i++)
    {
	if (SWAPU (tableptr->isoCode) == code)
	    return ((WORD) SWAPU (tableptr->cgCode));
	tableptr++;
    }
    return ((UWORD) FALSE);
}


void
 cleanup (fl)
    struct FaisLocals *fl;
{

    /* close files */
    if (fl->LIB)
	fclose (fl->LIB);
    if (fl->FONTATTRIBUTE)
	fclose (fl->FONTATTRIBUTE);
    if (fl->FONTDISPLAY)
	fclose (fl->FONTDISPLAY);
}

void
 swapcharwidth (fl, ptr)
    struct FaisLocals *fl;
    BYTE *ptr;
{

    charwidth_type *cptr;
    UWORD i;

    cptr = (charwidth_type *) ptr;

    for (i = 0; i < fl->nchar_in_font; i++)
    {
	cptr->charWidth = SWAPU (cptr->charWidth);
	cptr->charFlags = SWAPU (cptr->charFlags);
	cptr++;
    }
}

void
 swapdisplayheader (ptr)
    BYTE *ptr;
{

    displayheader_type *dptr;
    UWORD i;

    dptr = (displayheader_type *) ptr;

    dptr->NCHAR = SWAPU (dptr->NCHAR);
    for (i = 0; i < 4; i++)
    {
	dptr->fontLimits[i] = SWAPU (dptr->fontLimits[i]);
	dptr->reverseVideoLimits[i] = SWAPU (dptr->reverseVideoLimits[i]);
    }
    dptr->leftReference = SWAPU (dptr->leftReference);
    dptr->baselinePosition = SWAPU (dptr->baselinePosition);
    dptr->minimumPointSize = SWAPU (dptr->minimumPointSize);
    dptr->maximumPointSize = SWAPU (dptr->maximumPointSize);
    dptr->minimumSetSize = SWAPU (dptr->minimumSetSize);
    dptr->maximumSetSize = SWAPU (dptr->maximumSetSize);
    dptr->masterPointSize = SWAPU (dptr->masterPointSize);
    dptr->scanDirection = SWAPU (dptr->scanDirection);
    dptr->italicAngle = SWAPW (dptr->italicAngle);
    dptr->xHeight = SWAPW (dptr->xHeight);
    dptr->scanResolutionY = SWAPU (dptr->scanResolutionY);
    dptr->scanResolutionX = SWAPU (dptr->scanResolutionX);
    dptr->outputEnable = SWAPU (dptr->outputEnable);
}

void
 swapattributeheader (fl, ptr)
    struct FaisLocals *fl;
    BYTE *ptr;
{
    attributeheader_type *aptr;
    UWORD i;

    aptr = (attributeheader_type *) ptr;

    aptr->scaleFactor = SWAPU (aptr->scaleFactor);
    for (i = 0; i < 3; i++)
	aptr->fixedSpaceRelWidths[i] = SWAPU (aptr->fixedSpaceRelWidths[i]);
    aptr->leftReference = SWAPU (aptr->leftReference);
    aptr->baselinePosition = SWAPU (aptr->baselinePosition);
    aptr->windowTop = SWAPW (aptr->windowTop);
    aptr->windowBottom = SWAPW (aptr->windowBottom);
    for (i = 0; i < 3; i++)
    {
	aptr->autoVarComp[i].zeroPoint = SWAPW (aptr->autoVarComp[i].zeroPoint);
	aptr->autoVarComp[i].variablePoint =
	  SWAPW (aptr->autoVarComp[i].variablePoint);
    }
    aptr->ascender = SWAPW (aptr->ascender);
    aptr->descender = SWAPW (aptr->descender);
    aptr->capHeight = SWAPW (aptr->capHeight);
    aptr->xHeight = SWAPW (aptr->xHeight);
    aptr->lcAccenHeight = SWAPW (aptr->lcAccenHeight);
    aptr->ucAccentHeight = SWAPW (aptr->ucAccentHeight);
    aptr->charPica = SWAPU (aptr->charPica);
    aptr->leftAlign = SWAPW (aptr->leftAlign);
    aptr->rightAlign = SWAPW (aptr->rightAlign);
    aptr->uscoreDepth = SWAPW (aptr->uscoreDepth);
    aptr->uscoreThickness = SWAPU (aptr->uscoreThickness);
    aptr->windowLeft = SWAPW (aptr->windowLeft);
    aptr->windowRight = SWAPW (aptr->windowRight);
    aptr->spaceBand = SWAPU (aptr->spaceBand);
    fl->ySizeNumerator = aptr->scaleFactor;
    fl->ySizeDenominator = aptr->ascender - aptr->descender;
    fl->spaceBand = aptr->spaceBand;
    fl->isFixed = aptr->isFixedPitch;
}

void
 swaptypefaceheader (ptr)
    BYTE *ptr;
{

    typefaceheader_type *tptr;
    UWORD i;

    tptr = (typefaceheader_type *) ptr;

    tptr->NFACES = SWAPU (tptr->NFACES);
    for (i = 0; i < tptr->NFACES; i++)
	tptr->typeFaceSet[0] = SWAPL (tptr->typeFaceSet[0]);
}

void
 swaptrackkern (ptr)
    BYTE *ptr;
{

    WORD *wptr;
    UWORD i, num;

    wptr = (WORD *) ptr;

    num = *wptr++ = SWAPU (*(UWORD *) wptr);

    for (i = 0; i < num; i++)
    {
	*wptr++ = SWAPW (*wptr);
	*wptr++ = SWAPU (*(UWORD *) wptr);
	*wptr++ = SWAPU (*(UWORD *) wptr);
	*wptr++ = SWAPW (*wptr);
	*wptr++ = SWAPW (*wptr);
    }
}

void
 swapcharacters (srcB)
    BYTE *srcB;
{
    union PointerUnion src, ptr, ptr2;
    void *hold;
    UWORD i, j;
    UWORD NLOP, NLYLN, NPNT;
    UBYTE NYSK, NYND, NYDM, NYPR, NYIN, NYLC;
    UBYTE NYCH, NYCA, NYTR, NLYCD, NXSK, NXND, NXDM;
    UBYTE NXPR, NXIN, NXLC;

    src.b = srcB;
    ptr.b = src.b;

    for (i = 0; i < 5; i++)
    {
	*ptr.w = SWAPU (*ptr.w);
	ptr.w++;
    }

    /* swap metric data */
    if (src.w[1] > 0)
    {
	ptr.w = (WORD *) (src.b + (UWORD) src.w[1]);
	for (i = 0; i < 10; i++)
	{
	    *ptr.w = SWAPW (*ptr.w);
	    ptr.w++;
	}
    }

    /* swap first two words of contour tree */
    if (src.w[3] > 0)
    {
	ptr.w = (UWORD *) (src.b + (UWORD) src.w[3]);
	*ptr.w = NLOP = SWAPU (*ptr.w);
	ptr.w++;
	*ptr.w = SWAPU (*ptr.w);

	if (src.w[2] > 0)
	{
	    hold = ptr.b;

	    if (*ptr.w & HQ2FLAG)
	    {
		DBG ("HQ2 data!\n");
		/* swap Intellifont data for hq2 */
		ptr.b = src.b + (UWORD) src.w[2];
		NYSK = *ptr.b++;
		ptr.b += NLOP;
		ptr.b += (((LONG) ptr.b) & 1);
		for (i = 0; i < NYSK; i++)
		{
		    *ptr.w = SWAPU (*ptr.w);
		    ptr.w++;
		}
		NYTR = *ptr.b++;
		NYND = *ptr.b++;
		ptr.b += NYND * 3;
		NYDM = *ptr.b++;
		ptr.b += NYDM;
		NYPR = *ptr.b++;
		ptr.b += NYPR * 3;
		NYIN = *ptr.b++;
		ptr.b += NYIN;
		NYLC = *ptr.b++;
		ptr.b += (((LONG) ptr.b) & 1);
		for (i = 0; i < NYLC; i++)
		{
		    *ptr.w = SWAPU (*ptr.w);
		    ptr.w++;
		}
		ptr.b += NYLC;
		ptr.b += (((LONG) ptr.b) & 1);
		*ptr.w = SWAPU (*ptr.w);
		ptr.w++;
		NYCH = *ptr.b++;
		ptr.b += NYCH;
		ptr.b += (((LONG) ptr.b) & 1);
		ptr.b += NYCH * 2;	/* not swapped */
		NYCA = *ptr.b++;
		ptr.b += NYCA + NYTR + NYCA;
		NLYCD = *ptr.b++;
		ptr.b += NLYCD * 2;
		ptr.b += (((LONG) ptr.b) & 1);
		*ptr.w = NLYLN = SWAPU (*ptr.w);
		ptr.w++;
		for (i = 0; i < NLYLN; i++)
		{		/* was NYSK */
		    *ptr.w = SWAPU (*ptr.w);
		    ptr.w++;
		}
		NXSK = *ptr.b++;
		ptr.b += NLOP;
		ptr.b += (((LONG) ptr.b) & 1);
		ptr.b += NXSK * 2;	/* not swapped */
		*ptr.b++;	/* NXTR */
		NXND = *ptr.b++;
		ptr.b += NXND * 3;
		NXDM = *ptr.b++;
		ptr.b += NXDM;
		NXPR = *ptr.b++;
		ptr.b += NXPR * 3;
		NXIN = *ptr.b++;
		ptr.b += NXIN;
		NXLC = *ptr.b++;
		ptr.b += (((LONG) ptr.b) & 1);
		for (i = 0; i < NXLC; i++)
		{		/* was NYSK */
		    *ptr.w = SWAPU (*ptr.w);
		    ptr.w++;
		}
		ptr.b += NXLC;
		ptr.b += (((LONG) ptr.b) & 1);
		*ptr.w = SWAPU (*ptr.w);
		ptr.w++;
		*ptr.w = SWAPU (*ptr.w);
	    }

	    else
	    {
		ptr.b = src.b + src.w[2];
		*ptr.b++;	/* NYSK */
		NYTR = *ptr.b++;
		NYND = *ptr.b++;
		ptr.b += NYND * 2;
		NYPR = *ptr.b++;
		ptr.b += NYPR * 3;
		NYIN = *ptr.b++;
		ptr.b += NYIN;
		ptr.b += (((LONG) ptr.b) & 1);
		*ptr.w = SWAPU (*ptr.w);
		ptr.w++;
		NYCA = *ptr.b++;
		ptr.b += NYCA + NYTR + NYCA;
		NLYCD = *ptr.b++;
		ptr.b += NLYCD * 2;
		ptr.b += (((LONG) ptr.b) & 1);
		*ptr.w = NLYLN = SWAPU (*ptr.w);
		*ptr.w++;
		for (i = 0; i < NLYLN; i++)
		{
		    *ptr.w = SWAPU (*ptr.w);
		    ptr.w++;
		}
		*ptr.b++;	/* NXSK */
		*ptr.b++;	/* NXTR */
		NXND = *ptr.b++;
		ptr.b += NXND * 2;
		NXPR = *ptr.b++;
		ptr.b += NXPR * 3;
		NXIN = *ptr.b++;
		ptr.b += NXIN;
		ptr.b += (((LONG) ptr.b) & 1);
		*ptr.w = SWAPU (*ptr.w);
		ptr.w++;
		*ptr.w = SWAPU (*ptr.w);
	    }
	    ptr.b = hold;
	}



	if (*ptr.w & HQ2FLAG)	/* HQ2data */
	{
	    /* swap hq2 contour loop and character data */
	    ptr.b = src.b + src.w[3] + 4;
	    for (i = 0; i < NLOP; i++)
	    {
		/* swap offset to character data */
		*ptr.w = SWAPU (*ptr.w);
		ptr2.b = src.b + *ptr.w;
		/* skip polarity and spare byte */
		ptr.w += 2;
		/* swap pointers to right and left branches */
		*ptr.w = SWAPU (*ptr.w);
		ptr.w++;
		*ptr.w = SWAPU (*ptr.w);
		ptr.w++;

		/* swap character data */
		*ptr2.w = NPNT = SWAPU (*ptr2.w);
		ptr2.w++;
		*ptr2.w = SWAPU (*ptr2.w);
		ptr2.w++;
		*ptr2.w = SWAPU (*ptr2.w);
		ptr2.w++;
		for (j = 0; j < NPNT * 2; j++)
		{
		    *ptr2.w = SWAPW (*ptr2.w);
		    ptr2.w++;
		}
	    }
	}

	else
	{
	    /* swap hq3 contour loop and character data */
	    ptr.b = src.b + src.w[3] + 4;
	    for (i = 0; i < NLOP; i++)
	    {
		/* swap offset to character data */
		*ptr.w = SWAPU (*ptr.w);
		ptr2.b = src.b + *ptr.w;
		/* skip polarity and spare byte */
		ptr.w += 2;
		/* swap pointers to right and left branches */
		*ptr.w = SWAPU (*ptr.w);
		ptr.w++;
		*ptr.w = SWAPU (*ptr.w);
		ptr.w++;

		/* swap character data */
		*ptr2.w = NPNT = SWAPU (*ptr2.w);
		ptr2.w++;
		*ptr2.w = SWAPU (*ptr2.w);
		ptr2.w++;
		for (j = 0; j < NPNT * 2; j++)
		{
		    *ptr2.w = SWAPW (*ptr2.w);
		    ptr2.w++;
		}
	    }
	}
    }
}
