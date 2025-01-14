/*
**	$Id: support.c,v 37.7 91/12/16 14:59:54 havemose Exp $
**
**	Fountain/support.c -- misc support routines
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
*/

#include  "fountain.h"

extern struct Library *SysBase;
extern struct Library *DOSBase;
extern struct Library *IntuitionBase;
extern struct Library *UtilityBase;

extern struct MinList DFaceList;
extern struct MinList MFaceList;

void GenBucket(typeEntry)
struct TypeEntry *typeEntry;
{
    UWORD bucket;

    bucket = 0;
    if (typeEntry->stemWeight > OTS_SemiBold)
	bucket |= 1;
    if (!typeEntry->serifFlag)
	bucket |= 2;
    if (typeEntry->slantStyle == 1)
	bucket |= 4;
    typeEntry->bucketBits = bucket;
}


static const UBYTE gcgcHorizStyles[] = {
    OTH_Normal, OTH_Normal, OTH_Condensed,
    OTH_Compressed, OTH_Expanded, OTH_ExtraExpanded
};
static const UBYTE gcgcStemWeights[] = {
    (OTS_Book+OTS_Medium)/2, OTS_ExtraLight, OTS_Light, (OTS_Book+OTS_Medium)/2,
    OTS_Bold, OTS_ExtraBold
};
static const UBYTE gcgcSlantStyles[] = {
    OTS_Upright, OTS_Upright, OTS_Italic, OTS_LeftItalic
};


GenCGCharacteristics(typeEntry)
struct TypeEntry *typeEntry;
{
    unsigned int index;

    DBG4("GenCGCharacteristics serif %ld horiz %ld stem %ld slant %ld\n",
	    typeEntry->serifFlag, typeEntry->horizStyle,
	    typeEntry->stemWeight, typeEntry->slantStyle);
    /* convert to amiga style ranges */
    if (typeEntry->serifFlag < 20)
	typeEntry->serifFlag = 0;
    else
	typeEntry->serifFlag = 1;	/* interesting: default to serif */
    index = typeEntry->horizStyle/10;
    if (index < sizeof(gcgcHorizStyles))
	typeEntry->horizStyle = gcgcHorizStyles[index];
    else
	typeEntry->horizStyle = OTH_Normal;
    index = typeEntry->stemWeight/10;
    if (index < sizeof(gcgcStemWeights))
	typeEntry->stemWeight = gcgcStemWeights[index];
    else
	typeEntry->stemWeight = (OTS_Book+OTS_Medium)/2;
    index = typeEntry->slantStyle/10;
    if (index < sizeof(gcgcSlantStyles))
	typeEntry->slantStyle = gcgcSlantStyles[index];
    else
	typeEntry->slantStyle = OTS_Upright;
    GenBucket(typeEntry);
}


static const UBYTE ghpcHorizStyles[] = {
    OTH_UltraCompressed, OTH_ExtraCompressed, OTH_Compressed, OTH_Condensed,
    (OTH_Condensed+OTH_Normal)/2, OTH_Normal, OTH_SemiExpanded, OTH_Expanded,
    OTH_ExtraExpanded
};
static const UBYTE ghpcStemWeights[] = {
    0, OTS_UltraThin, OTS_ExtraThin, OTS_Thin, OTS_ExtraLight, OTS_Light,
    OTS_DemiLight, OTS_SemiLight, (OTS_Book+OTS_Medium)/2, OTS_SemiBold,
    OTS_DemiBold, OTS_Bold, OTS_ExtraBold, OTS_Black, OTS_ExtraBlack,
    OTS_UltraBlack
};
static const UBYTE ghpcSlantStyles[] = {
    OTS_Upright, OTS_Italic, OTS_LeftItalic
};

GenHPCharacteristics(typeEntry)
struct TypeEntry *typeEntry;
{
    unsigned int index;

