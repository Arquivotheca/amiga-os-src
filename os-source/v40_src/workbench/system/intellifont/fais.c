/*
**	$Id: fais.c,v 37.8 92/02/10 10:51:30 davidj Exp $
**
**	Fountain/fais.c -- interpret FAIS data
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
**
**	based on licensed code...
**
**	"Copyright (C) Agfa Compugraphic, 1989, 1990. All rights reserved."
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

#define	DB(x)	;

extern struct MinList SourceList;

/*
**  Definition of Font Index File
*/
#define	MAX_FONT_FILES	32
struct IFontEntry
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
};

struct
{
    UWORD volId;
    UWORD nVols;
    UWORD volNo;
    LONG volLength;
    UWORD nFiles;
    struct IFontEntry font[MAX_FONT_FILES];
} indexFile;

/*
**  Definition of FAIS segment used by attribute file headers
*/
struct FileHeader
{
    UWORD Id;
    ULONG Length;
    UWORD nkeys;
    struct
    {
	WORD key;
	ULONG offset;
    } seg[50];
};

/*
**  Define FAIS Segment Keys
*/
#define  TYPEFACEHEADER		103
#define  FONTALIAS		107

struct TypefaceHeader
{
    UWORD NFACES;
    BYTE typeFaceName[50];
    BYTE familyName[20];
    BYTE weightName[20];
    LONG typefaceID;
    UBYTE stemStyle;
    UBYTE stemMod;
    UBYTE stemWeight;
    UBYTE slantStyle;
    UBYTE horizStyle;
    UBYTE vertXHeight;
    UBYTE videoStyle;
    UBYTE copyUsage;
};

#define  HPTABLENAME "HP                  "

struct HPFontAlias
{
    UBYTE p1[7];		/* (I'm not going to type *everything* in) */
    UBYTE appearance[1];
    UBYTE p2[9];		/* (I'm not going to type *everything* in) */
    UBYTE posture[1];
    UBYTE p3[1];		/* (I'm not going to type *everything* in) */
    UBYTE weight[2];
    UBYTE p4[11];		/* (I'm not going to type *everything* in) */
    UBYTE simpleSerif[1];
};

struct FontAliasTable
{
    char tableName[20];		/* e.g. HPTABLENAME */
    UWORD NXREF;
    struct HPFontAlias fontAlias;	/* actually [NXREF] */
};

struct FontAliasHeader
{
    UWORD NFATS;
    struct FontAliasTable fats;	/* actually [NFATS] */
};

