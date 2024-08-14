/*
**      $Id: modify.c,v 38.4 92/07/09 15:55:03 darren Exp $
**
**      Fountain/modify.c -- handle modify button
**
**      (C) Copyright 1990 Robert R. Burns
**          All Rights Reserved
*/

#include  "fountain.h"

extern struct Library *SysBase;
extern struct Library *GadToolsBase;
extern struct Library *GfxBase;
extern struct Library *IntuitionBase;

extern struct Window *Window;
extern struct Gadget *GPMFace;
extern struct Gadget *GPMFaceActive;
extern struct Gadget *GPMSize;
extern struct Gadget *GPMSizeNum;
extern struct Gadget *GPMPerform;
extern struct Gadget *GPMCancel;

extern struct FontsEntry *CurrDDir;
extern ULONG DeviceDPI;
extern ULONG DotSize;
extern UWORD SymbolSet;

extern struct MinList MFaceList;

struct MinList EmptyList = {
    (struct MinNode *) &EmptyList.mlh_Tail, 0,
    (struct MinNode *) &EmptyList.mlh_Head
};

struct FaceDisplay *CurrFaceDisplay;
struct SizeDisplay *CurrSizeDisplay;
int CurrFacePosition;
int CurrSizePosition;
WORD CurrHeight;
char *ChangeArg[4];

int AddSize(fd, y, isBitmap)
struct FaceDisplay *fd;
UWORD y;
UBYTE isBitmap;
{
    struct SizeDisplay *sd, *sdNode, *sdPrev;
    int ordinal;

    DBG3("AddSize %s %ld %lc\n", fd->ref.fe->amigaName, y, isBitmap?'*':' ');
    if (!(sd = (struct SizeDisplay *) malloc(sizeof(struct SizeDisplay))))
	EndGame(RETURN_ERROR, ENDGAME_NoMemory, sizeof(struct SizeDisplay));

    sd->node.ln_Name = sd->name;
    sd->ySize = y;
    sd->isBitmap = isBitmap;
    if (isBitmap)
	sprintf(sd->name, "%5ld %s", y, LzS[ARG_Bitmap]);
    else
	sprintf(sd->name, "%5ld", y);
    sdPrev = 0;
    ordinal = 0;
    sdNode = (struct SizeDisplay *) fd->sizes.mlh_Head;
    while (sdNode->node.ln_Succ) {
	if (sd->ySize < sdNode->ySize)
	    break;
	if (sd->ySize == sdNode->ySize) {
	    DBG3("Duplicate ySize %ld isBitmap %ld vs. %ld\n", sd->ySize,
		    sd->isBitmap, sdNode->isBitmap);
	    free(sd);
	    return(ordinal);
	}
	ordinal++;
	sdPrev = sdNode;
	sdNode = (struct SizeDisplay *) sdNode->node.ln_Succ;
    }
    Insert((struct List *) &fd->sizes, (struct Node *) sd,
		(struct Node *) sdPrev);
    return(ordinal);
}

