/*
**	$Id: lib.c,v 37.6 91/03/11 14:22:45 kodiak Exp $
**
**	Fountain/fais.c -- interpret FAIS data
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
**
**	based on licensed code...
**
**	"Copyright (C) Agfa Compugraphic, 1990. All rights reserved."
**
**	...and...
**
**	"Copyright (C) 1987, All Rights Reserved, by
**	 Compugraphic Corporation, Wilmington, Ma.
**
**	"This software is furnished under a license and may be used and copied
**	 only in accordance with the terms of such license and with the
**	 inclusion of the above copyright notice. This software or any other
**	 copies thereof may not be provided or otherwise made available to any
**	 other person. No title to and ownership of the software is hereby
**	 transferred.
**
**	"The information in this software is subject to change without notice
**	 and should not be construed as a commitment by Compugraphic
**	 Corporation."
**
*/

#include  "fountain.h"

BOOL LIBInfo(libDir, libList, huntLimit)
BYTE *libDir;
struct List *libList;
int huntLimit;
{
    struct TypeEntry *typeEntry;
    struct MyDir *md;
    struct {
	WORD    key;
	ULONG   offset;
	UWORD   count;
    } fdSeg,					/* file directory segment */
	tgSeg;					/* type face global segment */
    struct {
	LONG    faceNum;
	LONG    offset;
	UWORD   count;
    } fDir[2];					/* file directory */
    struct {
	LONG    offset;
	WORD	count;
    } fhSeg;					/* face header segment */
#if 0
    struct {
	WORD    charNum;
	LONG	offset;
	WORD	count;
    } cDir;
    struct {
	WORD    ccharNum;
	WORD	junk1;
	WORD	junk2;
	WORD	npcc;
    } ccDir;
    struct {
	WORD    ccharNum;
	WORD	x;
	WORD    y;
    } ccPart;
#endif
    struct {
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
	struct {
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
    } faSeg;
    struct {
	UWORD   NFACES;				/* must be 1 for this defn */
	char    typeFaceName[50];
	char    familyName[20];
	char    weightName[20];
	ULONG   typefaceID;
	UBYTE   stemStyle;
	UBYTE   stemMod;
	UBYTE   stemWeight;
	UBYTE   slantStyle;
	UBYTE   horizStyle;
	UBYTE   vertXHeight;
	UBYTE   videoStyle;
	UBYTE   copyUsage;
    } typefaceHeader;
#define  HPTABLENAME "HP                  "
    struct {
	char    tableName[20];
	UWORD	NXREF;
    } fontAliasTable;
    struct {
	UBYTE   p1[7];		/* (I'm not going to type *everything* in) */
	UBYTE   appearance[1];
	UBYTE   p2[9];		/* (I'm not going to type *everything* in) */
	UBYTE   posture[1];
	UBYTE   p3[1];		/* (I'm not going to type *everything* in) */
	UBYTE   weight[2];
	UBYTE   p4[11];		/* (I'm not going to type *everything* in) */
	UBYTE   simpleSerif[1];
    } hpFontAlias;

    FILE *f;
    LONG offset;
    WORD w, nfats;
    char foundLibFlag, faFlag, fontAliasFlag, typefaceHeaderFlag;

    if (!(md = MyDirStart(libDir))) {
	DBG1("MyDirStart %s failed\n", libDir);
	return(0);
    }

    foundLibFlag = FALSE;
    typeEntry = 0;
    while (MyDirNext(md)) {
	if (typeEntry == 0) {
	    if (!(typeEntry = (struct TypeEntry *)
		    malloc(sizeof(struct TypeEntry)))) {
		DBG("malloc failure\n");
		break;
	    }
	}

	if (md->md_FIB.fib_DirEntryType >= 0)
	    continue;
	if (!(f = fopen(md->md_File, "r"))) {
	    DBG1("file %s open failure\n", md->md_File);
	    continue;
	}
	if (fread((char *) &w, 2, 1, f) != 1) {
	    DBG("  cannot read 2 bytes from file\n");
	    goto liBailoutFile;
	}
	if (w != 'D') {
	    DBG("  file not intellifont disk library\n");
	    goto liBailoutFile;
	}
	for (;;) {
	    if (fread((char *) &fdSeg, sizeof(fdSeg), 1, f) != 1) {
		DBG("  failure reading segment entries\n");
		goto liBailoutFile;
	    }
	    if (fdSeg.key == 2) {
		if (fdSeg.count != sizeof(fDir)) {
		    DBG("  not just one typeface in library\n");
		    goto liBailoutFile;
		}
		break;
	    }
	    if (fdSeg.key == -1) {
		DBG("  exhausted segment entries\n");
		goto liBailoutFile;
	    }
	}
	if (fseek(f, fdSeg.offset, 0) < 0) {
            DBG("fdSeg seek error\n");
	    goto liBailoutFile;
	}
	if (fread((UBYTE *) fDir, fdSeg.count, 1, f) != 1) {
            DBG("read fDir error\n");
	    goto liBailoutFile;
	}
	if ((fDir[0].faceNum == -1) || (fDir[1].faceNum != -1)) {
	    DBG("  not exactly one typeface in library\n");
	    goto liBailoutFile;
	}

	/* get Face Header Segment */
	if (fseek(f, fDir[0].offset, 0) < 0) {
            DBG("fDir seek error\n");
	    goto liBailoutFile;
	}
	if (fread((UBYTE *) &fhSeg, sizeof(fhSeg), 1, f) != 1) {
            DBG("read fhSeg error\n");
	    goto liBailoutFile;
	}

#if 0
	while (fread((UBYTE *) &cDir, sizeof(cDir), 1, f) == 1) {
	    if (cDir.charNum == -1)
		break;
	    DBG3("%6ld CG# %ld %ld\n", fDir[0].faceNum,
		    cDir.charNum, cDir.count);
	}
#endif
	/* point to Type Face Global Segment */
	offset = fhSeg.offset+6;


	faFlag = fontAliasFlag = typefaceHeaderFlag = FALSE;
	do {
	    if (fseek(f, offset, 0) < 0) {
        	DBG("tgSeg seek error\n");
		goto liBailoutFile;
	    }
	    if (fread((UBYTE *) &tgSeg, sizeof(tgSeg), 1, f) != 1) {
        	DBG("read tgSeg error\n");
		goto liBailoutFile;
	    }
	    DBG2("tgSeg %ld @ $%lx\n", tgSeg.key, offset);
	    offset += sizeof(tgSeg);	/* update offset */
	    if (tgSeg.key == 105) {	/* faceattribute header key */
		if (fseek(f, fhSeg.offset+tgSeg.offset+6, 0) < 0) {
		    DBG("faceattribute seek error\n");
		    continue;
		}
		if (fread((UBYTE *) &faSeg, sizeof(faSeg), 1, f)
			!= 1) {
        	    DBG("read faceattribute error\n");
		    continue;
		}
		typeEntry->ySizeFactor = faSeg.ascender-faSeg.descender;
		DBG2("ySizeFactor %ld capHeight %ld\n", typeEntry->ySizeFactor,
			faSeg.capHeight);
		typeEntry->ySizeNumerator = faSeg.scaleFactor;
		typeEntry->isFixed = faSeg.isFixedPitch;
		typeEntry->spaceWidth = faSeg.spaceBand;
		faFlag = TRUE;
	    }
	    else if (tgSeg.key == 107) {	/* typeface header key */
		if (fseek(f, fhSeg.offset+tgSeg.offset+6, 0) < 0) {
		    DBG("typeface header seek error\n");
		    goto liBailoutFile;
		}
		if (fread((UBYTE *) &typefaceHeader, sizeof(typefaceHeader),
			1, f) != 1) {
        	    DBG("read typeface header error\n");
		    goto liBailoutFile;
		}
		if (typefaceHeader.NFACES != 1) {
		    DBG1("typeface NFACES %ld, not 1\n", typefaceHeader.NFACES);
		    DBG1("  family \"%.20s\"\n", typefaceHeader.familyName);
		    goto liBailoutFile;
		}

		/* typeface header exists: typeEntry can be linked in */
		typeEntry->typefaceID = typefaceHeader.typefaceID;
		typeEntry->complementID = 0;

		typeEntry->faceheaderOffset = fDir[0].offset;
		typeEntry->faceheaderCount = fDir[0].count;

		typeEntry->spaceReq = MyFileSize(md->md_File);

		getString(typeEntry->family,
			typefaceHeader.familyName, 20);

		if (!fontAliasFlag) {
		    typeEntry->serifFlag =
			    typefaceHeader.stemStyle;
		    typeEntry->stemWeight =
			    typefaceHeader.stemWeight;
		    typeEntry->slantStyle =
			    typefaceHeader.slantStyle;
		    typeEntry->horizStyle =
			    typefaceHeader.horizStyle;
		    GenCGCharacteristics(typeEntry);
		}
		strcpy(typeEntry->sourceFile, md->md_Path);
		typefaceHeaderFlag = TRUE;
	    }
	    else if (tgSeg.key == 110) {	/* fontalias key */
		if (fseek(f, fhSeg.offset+tgSeg.offset+6, 0) < 0) {
		    DBG("fontalias seek error\n");
		    continue;
		}
		if (fread((UBYTE *) &nfats, sizeof(nfats), 1, f) != 1) {
        	    DBG("read fontalias nfats error\n");
		    continue;
		}
		DBG1("  fontalias nfats %ld\n", nfats);
		for (; nfats > 0; nfats--) {
		    if (fread((UBYTE *) &fontAliasTable,
			    sizeof(fontAliasTable), 1, f) != 1) {
        		DBG("read fontalias table name error\n");
			goto liBailoutFontAlias;
		    }
		    DBG2("  fontalias tablename \"%.20s\", NXREF %ld\n",
			    fontAliasTable.tableName, fontAliasTable.NXREF);
		    if (strncmp(fontAliasTable.tableName, HPTABLENAME, 20)) {
			if (fseek(f, fontAliasTable.NXREF*104, 1) < 0) {
			    DBG("fontalias skip nxref error\n");
			    goto liBailoutFontAlias;
			}
			continue;
		    }
		    if (fontAliasTable.NXREF >= 1) {
			if (fread((UBYTE *) &hpFontAlias, sizeof(hpFontAlias),
				1, f) != 1) {
        		    DBG("read hpFontAlias error\n");
			    goto liBailoutFontAlias;
			}
			DBG1("HP FontAlias %.40s\n", &hpFontAlias);
			typeEntry->serifFlag = atoi(hpFontAlias.simpleSerif);
			typeEntry->stemWeight = atoi(hpFontAlias.weight);
			typeEntry->slantStyle = atoi(hpFontAlias.posture);
			typeEntry->horizStyle = atoi(hpFontAlias.appearance);
			GenHPCharacteristics(typeEntry);
			fontAliasFlag = TRUE;
		    }
		}
liBailoutFontAlias:;
	    }
#if 0
	    else if (tgSeg.key == 108) {
		/* compound character segment */
		if (fseek(f, fhSeg.offset+tgSeg.offset, 0) < 0) {
		    DBG("compoundchar seek error\n");
		    continue;
		}
		if (fread((char *) &cDir, sizeof(cDir), 1, f) != 1) {
		    DBG("compoundchar seek error\n");
		    continue;
		}
		for (; cDir.count > 0; cDir.count--) {
		    if (fread((UBYTE *) &ccDir, sizeof(ccDir), 1, f) != 1)
			break;
		    DBG3("%6ld CG# %ld %ld", fDir[0].faceNum,
			    ccDir.ccharNum, ccDir.npcc);
		    for (; ccDir.npcc > 0; ccDir.npcc--) {
			if (fread((UBYTE *) &ccPart, sizeof(ccPart), 1, f) != 1)
			    break;
			DBG3(", %ld %ld %ld", ccPart.ccharNum,
				ccPart.x, ccPart.y);
		    }
		    DBG("\n");
		}
	    }
#endif
	}
	    while ((tgSeg.key != -1)
#if 1
		    && ((!faFlag) || (!fontAliasFlag) || (!typefaceHeaderFlag))
#endif
		    );

	if (faFlag && typefaceHeaderFlag) {
	    GenAmigaTypeName(typeEntry, typefaceHeader.typeFaceName);
	    DBG1("  name \"%s\"\n", typeEntry->amigaName);
	    DBG1("  family \"%s\"\n", typeEntry->family);
	    DBG1("  serifFlag %ld\n", typeEntry->serifFlag);
	    DBG1("  stemWeight %ld\n", typeEntry->stemWeight);
	    DBG1("  slantStyle %ld\n", typeEntry->slantStyle);
	    DBG1("  horizStyle %ld\n", typeEntry->horizStyle);
	    DBG1("  spaceReq %ld\n", typeEntry->spaceReq);
	    typeEntry->fFlags = FFLAG_TYPE;
	    strcpy(typeEntry->typePath, libDir);
	    AddTail(libList, (struct Node *) typeEntry);
	    typeEntry = 0;
	    foundLibFlag = TRUE;
	}

liBailoutFile:
	fclose(f);
	if ((!foundLibFlag) && (--huntLimit == 0))
	    break;
    }

    if (typeEntry)
	free(typeEntry);

    MyDirEnd(md);

    return((BOOL) foundLibFlag);
}
