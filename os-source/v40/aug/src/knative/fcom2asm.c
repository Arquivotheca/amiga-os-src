#define	D(p)

/****** FCom2Asm *****************************************************
*
*   NAME
*       FCom2Asm - create an assembly file from font assembly comments
*
*********************************************************************/

#include	<proto/all.h>
#include	<stdio.h>
#include	<string.h>
#include	<graphics/text.h>

char *Prog;

FILE *InFile, *OutFile;

int ColorFontFlag;
int KernSpaceFlag;
int ByteWideFlag;

char LineBuffer[128];

char *Comment;

struct Definition {
    struct Definition *llink;
    struct Definition *rlink;
    WORD    code;	/* associated character code */
    WORD    progress;	/* progress code*/
    WORD    start;	/* relative to start of bits */
    WORD    end;	/* relative to start of bits */
    UWORD   width;	/* bit width */
    WORD    owidth;	/* width until overlap with next neighbor */
    char    pixels[2];	/* stored by columns! (y0 x0-n; y1, x0-n; ... ) */
} *Definitions[257] = { 0 };

#define	PROG_INIT	0
#define	PROG_INLIST	1

int Height;
int Width;
int Depth;
int Col;

char huge LMatch[256][257] = {0}; /* m < n */
char huge RMatch[256][257] = {0}; /* m < n */

short DefReloc[257];
short DefRelocEnd = 0;

short LocLoc[257];
short LocWidth[257];
short Kern[257];
short Space[257];

void
endGame(fmt, arg1, arg2, arg3)
char *fmt, *arg1, *arg2, *arg3;
{
    if (fmt) {
	printf(fmt, arg1, arg2, arg3);
	printf("usage: %s <asm-in> <asm-out> [0|1]\n", Prog);
	exit(10);
    }
    exit(0);
}

#define	GFE_FAIL	-32768

int
getFontElement(s, base)
char *s;
int base;
{
    char *b;
    int result;

    /* read in the font description up to desired element */
    while ((b = fgets(LineBuffer, sizeof(LineBuffer), InFile)) != NULL) {
	b = strchr(LineBuffer, '\n');
	if (b)
	    *b = '\0';
	b = strchr(LineBuffer, ';');
	if ((b) && (b[1] == ' ') && (strcmp(b+2, s) == 0)) {
	    if (base == 10) {
		result = strtol(LineBuffer+7, &b, 10);
		if (b == (LineBuffer+7))
		    result = GFE_FAIL;
	    }
	    else {
		/* base == 16 */
		result = strtol(LineBuffer+8, &b, 16);
		if (b == (LineBuffer+8))
		    result = GFE_FAIL;
	    }
	    strcat(LineBuffer, "\n");
	    break;
	}
	strcat(LineBuffer, "\n");
	fputs(LineBuffer, OutFile);
    }
    if (!b)
	endGame("ERROR: %s not found\n", s);
    D(("%s: %d 0x%x\n", s, result, result));
    return(result);
}