short CalcChanges()
{
    struct FaceDisplay *fd;
    struct FontEntry *fe;
    struct OTagEntry *oe;
    struct SizeDisplay *sd;
    struct SizeEntry *se;
    short i, j, items;

    DBG("CalcChanges...\n");
    /* see what changed */
    /* set all tags */
    for (fd = (struct FaceDisplay *) MFaceList.mlh_Head;
	    fd->node.ln_Succ;
	    fd = (struct FaceDisplay *) fd->node.ln_Succ) {
	for (sd = (struct SizeDisplay *) fd->sizes.mlh_Head;
		sd->node.ln_Succ;
		sd = (struct SizeDisplay *) sd->node.ln_Succ) {
	    DBG3("set sd %ld %ld %s\n", sd->ySize, sd->isBitmap, fd->name);
	    sd->fFlags |= FFLAG_TAG;
	}
    }
    for (fe = (struct FontEntry *) CurrDDir->fcList.mlh_Head;
	    fe->node.mln_Succ;
	    fe = (struct FontEntry *) fe->node.mln_Succ) {
	if ((fe->fFlags & FFLAG_COMPLETE) == FFLAG_COMPLETE) {
	    DBG1("set fe %s\n", fe->amigaName);
	    fe->fFlags |= FFLAG_TAG;
	    for (se = (struct SizeEntry *) fe->sizes.mlh_Head;
		    se->node.mln_Succ;
		    se = (struct SizeEntry *) se->node.mln_Succ) {
		DBG2("set fe se %ld %s\n", se->ySize, fe->amigaName);
		se->fFlags |= FFLAG_TAG;
	    }
	    for (se = (struct SizeEntry *) fe->oe->sizes.mlh_Head;
		    se->node.mln_Succ;
		    se = (struct SizeEntry *) se->node.mln_Succ) {
		DBG2("set oe se %ld %s\n", se->ySize, fe->oe->amigaName);
		se->fFlags |= FFLAG_TAG;
	    }
	}
	else
	    /* just an Amiga bitmap font */
	    fe->fFlags &= ~FFLAG_TAG;
    }

    /* look for differences and then clear tags */
    for (fd = (struct FaceDisplay *) MFaceList.mlh_Head;
	    fd->node.ln_Succ;
	    fd = (struct FaceDisplay *) fd->node.ln_Succ) {
	fe = fd->ref.fe;
	oe = fe->oe;

	/*   clear fe tags from typefaces not deleted */
	DBG1("clr fe %s\n", fe->amigaName);
	fe->fFlags &= ~FFLAG_TAG;

	/*   clear sd tags from sizes not created */
	for (sd = (struct SizeDisplay *) fd->sizes.mlh_Head;
		sd->node.ln_Succ;
		sd = (struct SizeDisplay *) sd->node.ln_Succ) {
	    if (sd->isBitmap) {
		/* look for it on the .font list */
		for (se = (struct SizeEntry *) fe->sizes.mlh_Head;
			se->node.mln_Succ;
			se = (struct SizeEntry *) se->node.mln_Succ) {
		    if (sd->ySize == se->ySize) {
			DBG2("clr fe se %ld %s\n", se->ySize, fe->amigaName);
			se->fFlags &= ~FFLAG_TAG;
			break;
		    }
		}
	    }
	    else {
		/* look for it on the .otag list */
		for (se = (struct SizeEntry *) oe->sizes.mlh_Head;
			se->node.mln_Succ;
			se = (struct SizeEntry *) se->node.mln_Succ) {
		    if (sd->ySize == se->ySize) {
			DBG2("clr oe se %ld %s\n", se->ySize, oe->amigaName);
			se->fFlags &= ~FFLAG_TAG;
			break;
		    }
		}
	    }
	}

	/* clear fe and oe flags from sizes not deleted */
	for (se = (struct SizeEntry *) fe->sizes.mlh_Head;
		se->node.mln_Succ;
		se = (struct SizeEntry *) se->node.mln_Succ) {
	    for (sd = (struct SizeDisplay *) fd->sizes.mlh_Head;
		    sd->node.ln_Succ;
		    sd = (struct SizeDisplay *) sd->node.ln_Succ) {
		if ((se->ySize == sd->ySize) &&
			(sd->isBitmap)) {
		    DBG2("clr fe sd %ld %s\n", se->ySize, fe->amigaName);
		    sd->fFlags &= ~FFLAG_TAG;
		    break;
		}
	    }
	}
	for (se = (struct SizeEntry *) oe->sizes.mlh_Head;
		se->node.mln_Succ;
		se = (struct SizeEntry *) se->node.mln_Succ) {
	    for (sd = (struct SizeDisplay *) fd->sizes.mlh_Head;
		    sd->node.ln_Succ;
		    sd = (struct SizeDisplay *) sd->node.ln_Succ) {
		if ((se->ySize == sd->ySize) &&
			(!sd->isBitmap)) {
		    DBG2("clr oe sd %ld %s\n", se->ySize, oe->amigaName);
		    sd->fFlags &= ~FFLAG_TAG;
		    break;
		}
	    }
	}
    }

    ChangeArg[0] = ChangeArg[1] = ChangeArg[2] = ChangeArg[3] = 0;
    items = 0;
    for (fd = (struct FaceDisplay *) MFaceList.mlh_Head;
	    fd->node.ln_Succ;
	    fd = (struct FaceDisplay *) fd->node.ln_Succ) {
	for (sd = (struct SizeDisplay *) fd->sizes.mlh_Head;
		sd->node.ln_Succ;
		sd = (struct SizeDisplay *) sd->node.ln_Succ) {
	    if (sd->fFlags & FFLAG_TAG)
		if (sd->isBitmap) {
		    ChangeArg[2] = LzS[ARG_CreateBitmap];
		    items++;
		}
		else
		    ChangeArg[1] = LzS[ARG_UpdateSizes];
	}
    }
    for (fe = (struct FontEntry *) CurrDDir->fcList.mlh_Head;
	    fe->node.mln_Succ;
	    fe = (struct FontEntry *) fe->node.mln_Succ) {
	if ((fe->fFlags & FFLAG_COMPLETE) == FFLAG_COMPLETE) {
	    if (fe->fFlags & FFLAG_TAG)
		ChangeArg[0] = LzS[ARG_DeleteType];
	    for (se = (struct SizeEntry *) fe->sizes.mlh_Head;
		    se->node.mln_Succ;
		    se = (struct SizeEntry *) se->node.mln_Succ) {
		if (se->fFlags & FFLAG_TAG)
		    ChangeArg[3] = LzS[ARG_DeleteBitmap];
	    }
	    for (se = (struct SizeEntry *) fe->oe->sizes.mlh_Head;
		    se->node.mln_Succ;
		    se = (struct SizeEntry *) se->node.mln_Succ) {
		if (se->fFlags & FFLAG_TAG)
		    ChangeArg[1] = LzS[ARG_UpdateSizes];
	    }
	}
    }
    for (i = 0; i < 4; i++) {
	if (!ChangeArg[i]) {
	    for (j = i+1; j < 4; j++)
		ChangeArg[j-1] = ChangeArg[j];
	    ChangeArg[3] = "";
	    i--;
	}
    }
    DBG1("CalcChanges done: %ld.\n", items);
    return(items);
}