    DBG4("GenHPCharacteristics serif %ld horiz %ld stem %ld slant %ld\n",
	    typeEntry->serifFlag, typeEntry->horizStyle,
	    typeEntry->stemWeight, typeEntry->slantStyle);
    /* convert to amiga style ranges */
    if (typeEntry->serifFlag)
	typeEntry->serifFlag = 1;
    else
	typeEntry->serifFlag = 0;
    index = typeEntry->horizStyle;
    if (index < sizeof(ghpcHorizStyles))
	typeEntry->horizStyle = ghpcHorizStyles[index];
    else
	typeEntry->horizStyle = OTH_Normal;
    index = typeEntry->stemWeight;
    if (index < sizeof(ghpcStemWeights))
	typeEntry->stemWeight = ghpcStemWeights[index];
    else
	typeEntry->stemWeight = (OTS_Book+OTS_Medium)/2;
    index = typeEntry->slantStyle;
    if (index < sizeof(ghpcSlantStyles))
	typeEntry->slantStyle = ghpcSlantStyles[index];
    else
	typeEntry->slantStyle = OTS_Upright;
    GenBucket(typeEntry);
}


char *gatiHorizNames[] = {
    "UP", "XP", "XC", "C", "", "SE", "E", "XE"
};
char *gatiStemNames[] = {
    "UT", "XT", "T", "XL", "L", "DL", "SL", "",
    "", "SB", "DB", "B", "XB", "X", "XX", "UX"
};
char *gatiSlantNames[] = {
    "", "I", "R"
};


void getString(dest, source, n)
char *dest, *source;
int n;
{
    strncpy(dest, source, n);
    dest[n--] = '\0';
    for (; n >=0; n--) {
	if (isgraph(dest[n])) {
	    dest[n+1] = '\0';
	    break;
	}
    }
}


stripCopy(dest, src)
char *dest;
char *src;
{
    while (*src) {
	if (isgraph(*src))
	    *dest++ = *src;
	src++;
    }
    *dest = '\0';
}

GenAmigaTypeName(typeEntry, typeFaceName)
struct TypeEntry *typeEntry;
char *typeFaceName;
{
    char *name, name0[51], name1[51], name2[21], suffix[6];
    short index, slack, width;

    getString(name0, typeFaceName, 50);
    DBG1("GenAmigaTypeName(, %s)\n", name0);
    name = name1;
    stripCopy(name1, name0);
    if (strlen(name1) > 25) {
	DBG1("  name \"%s\" not short enough\n", name1);
	/* generate algo suffix */
	suffix[0] = '\0';
	index = typeEntry->horizStyle>>5;
	if (index < SIZEOF_HorizStyle)
	    strcat(suffix, LzS[SUFFIX_HorizStyle+index]);
	index = typeEntry->stemWeight>>4;
	DBG2("  amiga stemWeight $%lx, index %ld\n", typeEntry->stemWeight,
		index);
	if (index < SIZEOF_StrokeWeight)
	    strcat(suffix, LzS[SUFFIX_StrokeWeight+index]);
	index = typeEntry->slantStyle;
	if (index < SIZEOF_Posture)
	    strcat(suffix, LzS[SUFFIX_Posture+index]);

	/* find family suffix in typeFaceName */
	stripCopy(name2, typeEntry->family);
	DBG1("stripped family name \"%s\"\n", name2);
	name = name2;
	width = strlen(name2);
	slack = strlen(name1) - width;
	DBG2("width %ld slack %ld\n", width, slack);
	for (index = 0; index < slack; index++) {
	    if (strncmp(name1+index, name2, width) == 0) {
		name1[width+index] = '\0';
		if ((strlen(name1)+strlen(suffix)) <= 25)
		    name = name1;
		break;
	    }
	}
	strcat(name, suffix);
    }
    strcpy(typeEntry->amigaName, name);
}


struct FaceDisplay *
AddFace(list, name, ref)
struct MinList *list;
char *name;
struct FontEntry *ref;
{
    struct FaceDisplay *fd, *fdNode, *fdPrev;