BOOL FAISInfo (BYTE *faisDir)
{
    char buffer[256], smallBuffer[10];
    struct TypefaceHeader typefaceHeader;
    struct FontAliasHeader *fontAliasHeader;
    struct FontAliasTable *fontAliasTable;
    struct HPFontAlias *hpFontAlias;
    struct FileHeader *attrFileHeader;
    FILE *f;
    struct TypeEntry *typeEntry;
    char foundFAISFlag, taFlag, faFlag;
    short i, j, k, numKeys, nFiles;
    int len, typefaceID, complementID, spaceRequired;

    /* read the font index file */
    DB (kprintf ("buildpath [%s] fontindx.fi\n", faisDir));
    buildpath (buffer, faisDir, "fontindx.fi");

    if (!(f = fopen (buffer, "r")))
    {
	DB (kprintf ("fopen failure for \"%s\"\n", buffer));
	return (FALSE);
    }

    foundFAISFlag = TRUE;
    if ((fread ((BYTE *) & indexFile, 12, 1, f)) != 1)
    {
	DB (kprintf ("read failure on index file header\n"));
	foundFAISFlag = FALSE;
    }
    else
    {
	fseek (f, 0L, 0);
	nFiles = SWAPU (indexFile.nFiles);
	DB (kprintf ("%ld files\n", (LONG)nFiles));
	len = MyFileSize (buffer);

	if (nFiles > MAX_FONT_FILES)
	{
	    DB (kprintf ("Too many font files on FAIS disk: %ld > %ld\n", nFiles, MAX_FONT_FILES));
	    foundFAISFlag = FALSE;
	}
	/* Sometimes the files have a zero byte at the end */
	else if ((len != (nFiles * sizeof (struct IFontEntry) + 12)) &&
		 (len != (nFiles * sizeof (struct IFontEntry) + 13)))
	{
	    DB (kprintf ("file size not %ld\n", (nFiles * sizeof (struct IFontEntry) + 12)));
	    foundFAISFlag = FALSE;
	}
	else if ((fread ((BYTE *) & indexFile, nFiles * sizeof (struct IFontEntry) + 12, 1, f)) != 1)
	{
	    DB (kprintf ("read failure on index file\n"));
	    foundFAISFlag = FALSE;
	}
    }
    fclose (f);

    if (!foundFAISFlag)
	return (FALSE);

    /* go through table, read each dir file */
    foundFAISFlag = FALSE;
    typeEntry = 0;
    for (i = 0; i < nFiles; i++)
    {				/* loop thru indx file table */
	if (typeEntry == 0)
	{
	    if (!(typeEntry = (struct TypeEntry *)
		  malloc (sizeof (struct TypeEntry))))
	    {
		DB (kprintf ("malloc failure\n"));
		break;
	    }
	}

	DB (kprintf ("fais index %ld of %ld\n", i, nFiles));
	/* Skip attribute files no matter what we're doing */
	if (memcmp (indexFile.font[i].fileType, "FA", 2))
	{
	    DB (kprintf ("  skipped FA file\n"));
	    continue;		/* if not, skip */
	}

	/* get typeface id */
	strncpy (smallBuffer, indexFile.font[i].face, 6);
	smallBuffer[6] = '\0';
	typefaceID = atol (smallBuffer);

	/* get complement */
	strncpy (smallBuffer, indexFile.font[i].comp, 6);
	smallBuffer[6] = '\0';
	complementID = atol (smallBuffer);
	DB (kprintf ("  id %ld, complement %ld\n", typefaceID, complementID));

	/* open attribute file */
	strncpy (smallBuffer, indexFile.font[i].fileName, 8);
	smallBuffer[8] = '\0';
	buildpath (buffer, faisDir, smallBuffer);

	if ((f = fopen (buffer, "r+b")) == NULL)
	{
	    DB (kprintf ("  fopen %s failure\n", buffer));
	    continue;
	}

	/* setup attrib file header */
	if (fread (buffer, 256, 1, f) != 1)
	{
	    DB (kprintf ("fread failed\n"));
	    goto fiBailoutFile;
	}

	attrFileHeader = (struct seg_type *) buffer;
	len = SWAPU (attrFileHeader->nkeys) * 6 + 8;
	if ((attrFileHeader = (struct FileHeader *) malloc (len)) == NULL)
	{
	    DB (kprintf ("malloc failure\n"));
	    goto fiBailoutFile;
	}

	fseek (f, 0L, 0);
	if (fread ((char *) attrFileHeader, len, 1, f) != 1)
	{
	    DB (kprintf ("fread for %ld failed\n", len));
	    goto fiBailoutMem;
	}

	numKeys = SWAPU (attrFileHeader->nkeys);
	taFlag = faFlag = FALSE;
	for (k = 0; (k < numKeys) && ((!taFlag) || (!faFlag)); k++)
	{
	    if (attrFileHeader->seg[k].key == SWAPW (TYPEFACEHEADER))
	    {
		fseek (f, SWAPL (attrFileHeader->seg[k].offset), 0);
		if (fread ((char *) &typefaceHeader, sizeof (typefaceHeader),
			   1, f) != 1)
		{
		    DB (kprintf ("fread typefaceHeader failed\n"));
		    goto fiBailoutMem;
		}
		spaceRequired = 0;
		for (j = 0; (j < nFiles); j++)
		{		/* loop through index */
		    strncpy (smallBuffer, indexFile.font[j].face, 6);
		    smallBuffer[6] = '\0';
		    if (typefaceID == atol (smallBuffer))
		    {		/* id matches */
			/* get file size */
			strncpy (smallBuffer, indexFile.font[j].fileName, 8);
			smallBuffer[8] = '\0';
			buildpath (buffer, faisDir, smallBuffer);
			spaceRequired += MyFileSize (buffer);
		    }
		}

		typeEntry->typefaceID = typefaceID;
		typeEntry->complementID = complementID;
		typeEntry->spaceReq = spaceRequired;
		getString (typeEntry->family,
			   typefaceHeader.familyName, 20);
		if (!faFlag)
		{
		    typeEntry->serifFlag = typefaceHeader.stemStyle;
		    typeEntry->stemWeight = typefaceHeader.stemWeight;
		    typeEntry->slantStyle = typefaceHeader.slantStyle;
		    typeEntry->horizStyle = typefaceHeader.horizStyle;
		    GenCGCharacteristics (typeEntry);
		}
		strcpy (typeEntry->sourceFile, faisDir);
		taFlag = TRUE;
	    }
	    else if (attrFileHeader->seg[k].key == SWAPW (FONTALIAS))
	    {
		fseek (f, SWAPL (attrFileHeader->seg[k].offset), 0);
		if (fread ((char *) buffer, sizeof (buffer), 1, f) != 1)
		{
		    DB (kprintf ("fread fontaliasheader failed\n"));
		    goto fiBailoutMem;
		}
		fontAliasHeader = (struct FontAliasHeader *) buffer;
		if (SWAPU (fontAliasHeader->NFATS) > 0)
		{
		    fontAliasTable = &fontAliasHeader->fats;
		    if ((!(strncmp (fontAliasTable->tableName, HPTABLENAME, 20)))
			&& (SWAPU (fontAliasTable->NXREF) > 0))
		    {
			hpFontAlias = &fontAliasTable->fontAlias;
			typeEntry->serifFlag = atoi (hpFontAlias->simpleSerif);
			typeEntry->stemWeight = atoi (hpFontAlias->weight);
			typeEntry->slantStyle = atoi (hpFontAlias->posture);
			typeEntry->horizStyle = atoi (hpFontAlias->appearance);
			GenHPCharacteristics (typeEntry);
			faFlag = TRUE;
		    }
		}
	    }
	}
	if (taFlag)
	{
	    GenAmigaTypeName (typeEntry, typefaceHeader.typeFaceName);
	    DB (kprintf ("  name \"%s\"\n", typeEntry->amigaName));
	    DB (kprintf ("  serifFlag %ld\n", typeEntry->serifFlag));
	    DB (kprintf ("  stemWeight %ld\n", typeEntry->stemWeight));
	    DB (kprintf ("  slantStyle %ld\n", typeEntry->slantStyle));
	    DB (kprintf ("  horizStyle %ld\n", typeEntry->horizStyle));
	    DB (kprintf ("  spaceReq %ld\n", typeEntry->spaceReq));
	    AddTail ((struct List *) & SourceList, (struct Node *) typeEntry);
	    typeEntry = 0;
	    foundFAISFlag = TRUE;
	}

      fiBailoutMem:
	free (attrFileHeader);
      fiBailoutFile:
	fclose (f);
    }				/* end loop through indx file table */

    if (typeEntry)
	free (typeEntry);

    DB (kprintf ("  FAISInfo done.\n"));
    return ((BOOL) foundFAISFlag);
}
