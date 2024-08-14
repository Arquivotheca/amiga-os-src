/****** Font2Asm *****************************************************
*
*   NAME
*       Font2Asm - create an assembly file from a font image
*
*********************************************************************/

#include	<stdio.h>
#include	<proto/exec.h>
#include	<proto/diskfont.h>

#include	"pragmas/graphics_pragmas.h"
#include        "exec/nodes.h"
#include        "exec/lists.h"
#include	"exec/libraries.h"
#include        "exec/memory.h"
#include        "exec/ports.h"
#include	"graphics/gfx.h"
#include	"graphics/gfxbase.h"
#include	"graphics/rastport.h"
#include	"graphics/text.h"
#include	"graphics/view.h"
#include        "libraries/dos.h"
#include	"libraries/diskfont.h"

char *Prog;
struct Library *DiskfontBase = 0;
struct GfxBase *GfxBase = 0;
struct ColorTextFont *CTF = 0;
FILE *File = 0;
struct BitMap BM  = { 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0 };
struct BitMap BMU = { 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0 };
char *Comment = 0;

struct TextAttr TA = { 0, 0, 0, 0 };
struct RastPort RP, RPU;
struct TextExtent TE, TEU, FE;
int X0, Y0, Width, Height, Page;


void
dataDump(data, height, width)
UWORD *data, height, width;
{
    UWORD col;

    for (; height > 0; height--) {
	col = 0;
	do {
	    if ((col & 7) == 0)
		fprintf(File, "\n\t	dc.w	");
	    else fprintf(File, ",");
	    fprintf(File, "$%04lx", *data++);
	    col++;
	}
	    while (col < width);
	fprintf(File, "\n");
    }
}


void
flushComment()
{
    int i, j;

    for (i = 0; i < Height+2; i++) {
	if (i-2+FE.te_Extent.MinY)
	    fprintf(File, "\n;   ");
	else
	    fprintf(File, "\n;___");
	fprintf(File, Comment+(i*80));
	for (j = 0; j < 80; j++)
	    Comment[(i*80)+j] = 0;
    }
    Page += Height+3;
    if (Page >= 60) {
	Page = Height+5;
	fprintf(File, "\n;\014;");
    }
    else
	fprintf(File, "\n;");
}


void
endGame(fmt, arg1, arg2, arg3)
char *fmt, *arg1, *arg2, *arg3;
{
    int i;

    printf("\nend... planes.");
    for (i = 0; i < 8; i++) {
	if (BM.Planes[i]) {
	    printf("%d.", i);
	    FreeRaster(BM.Planes[i], Width, Height);
	}
	if (BMU.Planes[i]) {
	    printf("%du.", i);
	    FreeRaster(BMU.Planes[i], Width, Height);
	}
    }
    if (Comment) {
	printf(" comment...");
	FreeMem(Comment, 80*(Height+2));
    }
    if (CTF) {
	printf(" CTF...");
	CloseFont((struct TextFont *) CTF);
    }
    if (DiskfontBase) {
	printf(" DF...");
	CloseLibrary(DiskfontBase);
    }
    if (GfxBase) {
	printf(" Gfx...");
	CloseLibrary((struct Library *) GfxBase);
    }

    if (fmt) {
	printf(fmt, arg1, arg2, arg3);
	printf("\nusage: %s <asm-file> <font-name> <font-size> [b] [i] [r]\n",
		Prog);
	exit(10);
    }

    printf("\n");
    exit(0);
}


parseNum(b)
char *b;
{
    int result;

    result = 0;
    while (*b) {
	if ((*b < '0') || (*b > '9')) {
	    endGame("error: illegal character \"%lc\"\n", *b);
	}
	result = result * 10 + *b - '0';
	b++;
    }
    return(result);
}