void
getDescriptions()
{
    struct Definition *def;
    short charCode, endPending, notEmpty, started, x, x0, xf, y;
    char *b, c, *t;

    x0 = 0;
    while (b = strchr(Comment+x0, '$')) {
	x0 = b - Comment;
	b++;
	if (strncmp(b, "--", 2) == 0)
	    charCode = 256;
	else
	    charCode = strtol(b, 0, 16);
	b = strchr(b, '.');
	if (b == 0)
	    endGame("ERROR: unterminated character description...\n%s",
		    Comment);
	xf = b - Comment;
	if (Definitions[charCode])
	    printf("Warning: duplicate definition for character $%x\n",
		    charCode);
	if ((def = (struct Definition *) malloc(sizeof(struct Definition) +
		((xf - x0 + 1) * Height))) == 0)
	    endGame("ERROR: Definition malloc failure\n");
	def->llink = 0;
	def->rlink = 0;
	def->code = charCode;
	def->progress = PROG_INIT;

	b = strchr(Comment+128+x0, '>');
	t = strchr(Comment+128+x0, 'X');
	if ((t) && ((!b) || (t < b))) {
	    def->start = t - Comment - x0 - 128;
	    def->end = def->start + 1;
	}
	else {
	    t = strchr(Comment+128+x0, '|');
	    if ((t) && ((!b) || (t < b))) {
		def->start = t - Comment - x0 - 128;
		def->end = def->start;
	    }
	    else {
		if (!b)
		    endGame("ERROR: character $%02x has no width start\n",
			    charCode);
		def->start = b - Comment - x0 - 128;
		b = strchr(Comment+128+x0, '<');
		if (!b)
		    endGame("ERROR: character $%02x has no width end\n",
			    charCode);
		def->end = b - Comment - x0 - 128;
		if (def->end > def->start)
		    def->end++;
		else
		    def->start++;
	    }
	}
	def->width = xf - x0 + 1;

	started = !KernSpaceFlag;
	endPending = 0;
	b = def->pixels;
	for (x = x0; x <= xf; x++) {
	    notEmpty = 0;
	    for (y = 0; y < Height; y++) {
		c = Comment[(y+2)*128+x];
		if (c != ' ') {
		    notEmpty = 1;
		    if (c == '*')
			b[y] = 1;
		    else
			if ((c >= '0') && (c <= 9))
			    b[y] = c - '0';
			else
			    b[y] = c - 'A'+10;
		}
		else b[y] = 0;
	    }
	    if (notEmpty)
		started = 1;
	    if (started) {
		b += Height;
		if (notEmpty)
		    endPending = 0;
		else
		    endPending++;
	    }
	    else {
		def->start--;
		def->end--;
		def->width--;
	    }
	}
	if (KernSpaceFlag)
	    def->width -= endPending;
	def->owidth = def->width;
	Definitions[charCode] = def;
	x0 = xf;
    }
}


void
genMatch(m, n)
short m;
short n;
{
    short match, maxMatch;
    char *bm, *bn;
    short i;

    if (Definitions[m] && Definitions[n]) {
	if (Definitions[m]->width <= Definitions[n]->width)
	    maxMatch = Definitions[m]->width;
	else
	    maxMatch = Definitions[n]->width;
	/* look for left match */
	for (match = maxMatch; match > 0; match--) {
	    bm = Definitions[m]->pixels;
	    bn = Definitions[n]->pixels +
		    ((Definitions[n]->width - match) * Height);
	    for (i = match*Height; i > 0; i--) {
		if (*bm++ != *bn++)
		    goto noLeftMatch;
	    }
	    LMatch[m][n] = match;
	    break;
noLeftMatch:;
	    if (ByteWideFlag)
		return;
	}
	/* look for right match */
	for (match = maxMatch; match > 0; match--) {
	    bm = Definitions[m]->pixels +
		    ((Definitions[m]->width - match) * Height);
	    bn = Definitions[n]->pixels;
	    for (i = match*Height; i > 0; i--) {
		if (*bm++ != *bn++)
		    goto noRightMatch;
	    }
	    RMatch[m][n] = match;
	    break;
noRightMatch:;
	}
    }
}


int
notCircular(lCode, rCode)
short lCode, rCode;
{
    struct Definition *def;

    def = Definitions[lCode]->llink;
    while (def) {
	if (def->code == rCode)
	    return(0);
	def = def->llink;
    }
    def = Definitions[rCode]->rlink;
    while (def) {
	if (def->code == lCode)
	    return(0);
	def = def->rlink;
    }
    return(1);
}


void
fillTable(charCode, defCode)
{
    short i, x;

    x = 0;
    for (i = 0; i < DefRelocEnd; i++) {
	if (Definitions[DefReloc[i]]->code == defCode)
	    break;
	x += Definitions[DefReloc[i]]->owidth;
    }
    LocLoc[charCode] = x;

    LocWidth[charCode] = Definitions[defCode]->width;

    if (KernSpaceFlag) {
	Kern[charCode] = -Definitions[defCode]->start;
	Space[charCode] = Definitions[defCode]->end;
    }
}