SwizzleBottom()
{
    CalcChanges();
    if (*ChangeArg[0])
	GT_SetGadgetAttrs(GPMPerform, Window, 0, GA_Disabled, FALSE, TAG_DONE);
    else
	GT_SetGadgetAttrs(GPMPerform, Window, 0, GA_Disabled, TRUE, TAG_DONE);
}

MFaceSize(gtFlag)
BOOL gtFlag;
{
    DBG("MFaceSize\n");
    CurrSizeDisplay = 0;
    CurrSizePosition = ~0;
    CurrHeight = 10;
    if (CurrFaceDisplay) {
	CurrSizeDisplay = (struct SizeDisplay *)
		CurrFaceDisplay->sizes.mlh_Head;
	if (CurrSizeDisplay->node.ln_Succ) {
	    CurrSizePosition = 0;
	    CurrHeight = CurrSizeDisplay->ySize;
	}
	if (gtFlag) {
	    GT_SetGadgetAttrs(GPMFace, Window, 0, GTLV_Labels, &MFaceList,
		    TAG_DONE);
	    GT_SetGadgetAttrs(GPMFaceActive, Window, 0,
		    GTTX_Text, CurrFaceDisplay->name, TAG_DONE);
	    GT_SetGadgetAttrs(GPMSize, Window, 0,
		    GTLV_Labels, &CurrFaceDisplay->sizes,
		    TAG_DONE);
	}
    }
    else {
	if (gtFlag) {
	    GT_SetGadgetAttrs(GPMFace, Window, 0, GTLV_Labels, &EmptyList,
		    TAG_DONE);
	    GT_SetGadgetAttrs(GPMFaceActive, Window, 0, GTTX_Text, 0, TAG_DONE);
	    GT_SetGadgetAttrs(GPMSize, Window, 0, GTLV_Labels, &EmptyList,
		    TAG_DONE);
	}
    }
    if (gtFlag) {
	GT_SetGadgetAttrs(GPMSizeNum, Window, 0,
		GTIN_Number, CurrHeight, TAG_DONE);
	SwizzleBottom();
    }
}


MFace(newPosition)
int newPosition;
{
    DBG1("MFace %ld\n", newPosition);
    GT_SetGadgetAttrs(GPMFace, Window, 0, GTLV_Labels, ~0L, TAG_DONE);
    GT_SetGadgetAttrs(GPMSize, Window, 0, GTLV_Labels, ~0L, TAG_DONE);

    /* find entry associated with this position */
    for (CurrFaceDisplay = (struct FaceDisplay *) MFaceList.mlh_Head;
	    (newPosition > 0) && CurrFaceDisplay->node.ln_Succ;
	    newPosition--, CurrFaceDisplay = (struct FaceDisplay *)
	    CurrFaceDisplay->node.ln_Succ);
    if (!CurrFaceDisplay->node.ln_Succ)
	CurrFaceDisplay = 0;
    MFaceSize(TRUE);
}