main(argc, argv)
int argc;
char *argv[];
{
    int i, j, k, x0, x, y;
    ULONG *longptr;
    WORD *wordptr;
    UBYTE *b, c, undef, cMore, undefMore, *cl;

    Prog = argv[0];

    if (argc < 3) {
	endGame("ERROR: wrong number of parameters\n");
    }

    GfxBase = (struct GfxBase *) OpenLibrary("graphics.library", 36);
    if (GfxBase == 0) {
	endGame("ERROR: Cannot open \"graphics.library\", 36\n");
    }
    DiskfontBase = (struct Library *) OpenLibrary("diskfont.library",32);
    if (DiskfontBase == 0) {
	endGame("ERROR: Cannot open \"diskfont.library\", 32\n");
    }

    File = fopen(argv[1], "w");
    if (File == 0) {
	endGame("ERROR: open failed for font file \"%s\"\n", argv[1]);
    }

    TA.ta_Name = argv[2];
    TA.ta_YSize = parseNum(argv[3]);
    for (i = 4; i < argc; i++) {
	b = argv[i];
	while (*b) {
	    if ((*b & 0x5f) == 'B') TA.ta_Style |= FSF_BOLD;
	    else if ((*b & 0x5f) == 'I') TA.ta_Style |= FSF_ITALIC;
	    else if ((*b & 0x5f) == 'R') TA.ta_Flags |= FPF_REVPATH;
	    b++;
	}
    }

    CTF = (struct ColorTextFont *) OpenDiskFont(&TA);
    if (CTF == 0) {
	endGame("ERROR: cannot OpenDiskFont \"%s\", %d, 0x%02lx\n",
		TA.ta_Name, TA.ta_YSize, TA.ta_Style);
    }


    InitRastPort(&RP);
    InitRastPort(&RPU);
    SetFont(&RP, CTF);
    SetFont(&RPU, CTF);
    FontExtent(CTF, &FE);

    X0 = -FE.te_Extent.MinX;
    Y0 = -FE.te_Extent.MinY;
    Width = ((FE.te_Extent.MaxX - FE.te_Extent.MinX) >= FE.te_Width) ?
	    (FE.te_Extent.MaxX - FE.te_Extent.MinX + 1) : FE.te_Width;
    Height = ((FE.te_Extent.MaxY - FE.te_Extent.MinY) >= FE.te_Height) ?
	    (FE.te_Extent.MaxY - FE.te_Extent.MinY + 1) : FE.te_Height;
    
    printf("\"%s\" Y %d S $%02lx F $%02lx  xy0 %d,%d wh %d,%d\n",
	    CTF->ctf_TF.tf_Message.mn_Node.ln_Name, CTF->ctf_TF.tf_YSize,
	    CTF->ctf_TF.tf_Style, CTF->ctf_TF.tf_Flags, X0, Y0, Width, Height);

    if (CTF->ctf_TF.tf_Style & FSF_COLORFONT) {
	InitBitMap(&BM, CTF->ctf_Depth, Width, Height);
	InitBitMap(&BMU, CTF->ctf_Depth, Width, Height);
	for (i = 0; i < CTF->ctf_Depth; i++) {
	    BM.Planes[i] = (PLANEPTR) AllocRaster(Width, Height);
	    if (BM.Planes[i] == 0) {
		endGame("AllocRaster failure\n");
	    }
	    BMU.Planes[i] = (PLANEPTR) AllocRaster(Width, Height);
	    if (BMU.Planes[i] == 0) {
		endGame("AllocRaster failure\n");
	    }
	}
    }
    else {
	InitBitMap(&BM, 1, Width, Height);
	InitBitMap(&BMU, 1, Width, Height);
	BM.Planes[0] = (PLANEPTR) AllocRaster(Width, Height);
	if (BM.Planes[0] == 0) {
	    endGame("AllocRaster failure\n");
	}
	BMU.Planes[0] = (PLANEPTR) AllocRaster(Width, Height);
	if (BMU.Planes[0] == 0) {
	    endGame("AllocRaster failure\n");
	}
    }

    Comment = AllocMem(80*(Height+2), MEMF_CLEAR);
    if (Comment == 0) {
	endGame("AllocMem failure\n");
    }

    RP.BitMap = &BM;
    RPU.BitMap = &BMU;

    /* write out the header */
    fprintf(File, ";------ included files ------\n");
    fprintf(File, "\n");
    fprintf(File, "	INCLUDE		\"exec/types.i\"\n");
    fprintf(File, "	INCLUDE		\"exec/nodes.i\"\n");
    fprintf(File, "	INCLUDE		\"libraries/diskfont.i\"\n");
    fprintf(File, "\n");
    fprintf(File, ";------ code to handle execution of this file ------\n");
    fprintf(File, "		moveq	#100,D0\n");
    fprintf(File, "		rts\n");
    fprintf(File, "\n");
    fprintf(File, ";------ DiskFontHeader structure ------\n");
    fprintf(File, "		dc.l	0		; ln_Succ\n");
    fprintf(File, "		dc.l	0		; ln_Pred\n");
    fprintf(File, "		dc.b	NT_FONT		; ln_Type\n");
    fprintf(File, "		dc.b	0		; ln_Pri\n");
    fprintf(File, "		dc.l	font%02dName	; ln_Name\n", Height);
    fprintf(File, "		dc.w	DFH_ID		; FileID\n");
    fprintf(File, "		dc.w	0		; Revision\n");
    fprintf(File, "		dc.l	0		; Segment\n");
    fprintf(File, "font%02dName:\n",  Height);
    fprintf(File, "		dcb.b	MAXFONTNAME,0	; Name\n");
    fprintf(File, "\n");
    fprintf(File, ";------ TextFont structure ------\n");
    fprintf(File, "		dc.l	0		; ln_Succ\n");
    fprintf(File, "		dc.l	0		; ln_Pred\n");
    fprintf(File, "		dc.b	NT_FONT		; ln_Type\n");
    fprintf(File, "		dc.b	0		; ln_Pri\n");
    fprintf(File, "		dc.l	font%02dName	; ln_Name\n", Height);
    fprintf(File, "		dc.l	0		; mn_ReplyPort\n");
    fprintf(File, "\t\tdc.w\t0\t\t; (graphics library internal use)\n");
    fprintf(File, "		dc.w	%d		; tf_YSize\n",
	    CTF->ctf_TF.tf_YSize);
    fprintf(File, "		dc.b	$%02lx		; tf_Style\n",
	    CTF->ctf_TF.tf_Style & 0x7f);
    fprintf(File, "		dc.b	$%02lx		; tf_Flags\n",
	    CTF->ctf_TF.tf_Flags & 0x7f);
    fprintf(File, "		dc.w	%d		; tf_XSize\n",
	    CTF->ctf_TF.tf_XSize);
    fprintf(File, "		dc.w	%d		; tf_Baseline\n",
	    CTF->ctf_TF.tf_Baseline);
    fprintf(File, "		dc.w	%d		; tf_BoldSmear\n",
	    CTF->ctf_TF.tf_BoldSmear);
    fprintf(File, "		dc.w	0		; tf_Accessors\n");
    fprintf(File, "		dc.b	%d		; tf_LoChar\n",
	    CTF->ctf_TF.tf_LoChar);
    fprintf(File, "		dc.b	%d		; tf_HiChar\n",
	    CTF->ctf_TF.tf_HiChar);
    fprintf(File, "		dc.l	font%02dData	; tf_CharData\n",
	    Height);
    fprintf(File, "		dc.w	FONT%02dMODULO	; tf_Modulo\n", Height);
    fprintf(File, "		dc.l	font%02dLoc	; tf_CharLoc\n",
	    Height);
    if (CTF->ctf_TF.tf_CharSpace)
	fprintf(File, "		dc.l	font%02dSpace	; tf_CharSpace\n",
		Height);
    else
	fprintf(File, "		dc.l	0		; tf_CharSpace\n");
    if (CTF->ctf_TF.tf_CharKern)
	fprintf(File, "		dc.l	font%02dKern	; tf_CharKern\n",
		Height);
    else
	fprintf(File, "		dc.l	0		; tf_CharKern\n");
    fprintf(File, "\n");
    if (CTF->ctf_TF.tf_Style & FSF_COLORFONT) {
	fprintf(File, "		dc.w	$%04lx		; ctf_Flags\n",
		CTF->ctf_Flags);
	fprintf(File, "		dc.b	%d		; ctf_Depth\n",
		CTF->ctf_Depth);
	fprintf(File, "		dc.b	%d		; ctf_FgColor\n",
		CTF->ctf_FgColor);
	fprintf(File, "		dc.b	%d		; ctf_Low\n",
		CTF->ctf_Low);
	fprintf(File, "		dc.b	%d		; ctf_High\n",
		CTF->ctf_High);
	fprintf(File, "		dc.b	$%02lx		; ctf_PlanePick\n",
		CTF->ctf_PlanePick);
	fprintf(File, "		dc.b	$%02lx		; ctf_PlaneOnOff\n",
		CTF->ctf_PlaneOnOff);
	if (CTF->ctf_ColorFontColors)
	    fprintf(File, "\t\tdc.l\tfont%02dColors\t; ctf_ColorFontColors\n",
		    Height);
	else
	    fprintf(File, "\t\tdc.l\t0		; ctf_ColorFontColors\n");
	for (i = 0; i < CTF->ctf_Depth; i++)
	    fprintf(File, "\t\tdc.l\tcolor%02dData%d	; ctf_CharData[%d]\n",
		    Height, i, i);
	for (; i < 8; i++)
	    fprintf(File, "\t	dc.l	0		; ctf_CharData[%d]\n",
		    i);
	if (CTF->ctf_ColorFontColors) {
	    fprintf(File, "\n");
	    fprintf(File, "font%02dColors:\n", Height);
	    fprintf(File, "\t	dc.w	0		; cfc_Reserved\n");
	    fprintf(File, "\t	dc.w	%d		; cfc_Count\n",
		    CTF->ctf_ColorFontColors->cfc_Count);
	    fprintf(File, "\t	dc.l	font%02dColorTable\n", Height);
	    fprintf(File, "\n");
	    fprintf(File, "font%02dColorTable:\n", Height);
	    for (i = 0; i < CTF->ctf_ColorFontColors->cfc_Count; i++)
		fprintf(File, "\t\tdc.w	$%04lx		; (color %d)\n",
			CTF->ctf_ColorFontColors->cfc_ColorTable[i], i);
	}
    }

    /* print hints for com2asm */
    fprintf(File, "\n;\014;   FONT DISPLAY\n;");
    Page = Height + 6;

    /* print character representations */
    undef = CTF->ctf_TF.tf_HiChar + 1;
    undefMore = 1;
    if (undef)
	undefMore = 2;
    else
	if (CTF->ctf_TF.tf_LoChar != 0)
	    undefMore = 2;

    if (undefMore == 2) {
	/* get extent of undefined character */
	TextExtent(&RPU, &undef, 1, &TEU);
	SetRast(&RPU, 0);
	Move(&RPU, X0, Y0);
	Text(&RPU, &undef, 1);
    }

    c = CTF->ctf_TF.tf_LoChar - 1;
    cMore = 1;
    i = 0;
    do {
	c++;
	printf("%02x", c);
	TextExtent(&RP, &c, 1, &TE);
	SetRast(&RP, 0);
	Move(&RP, X0, Y0);
	Text(&RP, &c, 1);

	if (undefMore == 2) {
	    /* check if this is the same as the undefined character */
	    if ((TE.te_Width == TEU.te_Width) &&
		    (TE.te_Extent.MaxX == TEU.te_Extent.MaxX) &&
		    (TE.te_Extent.MinX == TEU.te_Extent.MinX) &&
		    (TE.te_Extent.MaxY == TEU.te_Extent.MaxY) &&
		    (TE.te_Extent.MinY == TEU.te_Extent.MinY) &&
		    (TE.te_Height == TEU.te_Height)) {
		for (y = 0; y < Height; y++) {
		    for (x = 0; x < Width; x++) {
			if (ReadPixel(&RP, x, y) != ReadPixel(&RPU, x, y))
			    goto notUndef;
		    }
		}
		goto nextChar;
	    }
notUndef:;
	}

	/* determine width that this character needs for comment */
	if (TE.te_Extent.MaxX >= TE.te_Width)
	    k = TE.te_Extent.MaxX + 1;
	else
	    k = TE.te_Width;
	if (TE.te_Extent.MinX < 0) {
	    x0 = -TE.te_Extent.MinX;
	    k += x0;
	}
	else {
	    x0 = 0;
	}
	if (k < 5)
	    k = 5;			/* minimum char comment width */

	/* ensure room exists for comment */
	if (i + k > 72) {
	    flushComment();
	    i = 0;
	}

	/* set comment area to blank */
	cl = Comment+i;
	for (y = 0; y < Height+2; y++) {
	    for (x = 0; x < k; x++)
		cl[x] = ' ';
	    cl += 80;
	}

	/* label character */
	cl = Comment+i;
	if (cMore)
	    sprintf(cl, "$%02lx", c);
	else
	    sprintf(cl, "$--");
	cl[3] = ' ';
	cl[k-1] = '.';

	/* show character origin and width */
	cl += 80;
	if (TE.te_Width == 0)
	    cl[x0] = '|';
	else if (TE.te_Width == 1)
	    cl[x0] = 'X';
	else {
	    cl[x0] = '>';
	    cl[x0+TE.te_Width-1] = '<';
	}

	for (y = 0; y < Height; y++) {
	    cl += 80;
	    for (x = -x0; x <= TE.te_Extent.MaxX; x++) {
		if (j = ReadPixel(&RP, X0+x, y)) {
		    if (CTF->ctf_TF.tf_Style & FSF_COLORFONT)
			if (j < 10)
			    cl[x-TE.te_Extent.MinX] = '0'+j;
			else
			    cl[x-TE.te_Extent.MinX] = 'A'-10+j;
		    else
			cl[x-TE.te_Extent.MinX] = '*';
		}
	    }
	}
	i += k;

nextChar:
	if (cMore) {
	    if (c == CTF->ctf_TF.tf_HiChar) {
		c = undef - 1;
		cMore = 0;
		undefMore--;
	    }
	}
	else
	    undefMore--;
    }
	while (cMore || undefMore);

    flushComment();
    fprintf(File, "\n\n");

    printf("\nfont loc...");

    fprintf(File, "font%02dLoc:", Height);
    i = CTF->ctf_TF.tf_LoChar-1;
    longptr = (ULONG *) CTF->ctf_TF.tf_CharLoc;
    j = 0;
    do {
	if (j == 0)
	    fprintf(File, "\n\t	dc.l	");
	else fprintf(File, ",");
	fprintf(File, "$%08lx", *longptr++);
	j++;
	j &= 3;
	i++;
    }
	while (i <= CTF->ctf_TF.tf_HiChar);
    fprintf(File, "\n");

    if (CTF->ctf_TF.tf_CharSpace) {
	printf("space...");
	fprintf(File, "font%02dSpace:", Height);
	i = CTF->ctf_TF.tf_LoChar-1;
	wordptr = (WORD *) CTF->ctf_TF.tf_CharSpace;
	j = 0;
	do {
	    if (j == 0)
		fprintf(File, "\n\t	dc.w	");
	    else fprintf(File, ",");
	    if (*wordptr < 0)
		fprintf(File, "-%03d", -(*wordptr++));
	    else
		fprintf(File, "%04d", *wordptr++);
	    j++;
	    j &= 7;
	    i++;
	}
	    while (i <= CTF->ctf_TF.tf_HiChar);
	fprintf(File, "\n");
    }

    if (CTF->ctf_TF.tf_CharKern) {
	printf("kern...");
	fprintf(File, "font%02dKern:", Height);
	i = CTF->ctf_TF.tf_LoChar-1;
	wordptr = (WORD *) CTF->ctf_TF.tf_CharKern;
	j = 0;
	do {
	    if (j == 0)
		fprintf(File, "\n\t	dc.w	");
	    else fprintf(File, ",");
	    if (*wordptr < 0)
		fprintf(File, "-%03d", -(*wordptr++));
	    else
		fprintf(File, "%04d", *wordptr++);
	    j++;
	    j &= 7;
	    i++;
	}
	    while (i <= CTF->ctf_TF.tf_HiChar);
	fprintf(File, "\n");
    }

    if (CTF->ctf_TF.tf_Style & FSF_COLORFONT) {
	printf("color data .");
	for (i = 0; i < CTF->ctf_Depth; i++) {
	    for (j = 0; j < i; j++)
		if (CTF->ctf_CharData[i] == CTF->ctf_CharData[j]) break;
	    if (i == j) {
		if (CTF->ctf_TF.tf_CharData == CTF->ctf_CharData[i])
		    fprintf(File, "font%02dData:", Height);
		for (j = i; j < CTF->ctf_Depth; j++)
		    if (CTF->ctf_CharData[i] == CTF->ctf_CharData[j])
			fprintf(File, "\ncolor%02dData%d:", Height, j);
		printf("%d.", i);
		dataDump(CTF->ctf_CharData[i],
			CTF->ctf_TF.tf_YSize, CTF->ctf_TF.tf_Modulo/2);
	    }
	}
    }
    else {
	printf("data...");
	fprintf(File, "font%02dData:", Height);
	dataDump(CTF->ctf_TF.tf_CharData,
		CTF->ctf_TF.tf_YSize, CTF->ctf_TF.tf_Modulo/2);
    }

    fprintf(File, "\nFONT%02dMODULO	EQU	%d\n\n\tEND\n", Height,
	    CTF->ctf_TF.tf_Modulo);

    endGame(0);
}
