#include	<stdio.h>
#include	<strings.h>

char *Prog;
char  LibName[32];
char  SysName[32];
char  BaseName[32];
char  FunctionName[32];
char  FileName[64];
char  ProtectedRegs[48];
int   AbsBase;			/* absolute base address (i.e. 4 for exec) */
int   Bias;			/* current bias -6n offset in .fd file */
short Public;			/* true if ##public, false if ##private */

FILE *FDFile;			/* input library .fd file handle */
FILE *AFile,			/* output library .asm file */
     *BFile,			/* output library BCPL header file */
     *PFile;			/* output library lattice #pragma file */
FILE *MCFile,			/* output RAM/system/Makefile */
     *MRFile;			/* output ROM/system/Makefile */
FILE *CFile,			/* output function C ram interface .asm file */
     *RFile;			/* output function C rom interface .asm file */

#define	TOKEN_ABSBASE		1
#define	TOKEN_ASMSRC		2
#define	TOKEN_BASE		3
#define	TOKEN_BIAS		4
#define	TOKEN_CSRC		5
#define	TOKEN_PUBLIC		6
#define	TOKEN_PRIVATE		7
#define	TOKEN_END		8

char InBuffer[256];

char *
LSearch(item, table, tableSize, elementSize, compareFunction)
char *item, *table;
int tableSize, elementSize, (*compareFunction)();
{
    int i;
    for (i = 0; i < tableSize; i++) {
	if ((*compareFunction)(item, table) == 0) return(table);
	table += elementSize;
    }
    return(NULL);
}

struct KeyWord {
    char *Word;
    int Key;
};

struct KeyWord Tokens[] = {
    { "##asmsrc", TOKEN_ASMSRC },
    { "##csrc", TOKEN_CSRC },
    { "##public", TOKEN_PUBLIC },
    { "##private", TOKEN_PRIVATE },
    { "##bias", TOKEN_BIAS },
    { "##base", TOKEN_BASE },
    { "##end", TOKEN_END },
    { "##absbase", TOKEN_ABSBASE }
};

int
TokenCmp(s, t)
char *s;
struct KeyWord *t;
{
    return( strcmp( s, t->Word));
}

int
NextToken(s)
char **s;
{
    struct KeyWord *t;
    char *z;

    z = index(*s, ' ');
    if (z != 0) *z = '\0';

    t = (struct KeyWord *) LSearch(*s, Tokens,
	    sizeof(Tokens)/sizeof(struct KeyWord), sizeof(struct KeyWord),
	    TokenCmp);

    if (t == NULL) return(0);
    
    *s += strlen(*s) + 1;
    return(t->Key);
}

int
NextInt(s)
char **s;
{
    char *z;
    int i;

    i = 0;

    if (sscanf(*s, "%d", &i)) {
	while ((**s) && (**s == ' ')) (*s)++;
	z = index(*s, ' ');
	if (z != NULL) *s = z;
    }

    return(i);
}

char *
GetLine()
{
    char *s, *z;

    s = fgets(InBuffer, sizeof(InBuffer), FDFile);
    if (s) {
	while (z = rindex(s, '\n')) *z = 0;
	while (z = rindex(s, '\r')) *z = 0;
    }
    return(s);
}

EndGame(format, arg1, arg2)
char *format, *arg1, *arg2;
{
    if (format) fprintf(stderr, format, arg1, arg2);
    if (FDFile) fclose(FDFile);
    if (AFile) fclose(AFile);
    if (BFile) fclose(BFile);
    if (PFile) fclose(PFile);
    if (CFile) fclose(CFile);
    if (RFile) fclose(RFile);
    if (MCFile) fclose(MCFile);
    if (MRFile) fclose(MRFile);
    if (format) exit(100);
    exit(0);
}