    int result;

    DBG1("AddFace(,\"%s\",)\n", name);
    if (!(fd = (struct FaceDisplay *) malloc(sizeof(struct FaceDisplay))))
	EndGame(RETURN_ERROR, ENDGAME_NoMemory, sizeof(struct FaceDisplay));

    fd->node.ln_Name = fd->name;
    NewList(&fd->sizes);
    fd->ref.fe = ref;
    if (list == &DFaceList) {
	if (ref->fFlags & FFLAG_OFONT)
	    sprintf(fd->name, "%-25s\267", name);
	else
	    strcpy(fd->name, name);
    }
    else if (list == &MFaceList)
	strcpy(fd->name, name);
    else {
	strcpy(fd->name, "  ");
	strcpy(fd->name+2, name);
    }
    fdPrev = 0;
    fdNode = (struct FaceDisplay *) list->mlh_Head;
    while (fdNode->node.ln_Succ) {
	result = StrCmpNC(fd->name, fdNode->name);
	if (result < 0)
	    break;
	fdPrev = fdNode;
	fdNode = (struct FaceEntry *) fdNode->node.ln_Succ;
    }
    Insert((struct List *) list, (struct Node *) fd,
	    (struct Node *) fdPrev);
    return(fd);
}


unsigned char FoldC(ch)
unsigned char ch;
{
    if (((ch >= 0x61) && (ch <= 0x7a)) ||
	    ((ch >= 0xe0) && (ch != 0xf7) && (ch != 0xff)))
	return((unsigned char) (ch & 0xdf));
    return(ch);
}

int StrCmpNC(src1, src2)
unsigned char *src1, *src2;
{
    unsigned char c1;

    while (c1 = *src1++) {
	if (FoldC(c1) != FoldC(*src2++))
	    if (FoldC(c1) < FoldC(*--src2))
		return(-1);
	    else
		return(1);
    }
    if (*src2)
	return(-1);
    return(0);
}


int StrEquNC(src1, src2)
unsigned char *src1, *src2;
{
    return(!StrCmpNC(src1, src2));
}


void buildpath(dest, source1, source2)
BYTE *dest, *source1, *source2;
{
    DBG2("buildpath(, \"%s\", \"%s\")", source1, source2);

    if (strchr(source2, ':'))
	strcpy(dest, source2);
    else {
	strcpy(dest, source1);
	AddPart(dest, source2, PATHNAMELEN);
    }

    DBG1(": \"%s\"\n", dest);
}


void	memmove(dest, src, size)
char *dest, *src;
int size;
{
    while (size-- > 0)
	*dest++ = *src++;
}


void *GetTagPtr(ULONG tagVal, struct TagItem *tagList)
{
    struct TagItem *ti;

    if (ti = FindTagItem(tagVal | OT_Indirect, tagList)) {
	DBG2("GetTagPtr $%08x: +$%lx\n", tagVal, ti->ti_Data);
	return((void *) (((ULONG) tagList) + ti->ti_Data));
    }
    DBG1("GetTagPtr $%08x: <not found>\n", tagVal);
    return(0);
}


BOOL FontMatch(char *path1, char *name1, char *path2, char *name2)
{
    char c1, c2, *s;

    if (StrEquNC(name1, name2)) {
	/* do StrEquNC but keep track of failure location */
	do {
	    c1 = *path1++;
	    c2 = *path2++;
	}
	    while (c1 && (FoldC(c1) == FoldC(c2)));
	if ((c1 == 0) && (c2 == 0)) {
	    return(TRUE);
	}
	if (c1 == 0)
	    s = --path2;
	else if (c2 == 0)
	    s = --path1;
	else {
	    return(FALSE);
	}
	if ((*s == ':') || (*s == '/'))
	    s++;
	if (StrEquNC(s, DIR_OUTLINES)) {
	    return(TRUE);
	}
    }
    return(FALSE);
}