void
genLong(w1, w2)
UWORD w1, w2;
{
    if (Col == 0)
	fprintf(OutFile, "\n\t\tdc.l\t");
    else
	fprintf(OutFile, ",");
    fprintf(OutFile, "$%04lx%04lx", w1, w2);
    Col++;
    Col &= 3;
}


void
genDecimal(d)
short d;
{
    if (Col == 0)
	fprintf(OutFile, "\n\t\tdc.w\t");
    else fprintf(OutFile, ",");
    if (d < 0)
	fprintf(OutFile, "-%03d", -d);
    else
	fprintf(OutFile, "%04d", d);
    Col++;
    Col &= 7;
}


void
genWord(w)
UWORD w;
{
    if (Col == 0)
	fprintf(OutFile, "\n\t\tdc.w\t");
    else fprintf(OutFile, ",");
    fprintf(OutFile, "$%04x", w);
    Col++;
    Col &= 7;
}


main(argc, argv)
int argc;
char *argv[];
{
    struct Definition *def;
    char *b;
    short loChar, hiChar, i, j, k, mod, x, y, z;
    UWORD t;

    if (argc < 2) {
	endGame("ERROR: wrong number of parameters\n");
    }

    InFile = fopen(argv[1], "r");
    if (InFile == 0) {
	endGame("ERROR: open failed for font file \"%s\"\n", argv[1]);
    }

    OutFile = fopen(argv[2], "w");
    if (OutFile == 0) {
	endGame("ERROR: open failed for asm-out file \"%s\"\n", argv[2]);
    }

    /* read in the font description up thru needed elements */
    Height = getFontElement("tf_YSize", 10);
    if (Height == GFE_FAIL)
	endGame("ERROR: looking for tf_YSize in...\n%s\n", LineBuffer);
    fputs(LineBuffer, OutFile);
    i = getFontElement("tf_Style", 16);
    if (i == GFE_FAIL)
	endGame("ERROR: looking for tf_Style in...\n%s\n", LineBuffer);
    fputs(LineBuffer, OutFile);
    Width = getFontElement("tf_XSize", 10);
    if (Width == GFE_FAIL)
	endGame("ERROR: looking for tf_XSize in...\n%s\n", LineBuffer);
    fputs(LineBuffer, OutFile);
    ColorFontFlag = i & FSF_COLORFONT;
    loChar = getFontElement("tf_LoChar", 10);
    if (loChar == GFE_FAIL)
	endGame("ERROR: looking for tf_LoChar in...\n%s\n", LineBuffer);
    fputs(LineBuffer, OutFile);
    hiChar = getFontElement("tf_HiChar", 10);
    if (hiChar == GFE_FAIL)
	endGame("ERROR: looking for tf_HiChar in...\n%s\n", LineBuffer);
    fputs(LineBuffer, OutFile);

    /* get defaults for KernSpaceFlag */
    KernSpaceFlag = getFontElement("tf_CharSpace", 10);
    if (KernSpaceFlag != getFontElement("tf_CharKern", 10))
	endGame("ERROR: specification of Kern & Space not consistant\n");

    if (ColorFontFlag) {
	Depth = getFontElement("ctf_Depth", 10);
	if (Depth == GFE_FAIL)
	    endGame("ERROR: looking for ctf_Depth in...\n%s\n", LineBuffer);
	fputs(LineBuffer, OutFile);
    }
    else
	Depth = 1;

    for (i = 3; i < argc; i++) {
	b = argv[i];
	while (*b) {
	    switch (*b++) {
	      case '0':
		if (KernSpaceFlag)
		    printf("Overriding KernSpaceFlag to OFF\n");
		KernSpaceFlag = 0;
		break;
	      default:
		if (!KernSpaceFlag)
		    printf("Overriding KernSpaceFlag to ON\n");
		KernSpaceFlag = 1;
		break;
	    }
	}
    }
    if ((KernSpaceFlag == 0) && (Width < 0))
	fprintf(stderr,
	"WARNING: Negative XSize %d illegal for KernSpaceFlag OFF under V36\n",
		Width);

    if (KernSpaceFlag) {
	fprintf(OutFile, "\t\tdc.l\tfont%02ldSpace\t; tf_CharSpace\n", Height);
	fprintf(OutFile, "\t\tdc.l\tfont%02ldKern\t; tf_CharKern\n", Height);
    }
    else {
	fprintf(OutFile, "\t\tdc.l\t0\t\t; tf_CharSpace\n");
	fprintf(OutFile, "\t\tdc.l\t0\t\t; tf_CharKern\n");
    }

    /* read in the font description up to the comments */
    while ((b = fgets(LineBuffer, sizeof(LineBuffer), InFile)) != NULL) {
	fputs(LineBuffer, OutFile);
	if (strncmp(LineBuffer, ";\014;   FONT DISPLAY", 18) == 0)
	    break;
    }
    if (!b)
	endGame("ERROR: FONT DISPLAY comment not found\n");

    Comment = (char *) malloc(128*(Height+2));
    if (Comment == 0)
	endGame("ERROR: Comment malloc failure\n");

    printf("%s YSize %d, character range %d-%d, kern & space ", argv[1], Height,
	     loChar, hiChar);
    if (KernSpaceFlag)
	printf("on");
    else
	printf("off");
    if (ColorFontFlag)
	printf(", color");
    printf("\n");

    /* gather the character definitions */
    while ((b = fgets(LineBuffer, sizeof(LineBuffer), InFile)) != NULL) {
	if (LineBuffer[0] != ';')
	    break;			/* end of character definitions */
	fputs(LineBuffer, OutFile);
	if (strncmp(LineBuffer, ";   $", 5) == 0) {
	    /* a group of character comments */
	    for (i = 0; i < Height+2; i++) {
		strcpy(Comment+(i*128), LineBuffer);
		if (fgets(LineBuffer, sizeof(LineBuffer), InFile) == NULL)
		    endGame("ERROR: Unexpected EOF in FONT DISPLAY\n");
		if (LineBuffer[0] != ';')
		    endGame("ERROR: Unexpected end in FONT DISPLAY\n");
		fputs(LineBuffer, OutFile);
	    }
	    getDescriptions();
	}
    }

    /* all the descriptions have been read in */

    /* check for descriptions outside range of loChar-hiChar */
    for (i = 0; i < loChar; i++)
	if (Definitions[i])
	   endGame("Warning: definition for character $%02x ignored\n", i);
    for (i = hiChar + 1; i < 256; i++)
	if (Definitions[i])
	   endGame("Warning: definition for character $%02x ignored\n", i);

    if (!Definitions[256])
	endGame("ERROR: no definition for undefined character\n");

    /* determine ByteWideFlag */
    ByteWideFlag = 1;
    if (KernSpaceFlag)
	ByteWideFlag = 0;
    else {
	for (i = loChar; i <= hiChar; i++) {
	    if ((Definitions[i]) && (
		    (Definitions[i]->width != 8) ||
		    (Definitions[i]->end != 8))) {
		ByteWideFlag = 0;
		break;
	    }
	}
    }

    /* generate matches */
    for (i = loChar; i <= hiChar; i++) {
	for (j = i+1; j <= hiChar; j++)
	    genMatch(i, j);
	genMatch(i, 256);
    }

    if (ByteWideFlag) {
	printf("Byte Wide Font\n");
	/* link definitions in roughly character order */
	for (i = loChar; i <= hiChar; i++) {
	    if ((Definitions[i]) && (Definitions[i]->progress < PROG_INLIST)) {
		DefReloc[DefRelocEnd++] = i;
		Definitions[i]->progress = PROG_INLIST;
		/* look for duplicates of this new definition */
		j = i;
		for (k = i+1; k <= hiChar; k++) {
		    if (LMatch[i][k] == Definitions[i]->width) {
			printf("  %02lx matches %02lx\n", j, k);
			Definitions[j]->owidth = 0;
			DefReloc[DefRelocEnd++] = k;
			Definitions[k]->progress = PROG_INLIST;
			j = k;
		    }
		}
	    }
	}
    }
    else {
	/* generate links */
	for (;;) {
	    /* find next overlap distance */
	    t = 0;
	    for (i = 0; i < 256; i++)
		for (j = i+1; j < 258; j++) {
		    if (LMatch[i][j] > t)
			t = LMatch[i][j];
		    if (RMatch[i][j] > t)
			t = RMatch[i][j];
		}
	    if (t == 0)
		break;

	    /* build neighbors for this overlap distance */
	    for (i = 0; i < 256; i++) {
		for (j = i+1; j <= 256; j++) {
		    if (LMatch[i][j] == t) {
			if ((Definitions[i]->llink == 0) &&
				(Definitions[j]->rlink == 0) &&
				notCircular(j, i)) {
			    Definitions[i]->llink = Definitions[j];
			    Definitions[j]->rlink = Definitions[i];
			    Definitions[j]->owidth -= t;
			}
			LMatch[i][j] = 0;
		    }
		    if (RMatch[i][j] == t) {
			if ((Definitions[i]->rlink == 0) &&
				(Definitions[j]->llink == 0) &&
				notCircular(i, j)) {
			    Definitions[i]->rlink = Definitions[j];
			    Definitions[j]->llink = Definitions[i];
			    Definitions[i]->owidth -= t;
			}
			RMatch[i][j] = 0;
		    }
		}
	    }
	}
	
	/* fill relocation table */
	for (i = loChar; i <= hiChar; i++) {
	    if ((Definitions[i]) && (Definitions[i]->progress < PROG_INLIST)) {
		def = Definitions[i];
		while (def->llink)
		    def = def->llink;
		while (def) {
		    DefReloc[DefRelocEnd++] = def->code;
		    def->progress = PROG_INLIST;
		    def = def->rlink;
		}
	    }
	}
    }

    if (Definitions[256]->progress < PROG_INLIST) {
	DefReloc[DefRelocEnd++] = 256;
	Definitions[256]->progress = PROG_INLIST;
    }

    /* fill in the tables */
    for (i = loChar; i <= hiChar; i++) {
	if (Definitions[i])
	    fillTable(i, i);
	else
	    fillTable(i, 256);
    }
    fillTable(256, 256);

    /* generate tf_CharLoc */
    fprintf(OutFile, "font%02ldLoc:", Height);
    Col = 0;
    for (i = loChar; i <= hiChar; i++) {
	genLong(LocLoc[i], LocWidth[i]);
    }
    genLong(LocLoc[256], LocWidth[256]);

    if (KernSpaceFlag) {
	fprintf(OutFile, "\n\nfont%02ldSpace:", Height);
	Col = 0;
	for (i = loChar; i <= hiChar; i++) {
	    genDecimal(Space[i]);
	}
	genDecimal(Space[256]);
	fprintf(OutFile, "\n\nfont%02ldKern:", Height);
	Col = 0;
	for (i = loChar; i <= hiChar; i++) {
	    genDecimal(Kern[i]);
	}
	genDecimal(Kern[256]);
    }

    /* generate data for defined characters */
    fprintf(OutFile, "\n\nfont%02ldData:", Height);

    for (z = 0; z < Depth; z++) {
	if (ColorFontFlag)
	    fprintf(OutFile, "\n\ncolor%02ldData%ld:", Height, z);
	for (y = 0; y < Height; y++) {
	    Col = 0;
	    t = 0;
	    x = 0;
	    mod = 0;
	    for (i = 0; i < DefRelocEnd; i++) {
		def = Definitions[DefReloc[i]];
		b = def->pixels + y;
		for (j = 0; j < def->owidth; j++) {
		    t <<= 1;
		    t |= (*b >> z) & 1;
		    if (++x == 16) {
			genWord(t);
			t = 0;
			x = 0;
			mod++;
		    }
		    b += Height;
		}
	    }
	    if (x) {
		t <<= 16-x;
		genWord(t);
		mod++;
	    }
	    fprintf(OutFile, "\n");
	}
    }

    fprintf(OutFile, "\nFONT%02ldMODULO\tEQU\t%d\n\n\tEND\n", Height, mod*2);

    endGame(0);
}