main(argc, argv)
int argc;
char *argv[];
{
    char *q, *r, *s, *t;
    char aP[8], dP[8], a6;		/* register flags */
    char regError[6];			/* optimization errors */
    short lvoCount;
    unsigned long pragmaWord;
    short pragmaCount, pragmaOK;
    int i, j;

    Prog = *argv++;
    while (--argc > 0) {
	FDFile = AFile = BFile = PFile = CFile = RFile = MCFile = MRFile = NULL;
	/* get the library name */
	strcpy(LibName, *argv);
	s = rindex(LibName, '.');
	if (s == NULL)
	    EndGame("%s: ERROR  .fd file name \"%s\" ill formed\n",
		    Prog, LibName);
	
	/* get the system name */
	strcpy(SysName, *argv);
	t = rindex(SysName, '.');
	*t = '\0';
	t = rindex(SysName, '_');
	if (t == NULL)
	    EndGame("%s: ERROR  .fd file name \"%s\" ill formed\n",
		    Prog, LibName);
	*t = '\0';

	/* open the .fd file */
	FDFile = fopen(LibName, "r");
	if (FDFile == 0)
	    EndGame("%s: ERROR  cannot open file \"%s\"\n",
		    Prog, LibName);

	/* open the .asm file */
	strcpy(FileName, "LVO/");
	strcat(FileName, *argv++);
	t = rindex(FileName, '.');
	strcpy(t, ".asm");
	AFile = fopen(FileName, "w");
	if (AFile == 0)
	    EndGame("%s: ERROR  cannot create file \"%s\"\n",
		    Prog, FileName);
	/* open the .b file */
	strcpy(FileName, "HDR/");
	r = FileName+4;
	t = SysName;
	do {
	    *r++ = *t & 0x5f;
	}
	    while (*t++);
	strcat(FileName, "LIBHDR");
	BFile = fopen(FileName, "w");
	if (BFile == 0)
	    EndGame("%s: ERROR  cannot create file \"%s\"\n",
		    Prog, FileName);
	/* open the pragma file */
	strcpy(FileName, "PRAGMAS/");
	strcat(FileName, SysName);
	strcat(FileName, "_pragmas.h");
	PFile = fopen(FileName, "w");
	if (PFile == 0)
	    EndGame("%s: ERROR  cannot create file \"%s\"\n",
		    Prog, FileName);

	/* open the RAM Makefile file */
	strcpy(FileName, "RAM/");
	strcat(FileName, SysName);
	strcat(FileName, "/Makefile");
	MCFile = fopen(FileName, "w");
	if (MCFile == 0)
	    EndGame("%s: ERROR  cannot create file \"%s\"\n",
		    Prog, FileName);

	/* open the ROM Makefile file */
	FileName[1] = 'O';		/* ROM/ */
	MRFile = fopen(FileName, "w");
	if (MRFile == 0)
	    EndGame("%s: ERROR  cannot create file \"%s\"\n",
		    Prog, FileName);

	/* remove tail from LibName */
	*s = '\0';
	
	/* initialize system files */
	lvoCount = 0;
	fprintf(AFile, "*** DO NOT EDIT: FILE BUILT AUTOMATICALLY\n");
	fprintf(AFile, "*** %s.asm function offsets\n", LibName);
	fprintf(AFile, "	IDNT	%s_LVO\n", SysName);

	fprintf(BFile, "MANIFEST\n$(\n");

	fprintf(MCFile,
    "######################################################################\n");
	fprintf(MCFile, "MAKEMETA=	../../../../tools/makemeta\n");
	fprintf(MCFile, "\n");
	fprintf(MCFile, "OBJS=");

	fprintf(MRFile,
    "######################################################################\n");
	fprintf(MRFile, "MAKEMETA=	../../../../tools/makemeta\n");
	fprintf(MRFile, "\n");
	fprintf(MRFile, "OBJS=");

	/* parse the .fd file */
	AbsBase = -1;
	BaseName[0] = BaseName[1] = '\0';
	Bias = -6;
	Public = 0;

	while (s = GetLine()) {
	    switch (*s) {
		case '*':		/* comment */
		case ';':
		    /* write pragma file comment */
		    fprintf(PFile, "/*%s*/\n", s+1);
		    break;
		case '#':		/* keyword possibility */
		    /* search for keyword */
		    switch (NextToken(&s)) {
			case TOKEN_ABSBASE:
			    AbsBase = NextInt(&s);
			    break;
			case TOKEN_BASE:
			    strcpy(BaseName, s);
			    break;
			case TOKEN_BIAS:
			    i = Bias;
			    Bias = -NextInt(&s);
			    if (Bias > i)
				EndGame("%s: ERROR  bias overlap: %ld\n",
				    Prog, i);
			    break;
			case TOKEN_PUBLIC:
			    Public = 1;
			    break;
			case TOKEN_PRIVATE:
			    Public = 0;
			    break;
			case TOKEN_END:
			    AbsBase = -1;
			    BaseName[0] = '\0';
			    break;
			default:
			    EndGame("%s: ERROR  illegal keyword line \"%s\"\n",
				    Prog, s);
		    }
		    break;
		default:
		    /* check for alpha function name */
		    if ((*s < 'A') || ((*s > 'Z') && (*s < 'a')) || (*s > 'z'))
			EndGame("%s: ERROR  illegal function line \"%s\"\n",
				Prog, s);

		    /* look for function name termination */
		    t = index(s, '(');
		    if (t == NULL)
			EndGame("%s: ERROR  missing function ( in \"%s\"\n",
				Prog, s);
		    r = index(t, ')');
		    if (r == NULL)
			EndGame("%s: ERROR  missing function ) in \"%s\"\n",
				Prog, s);

		    /* look for register arguments */
		    r = index(r, '(');
		    if (r != NULL) {
			q = index(++r, ')');
			if (q == NULL)
			    EndGame("%s: ERROR  missing register ) in \"%s\"\n",
				Prog, s);
			*q = '\0';
			i = q - r;
			if (i == 0) r = NULL;
			else
			    if ((i % 3) != 2)
				EndGame("%s: ERROR  bad registers \"%s\"\n",
					Prog, s);
		    }

		    *t = '\0';
		    strcpy(FunctionName, s);

		    /* open C interface files */
		    strcpy(FileName, "ROM/");
		    strcat(FileName, SysName);
		    strcat(FileName, "/");
		    strcat(FileName, FunctionName);
		    strcat(FileName, ".asm");
		    RFile = fopen(FileName, "w");
		    if (RFile == 0)
			EndGame("%s: ERROR  cannot create file \"%s\"\n",
				Prog, FileName);
		    /* add this to the Makefile */
		    fprintf(MRFile, "\\\n	%s.obj", FunctionName);

		    if (Public) {
			FileName[1] = 'A';		/* RAM/ */
			CFile = fopen(FileName, "w");
			if (CFile == 0)
			    EndGame("%s: ERROR  cannot create file \"%s\"\n",
				    Prog, FileName);
			/* add this to the Makefile */
			fprintf(MCFile, "\\\n	%s.obj", FunctionName);
		    }
		    else
			CFile = 0;

		    /* write C interface files */
		    fprintf(RFile,
			    "*** DO NOT EDIT: FILE BUILT AUTOMATICALLY\n");
		    fprintf(RFile, "*** %s rom interface\n", FunctionName);
		    if (r != NULL)
			fprintf(RFile, "*** (%s)\n", t+1);
		    if (BaseName[0] != '\0')
			fprintf(RFile, "	XREF	%sOffset\n", BaseName);
		    fprintf(RFile, "	SECTION	%s\n", SysName);
		    fprintf(RFile, "	XDEF	_%s\n", FunctionName);
		    fprintf(RFile, "_%s:\n", FunctionName);

		    if (CFile) {
			fprintf(CFile,
				"*** DO NOT EDIT: FILE BUILT AUTOMATICALLY\n");
			fprintf(CFile, "*** %s ram interface\n", FunctionName);
			if (r != NULL)
			    fprintf(CFile, "*** (%s)\n", t+1);
			if (BaseName[0] != '\0')
			    fprintf(CFile, "	XREF	%s\n", BaseName);
			fprintf(CFile, "	SECTION	%s\n", SysName);
			fprintf(CFile, "	XDEF	_%s\n", FunctionName);
			fprintf(CFile, "_%s:\n", FunctionName);
		    }

		    pragmaWord = pragmaCount = 0;
		    pragmaOK = Public;
		    /*     save protected registers and a6 */
		    if (r) {
			for (j = 0; j < 8; j++) {
			    aP[j] = dP[j] = 0;
			}
			j = 0;
			do {
			    if ((r[j] | 0x20) == 'a') aP[r[j+1]-'0'] = 1;
			    else dP[r[j+1]-'0'] = 1;
			    j += 3;
			}
			    while (r[j-1] != '\0');
			a6 = aP[6];		/* save a6 from parameters */
			aP[6] = 1;
			i = 4;
			t = ProtectedRegs;
			for (j = 2; j < 8; j++) {
			    if (dP[j]) {
				*t++ = '/';
				*t++ = 'd';
				*t++ = '0'+j;
				i += 4;
			    }
			}
			for (j = 2; j < 8; j++) {
			    if (aP[j]) {
				*t++ = '/';
				*t++ = 'a';
				*t++ = '0'+j;
				i += 4;
			    }
			}
			*t = '\0';
		    }
		    else {
			i = 8;
			strcpy(ProtectedRegs, "/a6");
		    }
		    if (strlen(ProtectedRegs) > 3) {
			fprintf(RFile, "		movem.l	%s,-(a7)\n",
				ProtectedRegs+1);
			if (CFile)
			    fprintf(CFile, "		movem.l	%s,-(a7)\n",
				    ProtectedRegs+1);
		    }
		    else {
			fprintf(RFile, "		move.l	a6,-(a7)\n");
			if (CFile)
			    fprintf(CFile, "		move.l	a6,-(a7)\n");
		    }
		    if (a6 == 0) {
			/* check for valid BaseName */
			if ((BaseName[0] == '\0') && (AbsBase < 0))
			    EndGame("%s: ERROR  no ##base for \"%s\"\n",
				    Prog, s);
			if (BaseName[0] != '\0') {
			    fprintf(RFile, "\t\tmove.l\t%sOffset(a6),a6\n",
				    BaseName);
			    if (CFile)
				fprintf(CFile, "\t\tmove.l\t%s,a6\n", BaseName);
			}
			else {
			    fprintf(RFile, "\t\tmove.l\t$%lx,a6\n", AbsBase);
			    if (CFile)
				fprintf(CFile, "\t\tmove.l\t$%lx,a6\n",
					AbsBase);
			}
		    }

		    /*     fix up unspecified multiple registers */
		    if (r) {
			j = 0;
			while (r+j+2 != q) {
			    /* there are >=2 register specifications left */
			    if (
				    (r[j+2] == '/') &&
				    ((((r[j] | 0x20) == (r[j+3] | 0x20)) &&
				    (r[j+1] > r[j+4])) ||
				    (((r[j] | 0x20) == 'a') &&
				    ((r[j+3] | 0x20) == 'd')))) {
				strncpy(regError, r+j, 5);
				regError[5] = '\0';
				printf(
				"%s %s >>> ILLEGAL movem <<< \"... %s ...\"\n",
					LibName, FunctionName, regError);
				r[j+2] = ',';
			    }
			    else if (
				    (r[j+2] != '/') &&
				    ((((r[j] | 0x20) == (r[j+3] | 0x20)) &&
				    (r[j+1] < r[j+4])) ||
				    (((r[j] | 0x20) == 'd') &&
				    ((r[j+3] | 0x20) == 'a')))) {
				strncpy(regError, r+j, 5);
				regError[5] = '\0';
				printf(
				"%s %s need to optimize movem \"... %s ...\"\n",
					LibName, FunctionName, regError);
				r[j+2] = '/';
			    }
			    j += 3;
			}

			/* gather this entry into pragma word */
			j = 0;
			do {
			    if ((r[j] | 0x20) == 'a') {
				pragmaWord |= ((r[j+1]-'0'+8) & 0xf) <<
					(pragmaCount++ * 4);
				if (r[j+1] >= '6')
				    pragmaOK = 0;
			    }
			    else
				pragmaWord |= ((r[j+1]-'0') & 0xf) <<
					(pragmaCount++ * 4);
			    j += 3;
			}
			    while (r[j-1] != '\0');
		    }


		    /*     copy parameters into registers */
		    while (r) {
			/* look for multiple registers */
			for (j = 0; r[j+2] == '/'; j += 3) {
			    r[j] |= 0x20;		/* make lower case */
			}
			r[j] |= 0x20;		/* make lower case */
			r[j+2] = '\0';
			if (j != 0) {
			    /* move multiple register */
			    fprintf(RFile, "\t\tmovem.l\t%ld(a7),%s\n",
				    i, r);
			    if (CFile)
				fprintf(CFile, "\t\tmovem.l\t%ld(a7),%s\n",
					i, r);
			}
			else {
			    /* move single register */
			    fprintf(RFile, "\t\tmove.l\t%ld(a7),%s\n",
				    i, r);
			    if (CFile)
				fprintf(CFile, "\t\tmove.l\t%ld(a7),%s\n",
					i, r);
			}
			if ((r+j+2) == q) r = NULL;
			else {
			    i += ((j/3)*4) + 4;
			    r += j+3;
			}
		    }

		    fprintf(RFile, "\t	jsr	%ld(a6)\n", Bias);
		    if (CFile)
			fprintf(CFile, "\t	jsr	%ld(a6)\n", Bias);
		    if (strlen(ProtectedRegs) > 3) {
			fprintf(RFile, "		movem.l	(a7)+,%s\n",
				ProtectedRegs+1);
			if (CFile)
			    fprintf(CFile, "		movem.l	(a7)+,%s\n",
				    ProtectedRegs+1);
		    }
		    else {
			fprintf(RFile, "		move.l	(a7)+,a6\n");
			if (CFile)
			    fprintf(CFile, "		move.l	(a7)+,a6\n");
		    }
		    fprintf(RFile, "		rts\n");
		    fprintf(RFile, "	END\n");
		    fclose(RFile);
		    if (CFile) {
			fprintf(CFile, "		rts\n");
			fprintf(CFile, "	END\n");
			fclose(CFile);
		    }
		    CFile = RFile = NULL;


		    /* write this entry into the library files */
		    if ((lvoCount & 0x7f) == 0) {
			/* create a new section every 128 names so as to */
			/* not overflow BLINK 5.04's 150 symbol limit */
			if (lvoCount) {
			    /* generate something that looks like code so */
			    /* that the symbols associated with the previous */
			    /* section get flushed by the assembler */
			    fprintf(AFile, "	OFFSET	0\n");
			    fprintf(AFile, "_LVO%d	ds.w	1\n");
			}
			fprintf(AFile, "	SECTION	_LVO%d\n", lvoCount>>7);
		    }
		    lvoCount++;
		    fprintf(AFile, "	XDEF	_LVO%s\n", FunctionName);
		    fprintf(AFile, "_LVO%s EQU	%ld\n", FunctionName, Bias);

		    fprintf(BFile, "    e.%s = %ld\n", FunctionName, Bias);


		    /* write pragma file */
		    if (pragmaCount > 6) {
			pragmaOK = 0;
			pragmaCount = 7;
		    }
		    pragmaWord = (pragmaWord << 8) | pragmaCount; 
		    if (strcmp(BaseName, "_SysBase") == 0)
			if (pragmaOK)
			    fprintf(PFile, "#pragma syscall %s %x %lx\n",
				    FunctionName, -Bias, pragmaWord);
			else
			    fprintf(PFile, "/*pragma syscall %s %x %lx*/\n",
				    FunctionName, -Bias, pragmaWord);
		    else if (BaseName[1])
			if (pragmaOK)
			    fprintf(PFile, "#pragma libcall %s %s %x %lx\n",
				    BaseName+1, FunctionName, -Bias,
				    pragmaWord);
			else
			    fprintf(PFile, "/*pragma libcall %s %s %x %lx*/\n",
				    BaseName+1, FunctionName, -Bias,
				    pragmaWord);
		    else
			fprintf(PFile, "/*pragma libcall  %s %x %lx*/\n",
				FunctionName, -Bias, pragmaWord);


		    /* decrement Bias */
		    Bias -= 6;
		    break;
	    }
	}
	fclose(FDFile);
	fprintf(AFile, "	END\n");
	fclose(AFile);
	fprintf(BFile, "$)\n");
	fclose(BFile);
	fclose(PFile);
	fprintf(MCFile, "\n\n");
	fprintf(MCFile, "lib.timestamp:	asm.timestamp\n");
	fprintf(MCFile, "	make objs ${MFLAGS} ${MARGS}\n");
	fprintf(MCFile, "	cat *.obj >../%s.lib\n", SysName);
	fprintf(MCFile, "	rm -f *.obj\n");
	fprintf(MCFile, "	touch lib.timestamp\n");
	fprintf(MCFile, "\n");
	fprintf(MCFile, "asm.timestamp:\n");
	fprintf(MCFile, "\n");
	fprintf(MCFile, "objs:		${OBJS}\n");
	fprintf(MCFile, "	rm -f *.asm\n");
	fprintf(MCFile, "	touch asm.timestamp\n");
	fprintf(MCFile, "\n");
	fprintf(MCFile, ".INCLUDE=${MAKEMETA}\n");
	fclose(MCFile);
	fprintf(MRFile, "\n\n");
	fprintf(MRFile, "lib.timestamp:	asm.timestamp\n");
	fprintf(MRFile, "	make objs ${MFLAGS} ${MARGS}\n");
	fprintf(MRFile, "	cat *.obj >../%s.lib\n", SysName);
	fprintf(MRFile, "	rm -f *.obj\n");
	fprintf(MRFile, "	touch lib.timestamp\n");
	fprintf(MRFile, "\n");
	fprintf(MRFile, "asm.timestamp:\n");
	fprintf(MRFile, "\n");
	fprintf(MRFile, "objs:		${OBJS}\n");
	fprintf(MRFile, "	rm -f *.asm\n");
	fprintf(MRFile, "	touch asm.timestamp\n");
	fprintf(MRFile, "\n");
	fprintf(MRFile, ".INCLUDE=${MAKEMETA}\n");
	fclose(MRFile);

	FDFile = AFile = BFile = PFile = MCFile = MRFile = NULL;
    }
}