BOOL MValidateSize(required, actionIndex)
BOOL required;
int actionIndex;
{
    struct SizeDisplay *sd;
    int pos;

    if (!CurrFaceDisplay) {
	ErrRequester(ERROR_NoFaceSelected, LzS[actionIndex]+1);
	return(FALSE);
    }

    CurrHeight = ((struct StringInfo *) GPMSizeNum->SpecialInfo)->LongInt;

    if (CurrHeight <= 0) {
	if (required)
	    ErrRequester(ERROR_NoSizeSelected, LzS[actionIndex]+1);
	else
	    ErrRequester(ERROR_NonPositiveSize);
	return(FALSE);
    }

    for (sd = (struct SizeDisplay *) CurrFaceDisplay->sizes.mlh_Head, pos = 0;
	    sd->node.ln_Succ;
	    sd = (struct SizeDisplay *) sd->node.ln_Succ, pos++) {
	if (sd->ySize > CurrHeight) {
	    /* no CurrHeight in entries */
	    break;
	}
	if (sd->ySize == CurrHeight) {
	    CurrSizeDisplay = sd;
	    CurrSizePosition = pos;
	    DBG2("Valid Size %ld position %ld\n", CurrHeight, pos);
	    return(TRUE);
	}
    }
    if (required) {
	ErrRequester(ERROR_NoSizeSelected, LzS[actionIndex]+1);
	return(FALSE);
    }
    return(TRUE);
}


void MAddSize()
{
    int selected;

    if (MValidateSize(FALSE, GADGET_AddSize)) {
	GT_SetGadgetAttrs(GPMSize, Window, 0, GTLV_Labels, ~0L, TAG_DONE);
	selected = AddSize(CurrFaceDisplay, CurrHeight, FALSE);
	GT_SetGadgetAttrs(GPMSize, Window, 0,
		GTLV_Labels, &CurrFaceDisplay->sizes,
		TAG_DONE);
	CurrSizeDisplay = (struct SizeDisplay *)
		CurrFaceDisplay->sizes.mlh_Head;
	CurrSizePosition = selected;
	while (selected-- > 0)
	    CurrSizeDisplay = (struct SizeDisplay *)
		     CurrSizeDisplay->node.ln_Succ;
    }
    SwizzleBottom();
}

void MDelSize()
{
    struct SizeDisplay *newCurr;

    if (MValidateSize(TRUE, GADGET_DeleteSize)) {
	GT_SetGadgetAttrs(GPMSize, Window, 0, GTLV_Labels, ~0L, TAG_DONE);
	newCurr = (struct SizeDisplay *) CurrSizeDisplay->node.ln_Succ;
	if (!newCurr->node.ln_Succ) {
	    newCurr = (struct SizeDisplay *) CurrSizeDisplay->node.ln_Pred;
	    CurrSizePosition--;
	}
	if (!newCurr->node.ln_Pred) {
	    newCurr = 0;
	    CurrSizePosition = ~0L;
	}
	else {
	    CurrHeight = newCurr->ySize;
	    GT_SetGadgetAttrs(GPMSizeNum, Window, 0,
		    GTIN_Number, CurrHeight, TAG_DONE);
	}
	Remove((struct Node *) CurrSizeDisplay);
	free(CurrSizeDisplay);
	CurrSizeDisplay = newCurr;
	GT_SetGadgetAttrs(GPMSize, Window, 0,
		GTLV_Labels, &CurrFaceDisplay->sizes,
		TAG_DONE);
    }
    SwizzleBottom();
}


void MModSize(isBitmap, actionIndex)
char isBitmap;
int actionIndex;
{
    if (MValidateSize(TRUE, actionIndex)) {
	GT_SetGadgetAttrs(GPMSize, Window, 0, GTLV_Labels, ~0L, TAG_DONE);
	if (isBitmap)
	    sprintf(CurrSizeDisplay->name, "%5ld %s", CurrSizeDisplay->ySize,
		    LzS[ARG_Bitmap]);
	else
	    sprintf(CurrSizeDisplay->name, "%5ld", CurrSizeDisplay->ySize);
	CurrSizeDisplay->ySize = CurrHeight;
	CurrSizeDisplay->isBitmap = isBitmap;
	GT_SetGadgetAttrs(GPMSize, Window, 0,
	    GTLV_Labels, &CurrFaceDisplay->sizes,
	    TAG_DONE);
    }
    SwizzleBottom();
}


MCreateBitmap()
{
    MModSize(TRUE, GADGET_CreateBitmap);
}


MDeleteBitmap()
{
    MModSize(FALSE, GADGET_DeleteBitmap);
}


MSize(newPosition)
int newPosition;
{
    short i;

    CurrSizePosition = newPosition;
    for (i = newPosition,
	    CurrSizeDisplay = (struct SizeDisplay *)
	    CurrFaceDisplay->sizes.mlh_Head;
	    i > 0;
	    i--, CurrSizeDisplay = (struct SizeDisplay *)
	    CurrSizeDisplay->node.ln_Succ);
    CurrHeight = CurrSizeDisplay->ySize;
    GT_SetGadgetAttrs(GPMSizeNum, Window, 0,
	    GTIN_Number, CurrHeight, TAG_DONE);
    SwizzleBottom();
}

MVanilla(key)
char key;
{
    char *is;
    int index;

    index = 0;
    is = LzS[GADGET_VanillaIndex];
    key &= 0xdf;		/* force to "upper case" cheaply */
    do {
	if (*is++ == key)
	    break;
	index++;
    }
	while (*is);
    switch (index) {
      case 0:			/* S: activate SizeNum gadget */
	ActivateGadget(GPMSizeNum, Window, 0);
	break;
      case 1:			/* A: Add Size */
	MAddSize();
	break;
      case 2:			/* C: Create Bitmap */
	MCreateBitmap();
	break;
      case 3:			/* D: Delete Size */
	if (*is == key) {
	    /* duplicate descriptions for Delete Size & Delete Bitmap */
	    if (MValidateSize(TRUE, GADGET_DeleteSize)) {
		if (CurrSizeDisplay->isBitmap)
		    MDeleteBitmap();
		else
		    MDelSize();
	    }
	    break;
	}
	MDelSize();
	break;
      case 4:			/* D: Delete Bitmap */
	MDeleteBitmap();
	break;
      default:;
    }
}


MDeleteFace()
{
    struct FaceDisplay *newCurrFace;

    if (CurrFaceDisplay) {
	GT_SetGadgetAttrs(GPMFace, Window, 0, GTLV_Labels, ~0L, TAG_DONE);
	GT_SetGadgetAttrs(GPMSize, Window, 0, GTLV_Labels, ~0L, TAG_DONE);
	/* find new current */
	newCurrFace = (struct FaceDisplay *) CurrFaceDisplay->node.ln_Succ;
	if (!newCurrFace->node.ln_Succ) {
	    CurrFacePosition--;
	    newCurrFace = (struct FaceDisplay *) CurrFaceDisplay->node.ln_Pred;
	}
	if (!newCurrFace->node.ln_Pred) {
	    /* no undeleted faces to make current */
	    DBG("CurrFaceDisplay cleared\n");
	    newCurrFace = 0;
	}
	Remove((struct Node *) CurrFaceDisplay);
	FreeList(&CurrFaceDisplay->sizes);
	free(CurrFaceDisplay);
	CurrFaceDisplay = newCurrFace;
	MFaceSize(TRUE);
    }
}


MUpdate()
{
    struct FaceDisplay *fd;
    struct FontEntry *fe;
    struct OTagEntry *oe;
    struct SizeDisplay *sd;
    struct SizeEntry *se, *seNode, *sePrev;
    struct List *seList;
    short items;

    /* see what changed */
    items = CalcChanges();

    if (WarnRequester(WARNING_Modify,
	    ChangeArg[0], ChangeArg[1], ChangeArg[2], ChangeArg[3])) {
	/* now convert tags into update entries */
	/* any tag on the entry list needs to be deleted */
	for (fe = (struct FontEntry *) CurrDDir->fcList.mlh_Head;
		fe->node.mln_Succ;
		fe = (struct FontEntry *) fe->node.mln_Succ) {
	    DBG1("chk fe %s\n", fe->amigaName);
	    if ((fe->fFlags & FFLAG_COMPLETE) == FFLAG_COMPLETE) {
		if (fe->fFlags & FFLAG_TAG) {
		    DBG1("DELETE fe %s\n", fe->amigaName);
		    fe->fFlags |= FFLAG_DELETE;
		    fe->oe->fFlags |= FFLAG_DELETE;
		    fe->te->fFlags |= FFLAG_DELETE;
		}
		for (se = (struct SizeEntry *) fe->sizes.mlh_Head;
			se->node.mln_Succ;
			se = (struct SizeEntry *) se->node.mln_Succ) {
		    if ((fe->fFlags & FFLAG_TAG) || (se->fFlags & FFLAG_TAG)) {
			DBG2("DELETE fe se %ld %s\n", se->ySize, fe->amigaName);
			se->fFlags |= FFLAG_DELETE;
		    }
		}
		for (se = (struct SizeEntry *) fe->oe->sizes.mlh_Head;
			se->node.mln_Succ;
			se = (struct SizeEntry *) se->node.mln_Succ) {
		    if ((fe->fFlags & FFLAG_TAG) || (se->fFlags & FFLAG_TAG)) {
			DBG2("DELETE fe oe %ld %s\n", se->ySize, fe->amigaName);
			se->fFlags |= FFLAG_DELETE;
		    }
		}
	    }
	}
	/* any tag on the display list needs to be created */
	for (fd = (struct FaceDisplay *) MFaceList.mlh_Head;
		fd->node.ln_Succ;
		fd = (struct FaceDisplay *) fd->node.ln_Succ) {
	    fe = fd->ref.fe;
	    oe = fe->oe;
	    for (sd = (struct SizeDisplay *) fd->sizes.mlh_Head;
		    sd->node.ln_Succ;
		    sd = (struct SizeDisplay *) sd->node.ln_Succ) {
		if (sd->fFlags & FFLAG_TAG) {
		    /* need to create a .font or .otag size entry */
		    if (!(se = (struct SizeEntry *)
			    malloc(sizeof(struct SizeEntry))))
			EndGame(RETURN_ERROR, ENDGAME_NoMemory,
				sizeof(struct SizeEntry));
		    if (sd->isBitmap) {
			DBG2("CREATE fe fe %ld %s\n", sd->ySize, fe->amigaName);
			se->ref.fe = fe;
			se->symbolSet = SymbolSet;
			sprintf(se->fontFile, "%s/%ld",
				fe->amigaName, sd->ySize);
			seList = (struct List *) &fe->sizes;
		    }
		    else {
			DBG2("CREATE fe oe %ld %s\n", sd->ySize, fe->amigaName);
			se->ref.oe = oe;
			se->symbolSet = oe->symbolSet;
			seList = (struct List *) &oe->sizes;
		    }
		    se->ySize = sd->ySize;
		    se->dpi = DeviceDPI;
		    se->dotSize = DotSize;
		    se->fFlags = FFLAG_COMPLETE | FFLAG_CREATE;
		    sePrev = 0;
		    seNode = (struct SizeEntry *) seList->lh_Head;
		    while (seNode->node.mln_Succ) {
			if (se->ySize < seNode->ySize)
			    break;
			sePrev = seNode;
			seNode = (struct SizeEntry *) seNode->node.mln_Succ;
		    }
		    Insert(seList, (struct Node *) se, (struct Node *) sePrev);
		}
	    }
	}
	FindOMatches(CurrDDir);
	Update(CurrDDir, items);
	WaitPointer(-1);
	OpenMainUI();
    }
}

SelectModify()
{
    struct FaceDisplay *fd;
    struct FontEntry *fe;
    struct SizeEntry *se;

    /* create MFaceList */
    /*   free prior list */
    for (fd = (struct FontDisplay *) MFaceList.mlh_Head;
	    fd->node.ln_Succ;
	    fd = (struct FontDisplay *) fd->node.ln_Succ) {
	FreeList(&fd->sizes);
    }
    FreeList(&MFaceList);

    if (CurrDDir) {
	for (fe = (struct FontEntry *) CurrDDir->fcList.mlh_Head;
		fe->node.mln_Succ;
		fe = (struct FontEntry *) fe->node.mln_Succ) {
	    if ((fe->fFlags & FFLAG_COMPLETE) == FFLAG_COMPLETE) {
		fd = (struct FaceDisplay *)
			AddFace(&MFaceList, fe->amigaName, fe);
		for (se = (struct SizeEntry *) fe->sizes.mlh_Head;
			se->node.mln_Succ;
			se = (struct SizeEntry *) se->node.mln_Succ)
		    AddSize(fd, se->ySize, TRUE);
		for (se = (struct SizeEntry *) fe->oe->sizes.mlh_Head;
			se->node.mln_Succ;
			se = (struct SizeEntry *) se->node.mln_Succ)
		    AddSize(fd, se->ySize, FALSE);
	    }
	    else
		DBG1("fe->fFlags $%lx not complete\n", fe->fFlags);
	}
    }

    CurrFaceDisplay = (struct FaceDisplay *) MFaceList.mlh_Head;

    if (!CurrFaceDisplay->node.ln_Succ) {
	ErrRequester(ERROR_NoTypefaces);
	return;
    }

    MFaceSize(FALSE);

    OpenModifyUI();
}
