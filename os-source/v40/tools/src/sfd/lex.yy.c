# include "stdio.h"
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX BUFSIZ
# define output(c) putc(c,yyout)
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
# define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng; extern char yytext[];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin = {stdin}, *yyout = {stdout};
extern int yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;
/******	sfd **********************************************************
*
*   NAME
*	sfd -- compile .sfd files
*
*   SYNOPSIS
*	sfd <_lib.sfd files>+		[lib/sfd useage]
*	sfd -h <_lib.sfd files>+	[lib/sfd useage w/o RAM/ROM]
*	sfd -i <_lib.sfd files>+	[local usage: create .i]
*	sfd -f <_lib.sfd files>+	[local usage: create .fd]
*	sfd -v				[determine version]
*
*   FUNCTION
*	sfd consumes files that describe a library's entry points
*	and generates other files in different forms.
*
*   INPUT
*	_lib.sfd file - a file with the file suffix "_lib.sfd"
*	    containing lines describing the library entry points:
*
*	This file has the following format:
*
*	SFD COMMANDS
*
*	SFD Commands exist one to a line, introduced by the character
*	string "==" at the beginning of the line, and extending to the
*	end of the line.
*
*	id: to describe the RCS Id string for the sfd file.  Required
*	    to be the first line in the file.
*	base: to describe the link address for the source for the
*	    module a6 in the RAM/module/*.asm interfaces files, and the
*	    offset off of a6 for the module a6 in the ROM/module/*.asm
*	    interfaces.
*		==base _ModuleBase
*	    The offset is constructed from this: _ModuleBaseOffset
*
*	include: to cause an #include to be generated for the
*	    prototype file to resolve a typedef or #define used
*	    to describe arguments to some of the functions.
*		==include <graphics/gfx.h>
*	    [This defines PLANEPTR for use by graphics functions]
*	    
*	bias: to start the function off
*		==bias 30
*	    This skips over Open/Close/Expunge/Reserved for libraries.
*	    Use 42 for standard devices.
*
*	alias: to alias a function.  The following function definition
*	    keeps the bias of the previous one.
*		LONG DoPkt( struct MsgPort *port, LONG action, LONG arg1,
*			LONG arg2, LONG arg3, LONG arg4 ) (d1,d2,d3,d4,d5,d6)
*		==alias
*		LONG DoPkt1( struct MsgPort *port, LONG action, LONG arg1 )
*			(d1,d2,d3)
*	    
*	varargs: to introduce a varargs alias.  The following function
*	    definition keeps the bias of the previous one, and includes
*	    a prototype with one instance of the varargs parameter,
*	    followed by ...
*		struct Menu *CreateMenusA( struct NewMenu *newmenu,
*			struct TagItem *taglist ) (A0,A1)
*		==varargs
*		struct Menu *CreateMenus( struct NewMenu *newmenu,
*			Tag tag1, ... ) (A0,A1)
*
*	private: to declare the following functions as system use only,
*	    and hide them somewhat.  No #pragma nor RAM/module/*.asm
*	    interfaces are built.
*		==private
*
*	public: to declare the following functions for general use.
*		==public
*
*	reserve: to reserve space in the function table without
*	    generating *any* symbols.  Comments are placed in the
*	    _lib.fd, _pragmas.h, and _protos.h files, and the .fd
*	    file has a ##bias generated for the next function entry.
*	    If a _lib.i file is being created (with the -i option),
*	    FUNCDEF macros are generated for the names moduleReservedNN,
*	    where NN is the -bias of the reserved entry.
*		==reserve 2
*	    reserves two function slots.
*
*	    This is preferred over private/function-declaration/public
*	    as a way to skip over reserved entry points that have no
*	    meaning.
*
*	version: to describe the version of the module that subsequent
*	    functions appear in.  This causes comments to be generated
*	    in _lib.fd, _pragmas.h, and _protos.h files.
*		==version 33
*	    this causes the following comment in the .fd file and similar
*	    ones in the pragma & proto files:
*	    *--- functions in V33 or higher (distributed as Release 1.2) ---
*
*	end: to declare the end of the sfd file.  Required.
*		==end
*
*
*	COMMENTS
*
*	Comments are any line starting with a "*", and last to the
*	end of the line.
*
*		* this is a comment.
*
*	They are converted to comments in the _lib.fd, _pragmas.h,
*	and _protos.h files.
*	o   .fd comments are identical to .sfd comments
*	o   .h comments use the following rules
*	    o   a "/" is prepended to the line
*	    o   if the second character is a blank or a tab, the
*		line is appended with " *\", otherwise, it is
*		appended with "*\".
*	
*
*	FUNCTION DEFINITION
*
*	Anything that is not an sfd command or a comment must be
*	part of a function definition.  A function definition
*	consists of three parts: the return value/function part,
*	the parameter definition, and the register definition:
*	    <return/function>(<parameters>)(<registers>)
*	All three parts must be present.  They may cross lines.
*	A particular function definition is terminated by the
*	second close paranthesis.  A function definition must
*	start on a fresh line.
*
*	    EXAMPLES
*		VOID OpenIntuition() ()
*
*		o   Even an empty parameter list needs an empty register
*		    list.
*		o   The C types void, unsigned, int, short, char, float
*		    and double are not allowed.  Use VOID, [U]LONG,
*		    [U]WORD, [U]BYTE, FLOAT or DOUBLE.  Use APTR, not VOID *.
*		o   Don't put a VOID in an empty parameter list.
*		    Currently, the sfd tool will put one there for you.
*
*		PLANEPTR AllocRaster( UWORD width, UWORD height ) (D0,D1)
*
*		o   The sfd tool must be taught about all-caps primitive
*		    types.  It knows about BPTR, BSTR, STRPTR, IX, and
*		    PLANEPTR so far.
*		o   The parameter list must contain typed variables.
*		o   The register list is generally delimited by commas.
*		    The sfd tool figures out which to cluster for the
*		    resulting _lib.fd file.
*
*		DOUBLE IEEEDPDiv( DOUBLE dividend, DOUBLE divisor )
*			(d0-d1,d2-d3)
*
*		o   The function definition can cross lines.
*		o   DOUBLE registers must be delimited by a dash.
*		o   The register list may use either capital or lower case.
*
*		struct Layer *CreateUpfrontLayer(struct Layer_Info *li,
*			struct BitMap *bm, LONG x0, LONG y0, LONG x1, LONG y1,
*			LONG flags, [struct BitMap *bm2] )
*			(A0,A1,D0,D1,D2,D3,D4,A2)
*
*		o   Optional parameters are indicated by being enclosed
*		    in braces.  Admittedly, the sfd tool currently
*		    outputs the same results as if they were not there,
*		    but we can change that by changing the tool if we
*		    decide to.  Changing the tool instead of changing
*		    the input is an important concept.
*
*		struct Window *OpenWindowTagList( struct NewWindow *newWindow,
*			struct TagItem *tagList ) (A0,A1)
*		==varargs
*		struct Window *OpenWindowTags( struct NewWindow *newWindow,
*			ULONG tag1Type, ... ) (A0,A1)
*		
*		o   The only time ... may appear is for a varargs definition.
*		o   A varargs definition must have one instance of the type.
*
*
*   OUTPUT
*	[lib/sfd usage]
*	    FD/module_lib.fd - an .fd file
*	    HDR/MODULELIBHDR - a bcpl manifest for the function offsets
*	    LVO/module_lib.asm - assembler source for the _LVO offsets
*	    PROTOS/module_protos.h - an ANSI C prototype file
*	    PRAGMAS/module_pragmas.h - a Lattice C #pragma file
*	[...lib/sfd usage w/ RAM/ROM adds...]
*	    RAM/module/*.asm - stack-based C to assembler interfaces
*		for each function, w/ library base at absolute link address
*	    ROM/module/*.asm - stack-based C to assembler interfaces
*		for each function, w/ library base at offset off of a6
*	[local usage]
*	    module_lib.i - macro file for function entries
*
*********************************************************************/

#ifdef   DEBUG
#define  D(a)		printf a
#else
#define  D(a)
#endif

#define  PROTONAMES	1	/* whether _protos.h have names */
#define  PROTOLONGS	1	/* whether num arguments are forced to longs */
#define  SYSBASEPRAGMA	0	/* whether exec #pragmas are special */

#ifdef   LATTICE

#include <string.h>
#define  MKDIR(d,p)	mkdir(d)
#define  RINDEX(s,c)	strrchr(s,c)

#else	/* SUN */

#include <strings.h>
#define  MKDIR(d,p)	mkdir(d,p)
#define  RINDEX(s,c)	rindex(s,c)

#endif

#define	TOKEN_NEWLINE		1
#define	TOKEN_ID		2
#define	TOKEN_BASE		3
#define	TOKEN_INCLUDE		4
#define	TOKEN_BIAS		5
#define	TOKEN_ALIAS		6
#define	TOKEN_VARARGS		7
#define	TOKEN_PRIVATE		8
#define	TOKEN_PUBLIC		9
#define	TOKEN_RESERVE		10
#define	TOKEN_VERSION		11
#define	TOKEN_END		12
#define	TOKEN_COMMENT		13
#define	TOKEN_RCSIDHEAD		14
#define	TOKEN_RCSIDTAIL		15
#define	TOKEN_FILEPATH		16
#define	TOKEN_NUMBER		17
#define	TOKEN_REGISTER		18
#define	TOKEN_BADNAME		19
#define	TOKEN_NAME		20
#define	TOKEN_STARS		21
#define	TOKEN_ELLIPSIS		22
#define	TOKEN_OPENPARAN		23
#define	TOKEN_CLOSEPARAN	24
#define	TOKEN_OPENBRACKET	25
#define	TOKEN_CLOSEBRACKET	26
#define	TOKEN_COMMA		27
#define	TOKEN_DASH		28

/* (see LEX grammar at the end of this file) */

char *Prog;
FILE *FDFile,			/* output library .fd file */
     *AFile,			/* output library .asm file */
     *BFile,			/* output library BCPL header file */
     *CPFile,			/* output library prototype file */
     *LPFile;			/* output library lattice #pragma file */
FILE *MCFile,			/* output RAM/system/Makefile */
     *MRFile;			/* output ROM/system/Makefile */
FILE *CFile,			/* output function C ram interface .asm file */
     *RFile;			/* output function C rom interface .asm file */


void
EndGame(format, arg1, arg2, arg3, arg4)
char *format, *arg1, *arg2, *arg3, *arg4;
{
    if (format) {
	fprintf(stderr, "%s: ERROR -- ", Prog);
	fprintf(stderr, format, arg1, arg2, arg3, arg4);
    }
    if (yyin) fclose(yyin);
    if (FDFile) fclose(FDFile);
    if (AFile) fclose(AFile);
    if (BFile) fclose(BFile);
    if (CPFile) fclose(CPFile);
    if (LPFile) fclose(LPFile);
    if (MCFile) fclose(MCFile);
    if (MRFile) fclose(MRFile);
    if (CFile) fclose(CFile);
    if (RFile) fclose(RFile);
    if (format) exit(100);
    exit(0);
}


int
yywrap() {
    return (1);
}

#define	STATE_START	0	/* first state */
#define	STATE_ID	1	/* waiting for Id string */
#define	STATE_RCSID	2	/* waiting for Id string tail */
#define	STATE_INACTIVE	3	/* nothing active */
#define	STATE_WAITNL	4	/* waiting for \r */
#define	STATE_WAITEOF	5	/* waiting for EOF */
#define	STATE_BASE	6	/* waiting for BaseName */
#define	STATE_INCLUDE	7	/* waiting for include file path */
#define	STATE_BIAS	8	/* waiting for Bias */
#define	STATE_RESERVE	9	/* waiting for Reserve */
#define	STATE_VERSION	10	/* waiting for Version */
#define	STATE_RETVALUE	11	/* waiting for first open paran */
#define	STATE_PARMS	12	/* waiting for first close paran */
#define	STATE_MIDDLE	13	/* waiting for second open paran */
#define	STATE_REGS	14	/* waiting for second close paran */

#define	NAMEMAX	31
char  SFDName[32];		/* "module_lib.sfd" */
char  LibName[32];		/* "module_lib" */
char  SysName[32];		/* "module" */
char  CapsName[32];		/* "MODULE" */
char  IdName[256];		/* rcs Id string */
char  BaseName[32];		/* "_ModuleBase" */
char  FunctionName[NAMEMAX];	/* "Foo" -- a module function */
char  PrevToken[NAMEMAX];	/* the previous name token while parsing */
char  FileName[64];		/* store for building file paths */
char  ProtectedRegs[48];	/* store for saved move[m] registers */
char  ArgRegs[48];		/* store for current move[m] argument */
char  ProtoGroup[80];		/* store for tokens for prototype group */
short LVOCount;			/* count of LVO entries so far */
short ObjCount;			/* count of number of obj files */
int   Bias;			/* current bias -6n offset in .fd file */
short ProtoLen;			/* length of current line in prototype file */
char  ParmRegs[16];		/* order of appearance of parms in d0-a7 */
char  Public;			/* true if ==public, false if ==private */
int   PrivateCount;		/* sequence number of private fcns for FD file */
char  FForm;			/* true if sfd -f */
char  HForm;			/* true if sfd -h */
char  IForm;			/* true if sfd -i */
char  NumRegs;			/* number of entries in ParmRegs */
char  NumArgs;			/* number of arguments in prototype */
char  Alias;			/* true if ==alias */
char  VarArgs;			/* true if ==varargs */
char  OptArg;			/* nest count of [] in STATE_PARMS */
char  OptArgExists;		/* true after first [ */
char  SuppressCPSpace;		/* e.g. previous writeCP was star(s) */
char  FuncParm;			/* actually nest count of () in STATE_PARMS */
char  FuncArg;			/* set going into second ()() in STATE_PARMS */
char  PrevParmToken;		/* nonzero if parameter was typed */

#define  BASE_RELEASE	30
#define  KNOWN_RELEASES	40
char *Release[KNOWN_RELEASES-BASE_RELEASE+1] = {
    "Release 1.0",		/* V30 */
    "Release 1.1",		/* V31 */
    "Preliminary Release 1.2",	/* V32 */
    "Release 1.2",		/* V33 */
    "Release 1.3",		/* V34 */
    "Release 1.3 A2024",	/* V35 */
    "Release 2.0",		/* V36 */
    "Release 2.04",		/* V37 */
    "Release 2.1",		/* V38 */
    "Release 3",		/* V39 */
    "Release 3.1",		/* V40 */
};

#define  NUM_SCALERS	4
#define  LEN_SCALERS	2
char *Scaler[NUM_SCALERS] = {
    "BY", "WO", "LO", "BO"	/* BYTE, WORD, LONG, BOOL */
};

void
writeCP(string)
char *string;
{
#if	PROTOLONGS
    char *pg, localProto[90];
#endif
    char *lp;
    short i;

    if (FForm || (*string == '\0'))
	return;

    /*
     * tokens that are not preceeded by '*' and are not the first name
     * within a function parameter, and are not ',' or '(',
     * are preceeded by a space
     */
    if ((!SuppressCPSpace) && (*string != ',') && (*string != '('))
	strcat(ProtoGroup, " ");

    if (strcmp(string, "VOID") == 0)
	strcat(ProtoGroup, "void");
    else
	strcat(ProtoGroup, string);

    /* ProtoGroups ending in ',', '(', or ')' don't cross line boundaries */
    if ((FuncParm == 0) &&
	    ((*string == ',') || (*string == '(') || (*string == ')'))) {
#if	PROTOLONGS
	if ((*string == ',') || (*string == ')')) {
	    pg = ProtoGroup;
	    lp = localProto;
	    if (*pg == ' ') {
		*lp++ = ' ';
		pg++;
	    }
	    if ((*pg == 'U') && (pg[6] != '*')) {
		/* must be UBYTE, UWORD, or ULONG */
		strcpy(lp, "unsigned long");
		strcpy(lp+13, pg+5);
		lp = localProto;
	    }
	    else {
		if (pg[5] != '*') {
		    for (i = 0; i < NUM_SCALERS; i++) {
			if (strncmp(pg, Scaler[i], LEN_SCALERS) == 0) {
			    /* must be BYTE, WORD, LONG, or BOOL */
			    strcpy(lp, "long");
			    strcpy(lp+4, pg+4);
			    lp = localProto;
			    goto gotLP;
			}
		    }
		}
		lp = ProtoGroup;
	    }
	}
	else {
	    lp = ProtoGroup;
	}
gotLP:
#else
	lp = ProtoGroup;
#endif
	i = strlen(lp);
	if ((ProtoLen + i) >= 80) {
	    /* wrap lines */
	    if (*lp == ' ') {
		fprintf(CPFile, "\n\t%s", lp + 1);
		ProtoLen = i + 8;
	    }
	    else {
		fprintf(CPFile, "\n\t%s", lp);
		ProtoLen = i + 7;
	    }
	}
	else {
	    fprintf(CPFile, lp);
	    ProtoLen += i;
	}
	ProtoGroup[0] = '\0';
    }

    if (FuncParm)
	SuppressCPSpace = ((*string == '*') ||
		(*string == '(') || (*string == ')'));
    else
	SuppressCPSpace = (*string == '*');
}


void
genInterface()
{
    short varReg, stackOffset, stackSave, DvarReg = 0;
    short argNum, reg, testReg;
    char *s;

    /* do all the work for a function definition */
    if (!HForm) {
	/* open C interface files */
	strcpy(FileName, "ROM/");
	strcat(FileName, SysName);
	strcat(FileName, "/");
	strcat(FileName, FunctionName);
	strcat(FileName, ".asm");
	RFile = fopen(FileName, "w");
	if (RFile == 0)
	    EndGame("cannot create file \"%s\"\n", FileName);
	/* add this to the Makefile */
	/* We break every 100 objects to prevent getting a
	 * "$? list too long" error, which would occur
	 * in our biggest modules.
	 */
	if ( ( ObjCount % 100 ) == 0 ) {
	    fprintf(MRFile, "\n\nOBJS%ld=", ObjCount/100 );
	    fprintf(MCFile, "\n\nOBJS%ld=", ObjCount/100 );
	}
	fprintf(MRFile, "\\\n	%s.obj", FunctionName);

	if (Public) {
	    FileName[1] = 'A';		/* RAM/ */
	    CFile = fopen(FileName, "w");
	    if (CFile == 0)
		EndGame("cannot create file \"%s\"\n", FileName);
	    /* add this to the Makefile */
	    fprintf(MCFile, "\\\n	%s.obj", FunctionName);
	}
	else
	    CFile = 0;

	/* write C interface files */
	fprintf(RFile, "*** DO NOT EDIT: FILE BUILT AUTOMATICALLY\n");
	fprintf(RFile, "*** %s rom interface\n", FunctionName);
	if (BaseName[0] != '\0')
	    fprintf(RFile, "	XREF	%sOffset\n", BaseName);
	fprintf(RFile, "	SECTION	%s\n", SysName);
	fprintf(RFile, "	XDEF	_%s\n", FunctionName);
	fprintf(RFile, "_%s:\n", FunctionName);

	if (CFile) {
	    fprintf(CFile, "*** DO NOT EDIT: FILE BUILT AUTOMATICALLY\n");
	    fprintf(CFile, "*** %s ram interface\n", FunctionName);
	    if (BaseName[0] != '\0')
		fprintf(CFile, "	XREF	%s\n", BaseName);
	    fprintf(CFile, "	SECTION	%s\n", SysName);
	    fprintf(CFile, "	XDEF	_%s\n", FunctionName);
	    fprintf(CFile, "_%s:\n", FunctionName);
	}
	ObjCount++;
    }

    /* save protected registers and a6 */
    stackOffset = 4;
    s = ProtectedRegs;
    for (reg = 2; reg <= 7; reg++) {
	if (ParmRegs[reg]) {
	    *s++ = '/';
	    *s++ = 'd';
	    *s++ = '0' + reg;
	    stackOffset += 4;
	}
    }
    for (reg = 2; reg <= 5; reg++) {
	if (ParmRegs[8+reg]) {
	    *s++ = '/';
	    *s++ = 'a';
	    *s++ = '0' + reg;
	    stackOffset += 4;
	}
    }
    *s = '\0';
    if (!HForm) {
	if (ProtectedRegs[0]) {
	    fprintf(RFile, "\t	movem.l	%s/a6,-(a7)\n", ProtectedRegs+1);
	    if (CFile)
		fprintf(CFile, "\t\tmovem.l\t%s/a6,-(a7)\n", ProtectedRegs+1);
	}
	else {
	    fprintf(RFile, "\t	move.l	a6,-(a7)\n");
	    if (CFile)
		fprintf(CFile, "\t	move.l	a6,-(a7)\n");
	}
	if (ParmRegs[14 /* a6 */] == 0) {
	    /* check for valid BaseName */
	    if (BaseName[0] == '\0')
		EndGame("%s: ==base required\n", SFDName);
	    fprintf(RFile, "\t\tmove.l\t%sOffset(a6),a6\n", BaseName);
	    if (CFile)
		fprintf(CFile, "\t\tmove.l\t%s,a6\n", BaseName);
	}


	if (VarArgs) {
	    if (NumRegs == 0)
		EndGame("%s %s: missing vararg register\n", SFDName,
			FunctionName);
	    /* remove varReg from ParmRegs */
	    for (reg = 0; ParmRegs[reg] != NumRegs; reg++);
	    varReg = reg;
/*	    if (varReg<8)
**		EndGame("%s %s: d%ld illegal vararg\n", SFDName, FunctionName,
**			varReg);
*/
	    ParmRegs[reg] = 0;
	    NumRegs--;

	    /* lea now and use to grab any other parameters */
	    if(varReg >= 8)
	    {
		varReg -= 8;
		/* points at first arg - will be incremented by movem */
		fprintf(RFile, "\t\tlea\t%ld(a7),a%d\n", stackOffset + 4, varReg);
	    	if (CFile)
			fprintf(CFile, "\t\tlea\t%ld(a7),a%d\n", stackOffset + 4,
				varReg);
	    }
	    else
	    {
		DvarReg = 1;	/* remember! */
		/* pea calculates address first!!! */
		/* unlike regular varargs, point it at the right entry */
		/* regular ones point it at the first arg, then movem (an)+,...
		   where an is the varargs ptr, leaving it pointing past the
		   last one. */

		/* NumRegs is the number of parameters on the stack before */
		/* the first vararg param.				   */

	        fprintf(RFile, "\t\tpea\t%ld(a7)\n", stackOffset + 4 +
						     4*NumRegs);
		fprintf(RFile, "\t\tmove.l\t(a7)+,d%ld\n", varReg);
	        if (CFile)
		{
		    /* pea decrements stack first! */
		    fprintf(CFile, "\t\tpea\t%ld(a7)\n", stackOffset + 4 +
							 4*NumRegs);
		    fprintf(CFile, "\t\tmove.l\t(a7)+,d%ld\n", varReg);
		}
	    }
	}
    }

    argNum = 1;
    while (argNum <= NumRegs) {
	stackSave = stackOffset;
	for (reg = 0; ParmRegs[reg] != argNum; reg++);
	/* argNum is parameter order, reg is register number */

	/* grok parameter register clustering */
	if ((!Alias) && (!VarArgs) && (argNum != 1)) {
	    /* there was a cluster preceeding this one, update FD File */
	    if (Public)fprintf(FDFile, ",");
	}

	/* build register cluster */
	s = ArgRegs;
	do {
	    *s++ = '/';
	    *s++ = (reg<8)?'d':'a';
	    *s++ = '0'+(reg&7);
	    stackOffset += 4;

	    /* look first for double */
	    testReg = reg;
	    do {
		reg++;
	    }
		while ((reg < 16) && (ParmRegs[reg] != argNum));

	    if (reg == 16) {
		/* look next at next parameter */
		reg = testReg;
		argNum++;
		do {
		    reg++;
		}
		    while ((reg < 16) && (ParmRegs[reg] != argNum));
	    }
	}
	    while (reg != 16);
	*s = '\0';

	if ((!Alias) && (!VarArgs)) {
	    /* update FD File */
	    if (Public)fprintf(FDFile, ArgRegs+1);
	}

	if (!HForm) {
	    /* don't use varReg if it was a Data reg */
	    if (VarArgs && !DvarReg) {
		/* use autoincrementing varReg */
		if (strlen(ArgRegs) == 3) {
		    /* move single register */
		    fprintf(RFile, "\t\tmove.l\t(a%ld)+,%s\n", varReg,
			    ArgRegs+1);
		    if (CFile)
			fprintf(CFile, "\t\tmove.l\t(a%ld)+,%s\n", varReg,
				ArgRegs+1);
		}
		else {
		    /* move multiple registers */
		    fprintf(RFile, "\t\tmovem.l\t(a%ld)+,%s\n", varReg,
			    ArgRegs+1);
		    if (CFile)
			fprintf(CFile, "\t\tmovem.l\t(a%ld)+,%s\n", varReg,
				ArgRegs+1);
		}
	    }
	    else {
		/* use explicit stack offset */
		if (strlen(ArgRegs) == 3) {
		    /* move single register */
		    fprintf(RFile, "\t\tmove.l\t%ld(a7),%s\n", stackSave + 4,
			    ArgRegs+1);
		    if (CFile)
			fprintf(CFile, "\t\tmove.l\t%ld(a7),%s\n",
				stackSave + 4, ArgRegs+1);
		}
		else {
		    /* move multiple registers */
		    fprintf(RFile, "\t\tmovem.l\t%ld(a7),%s\n", stackSave + 4,
			    ArgRegs+1);
		    if (CFile)
			fprintf(CFile, "\t\tmovem.l\t%ld(a7),%s\n",
				stackSave + 4, ArgRegs+1);
		}
	    }
	}
    }

    if ((!Alias) && (!VarArgs)) {
	/* terminate FD line */
	fprintf(FDFile, ")\n");
    }

    if (!HForm) {
	fprintf(RFile, "\t	jsr	%ld(a6)\n", Bias);
	if (CFile)
	    fprintf(CFile, "\t	jsr	%ld(a6)\n", Bias);
	if (ProtectedRegs[0]) {
	    fprintf(RFile, "		movem.l	(a7)+,%s/a6\n",
		    ProtectedRegs+1);
	    if (CFile)
		fprintf(CFile, "\t	movem.l	(a7)+,%s/a6\n",
			ProtectedRegs+1);
	}
	else {
	    fprintf(RFile, "		move.l	(a7)+,a6\n");
	    if (CFile)
		fprintf(CFile, "\t	move.l	(a7)+,a6\n");
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
    }
}


main(argc, argv)
int argc;
char *argv[];
{
    char *s, *t;
    short state, token;
    char pragmaWord[32];
    short pragmaCount, pragmaOK;
    int i, j;

    Prog = *argv++;
    if (s = RINDEX(Prog, '/'))
	Prog = s + 1;

    FForm = HForm = IForm = 0;
    if ((argc >= 2) && ((*argv)[0] == '-')) {
	switch ((*argv)[1]) {
	  case 'f':
	    FForm = 1;
	  case 'h':
	    HForm = 1;
	    argv++;
	    argc--;
	    break;
	  case 'i':
	    IForm = 1;
	    argv++;
	    argc--;
	    break;
	  default:
	    fprintf(stderr, "sfd $Revision: 40.5 $\n");
	    fprintf(stderr,
		    "\tsfd <_lib.sfd files>+\t\t[lib/sfd useage]\n");
	    fprintf(stderr,
		"\tsfd -h <_lib.sfd files>+\t[lib/sfd useage w/o RAM/ROM]\n");
	    fprintf(stderr,
		    "\tsfd -i <_lib.sfd files>+\t[local usage: create .i]\n");
	    fprintf(stderr,
		    "\tsfd -f <_lib.sfd files>+\t[local usage: create .fd]\n");
	    exit(1);
	}
    }

    while (--argc > 0) {
	yyin = FDFile = AFile = BFile = CPFile = LPFile =
		CFile = RFile = MCFile = MRFile = NULL;

	PrivateCount = 0;

	/* get the library name */
	strcpy(SFDName, *argv++);
	strcpy(LibName, SFDName);
	s = RINDEX(LibName, '.');
	if ((s == NULL) || (strcmp(s, ".sfd") != 0))
	    EndGame(".sfd file name \"%s\" ill formed\n", SFDName);
	*s = '\0';
	
	/* get the system name */
	strcpy(SysName, LibName);
	s = RINDEX(SysName, '_');
	if ((s == NULL) || (strcmp(s, "_lib") != 0))
	    EndGame(".sfd file name \"%s\" ill formed\n", SFDName);
	*s = '\0';

	/* open the .sfd file */
	yyin = fopen(SFDName, "r");
	if (yyin == 0)
	    EndGame("cannot open file \"%s\"\n", SFDName);

	if (FForm || IForm) {
	    /* open the function description file */
	    strcpy(FileName, LibName);
	    if (FForm)
		strcat(FileName, ".fd");
	    else
		strcat(FileName, ".i");
	    FDFile = fopen(FileName, "w");
	    if (FDFile == 0)
		EndGame("cannot create file \"%s\"\n", FileName);
	}
	else {
	    /* ensure the directories are there */

	    /* open the .fd file */
	    MKDIR("FD", 0777);
	    strcpy(FileName, "FD/");
	    strcat(FileName, LibName);
	    strcat(FileName, ".fd");
	    FDFile = fopen(FileName, "w");
	    if (FDFile == 0)
		EndGame("cannot create file \"%s\"\n", FileName);

	    /* open the .asm file */
	    MKDIR("LVO", 0777);
	    strcpy(FileName, "LVO/");
	    strcat(FileName, LibName);
	    strcat(FileName, ".asm");
	    AFile = fopen(FileName, "w");
	    if (AFile == 0)
		EndGame("cannot create file \"%s\"\n", FileName);

	    /* open the .b file */
	    MKDIR("HDR", 0777);
	    strcpy(FileName, "HDR/");
	    strcat(FileName, SysName);
	    s = FileName+4;
	    while (*s)
		*s++ &= 0x5f;
	    strcat(FileName, "LIBHDR");
	    BFile = fopen(FileName, "w");
	    if (BFile == 0)
		EndGame("cannot create file \"%s\"\n", FileName);

	    /* open the proto file */
	    MKDIR("PROTOS", 0777);
	    strcpy(FileName, "PROTOS/");
	    strcat(FileName, SysName);
	    strcat(FileName, "_protos.h");
	    CPFile = fopen(FileName, "w");
	    if (CPFile == 0)
		EndGame("cannot create file \"%s\"\n", FileName);

	    /* open the pragma file */
	    MKDIR("PRAGMAS", 0777);
	    strcpy(FileName, "PRAGMAS/");
	    strcat(FileName, SysName);
	    strcat(FileName, "_pragmas.h");
	    LPFile = fopen(FileName, "w");
	    if (LPFile == 0)
		EndGame("cannot create file \"%s\"\n", FileName);

	    if (!HForm) {
		/* open the RAM Makefile file */
		MKDIR("RAM", 0777);
		MKDIR("ROM", 0777);
		strcpy(FileName, "ROM/");
		strcat(FileName, SysName);
		MKDIR(FileName, 0777);
		FileName[1] = 'A';		/* RAM/ */
		MKDIR(FileName, 0777);

		strcat(FileName, "/Makefile");
		MCFile = fopen(FileName, "w");
		if (MCFile == 0)
		    EndGame("cannot create file \"%s\"\n", FileName);

		/* open the ROM Makefile file */
		FileName[1] = 'O';		/* ROM/ */
		MRFile = fopen(FileName, "w");
		if (MRFile == 0)
		    EndGame("cannot create file \"%s\"\n", FileName);
	    }

	    /* initialize system files */
	    fprintf(AFile, "*** DO NOT EDIT: FILE BUILT AUTOMATICALLY\n");
	    fprintf(AFile, "*** %s.asm function offsets\n", LibName);
	    fprintf(AFile, "	IDNT	%s_LVO\n", SysName);

	    fprintf(BFile, "MANIFEST\n$(\n");

	    s = SysName;
	    t = CapsName;
	    while (*s) {
		if (*s == '.')
		    *t++ = '_';
		else
		    if (*s > 0x60)
			*t++ = *s & 0x5f;
		    else
			*t++ = *s;
		s++;
	    }
	    *t = '\0';
	    fprintf(CPFile, "#ifndef  CLIB_%s_PROTOS_H\n", CapsName);
	    fprintf(CPFile, "#define  CLIB_%s_PROTOS_H\n", CapsName);

	    if (!HForm) {
		fprintf(MCFile, "########################################\n");
		fprintf(MCFile, "MAKEMETA=	../../../../tools/makemeta\n");

		fprintf(MRFile, "########################################\n");
		fprintf(MRFile, "MAKEMETA=	../../../../tools/makemeta\n");
	    }
	}

	/* parse the .sfd file */
	LVOCount = 0;
	ObjCount = 0;
	BaseName[0] = BaseName[1] = '\0';
	Bias = -6;
	Alias = 0;
	VarArgs = 0;
	Public = 0;
	strcpy(FunctionName, "[before 1st function]");

	state = STATE_START;
	while (token = yylex()) {
	    switch (state) {
	      case STATE_START:		/* waiting for ==id */
		if (token != TOKEN_ID)
		    EndGame("%s: ==id required as first line\n", SFDName);
		state = STATE_ID;
		break;
	      case STATE_ID:		/* waiting for rcs Id string */
		if (token != TOKEN_RCSIDHEAD)
		    EndGame("%s: illegal rcs Id \"%s\"... after ==id\n",
			    SFDName, yytext);
		state = STATE_RCSID;
		break;
	      case STATE_RCSID:		/* waiting for rcs Id string tail */
		if (token != TOKEN_RCSIDTAIL)
		    EndGame("%s: illegal rcs Id ...\"%s\" after ==id\n",
			    SFDName, yytext);
		strcpy(IdName, yytext);
		if ((!FForm) && (!IForm)) {
		    fprintf(CPFile,
			    "\n/*\n**\t$Id: %s_protos.h,v %s\n", SysName, IdName);
		    fprintf(CPFile, "**\n**\tC prototypes. For use with 32 bit integers only.\n**\n");
		    fprintf(CPFile,
			    "**\t(C) Copyright 1990 Commodore-Amiga, Inc.\n");
		    fprintf(CPFile, "**\t    All Rights Reserved\n*/\n\n");

		    fprintf(LPFile,
			    "/*\n**\t$Id: %s_pragmas.h,v %s\n", SysName, IdName);
		    fprintf(LPFile, "**\n**\tSAS/C format pragma files.\n");
		    fprintf(LPFile, "**\n**\tContains private definitions. COMMODORE INTERNAL USE ONLY!\n*/\n\n");
		}
		state = STATE_WAITNL;
		break;
	      case STATE_INACTIVE:	/* nothing active */
		switch (token) {
		  case TOKEN_NEWLINE:
		    break;
		  case TOKEN_BASE:
		    state = STATE_BASE;
		    break;
		  case TOKEN_INCLUDE:
		    state = STATE_INCLUDE;
		    break;
		  case TOKEN_BIAS:
		    state = STATE_BIAS;
		    break;
		  case TOKEN_ALIAS:
		    Bias += 6;
		    Alias = 1;
		    state = STATE_WAITNL;
		    break;
		  case TOKEN_VARARGS:
		    Bias += 6;
		    VarArgs = 1;
		    state = STATE_WAITNL;
		    break;
		  case TOKEN_PRIVATE:
		    Public = 0;
		    if (!IForm)
			fprintf(FDFile, "##private\n");
		    state = STATE_WAITNL;
		    break;
		  case TOKEN_PUBLIC:
		    Public = 1;
		    if (!IForm)
			fprintf(FDFile, "##public\n");
		    state = STATE_WAITNL;
		    break;
		  case TOKEN_RESERVE:
		    state = STATE_RESERVE;
		    break;
		  case TOKEN_VERSION:
		    state = STATE_VERSION;
		    break;
		  case TOKEN_END:
		    if (!IForm)
			fprintf(FDFile, "##end\n");
		    state = STATE_WAITEOF;
		    break;
		  case TOKEN_COMMENT:
		    if (!IForm) {
			fprintf(FDFile, "*%s\n", yytext+1);
			if (!FForm) {
			    if (yytext[1] == '\0') {
				/* Output blank lines as truly blank */
				if (Public)fprintf(CPFile, "\n");
				fprintf(LPFile, "\n");
			    }
			    else
			    if ((yytext[1] == ' ') || (yytext[1] == '\t')) {
				/* whitespace at front -> whitespace at back */
				if (Public)fprintf(CPFile, "/*%s */\n", yytext+1);
				fprintf(LPFile, "/*%s */\n", yytext+1);
			    }
			    else {
				if (Public)fprintf(CPFile, "/*%s*/\n", yytext+1);
				fprintf(LPFile, "/*%s*/\n", yytext+1);
			    }
			}
		    }
		    break;
		  case TOKEN_REGISTER:
		    fprintf(stderr, "%s: WARNING -- %s\n", Prog, SFDName);
		    fprintf(stderr,
		    "    function return type \"%s\" looks like a register\n",
			    yytext);
		  case TOKEN_NAME:
		    PrevToken[0] = '\0';
		    if (!IForm) {
			ProtoLen = 0;
			ProtoGroup[0] = '\0';
			SuppressCPSpace = 1;	/* suppress initial space */
			if(Public)writeCP(yytext);
		    }
		    for (i = 0; i < 16; i++)
			ParmRegs[i] = 0;
		    NumRegs = 0;
		    NumArgs = 0;
		    state = STATE_RETVALUE;
		    break;
		  default:
		    EndGame("%s near %s: illegal occurance of \"%s\"\n",
			    SFDName, FunctionName, yytext);
		}
		break;
	      case STATE_WAITNL:	/* waiting for \r */
		if (token != TOKEN_NEWLINE)
		    EndGame("%s: newline expected, got \"%s\"\n",
			    SFDName, yytext);
		state = STATE_INACTIVE;
		break;
	      case STATE_WAITEOF:	/* waiting for EOF */
		if (token != TOKEN_NEWLINE)
		    EndGame("%s: illegal occurance of \"%s\" after ==end\n",
			    SFDName, yytext);
		break;
	      case STATE_BASE:		/* waiting for BaseName */
		switch (token) {
		  case TOKEN_REGISTER:
		    fprintf(stderr, "%s: WARNING -- %s\n", Prog, SFDName);
		    fprintf(stderr,
			    "    base name \"%s\" looks like a register\n",
			    yytext);
		  case TOKEN_NAME:
		    strcpy(BaseName, yytext);
		    if (!IForm)
			fprintf(FDFile, "##base %s\n", BaseName);
		    state = STATE_WAITNL;
		    break;
		  default:
		    EndGame("%s: illegal name \"%s\" after ==base\n",
			    SFDName, yytext);
		    break;
		}
		break;
	      case STATE_INCLUDE:	/* waiting for include file path */
		switch (token) {
		  case TOKEN_FILEPATH:
		    if ((!FForm) && (!IForm)) {
			s = yytext + 1;
			t = FileName;
			while (*s != '>') {
			    if ((*s == '/') || (*s == '.'))
				*t++ = '_';
			    else
				if (*s > 0x60)
				    *t++ = *s & 0x5f;
				else
				    *t++ = *s;
			    s++;
			}
			*t = '\0';
			fprintf(CPFile, "#ifndef  %s\n", FileName);
			fprintf(CPFile, "#include %s\n", yytext);
			fprintf(CPFile, "#endif\n");
		    }
		    state = STATE_WAITNL;
		    break;
		  default:
		    EndGame("%s: illegal path \"%s\" after ==include\n",
			    SFDName, yytext);
		    break;
		}
		break;
	      case STATE_BIAS:		/* waiting for Bias */
		switch (token) {
		  case TOKEN_NUMBER:
		    i = Bias;
		    Bias = -atoi(yytext);
		    if (Bias > i)
			EndGame("%s: bias overlap %ld to %ld\n", SFDName,
				i, Bias);
		    if (IForm) {
			if (i != -6) {
			    for (; i > Bias; i -= 6)
				fprintf(FDFile, "\tFUNCDEF\t%sReserved%ld\n",
					SysName, -i);
			}
		    }
		    else
			fprintf(FDFile, "##bias %ld\n", -Bias);
		    state = STATE_WAITNL;
		    break;
		  default:
		    EndGame("%s: illegal bias \"%s\" after ==bias\n",
			    SFDName, yytext);
		}
		break;
	      case STATE_RESERVE:	/* waiting for Reserve */
		switch (token) {
		  case TOKEN_NUMBER:
		    i = atoi(yytext);
		    if (IForm) {
			for (j = 0; j < i; j++) {
			    fprintf(FDFile, "\tFUNCDEF\t%sReserved%ld\n",
				    SysName, -Bias);
			    Bias -= 6;
			}
		    }
		    else {
			Bias -= i * 6;
			fprintf(FDFile,
			"*--- (%ld function slot%s reserved here) ---\n",
				i, (i == 1)?"":"s");
			fprintf(FDFile, "##bias %ld\n", -Bias);
			if (!FForm) {
			    fprintf(LPFile,
			    "/*--- (%ld function slot%s reserved here) ---*/\n",
				    i, (i == 1)?"":"s");
			}
		    }
		    state = STATE_WAITNL;
		    break;
		  default:
		    EndGame("%s: illegal number \"%s\" after ==reserve\n",
			    SFDName, yytext);
		}
		break;
	      case STATE_VERSION:	/* waiting for Version */
		switch (token) {
		  case TOKEN_NUMBER:
		    i = atoi(yytext);
		    if (i < BASE_RELEASE)
			EndGame("%s: version %ld unheard of\n", SFDName, i);
		    if (!IForm) {
			char *releasename = "Beta release for developers only";

			if ( i <= KNOWN_RELEASES )
			{
			    releasename = Release[i-BASE_RELEASE];
			}
			fprintf( FDFile,
		"*--- functions in V%ld or higher (%s) ---\n",
				i, releasename );
			if (!FForm) {
			    fprintf( CPFile,
		"/*--- functions in V%ld or higher (%s) ---*/\n",
				    i, releasename );
			    fprintf( LPFile,
		"/*--- functions in V%ld or higher (%s) ---*/\n",
				    i, releasename );
			}
		    }
		    state = STATE_WAITNL;
		    break;
		  default:
		    EndGame("%s: illegal version \"%s\" after ==version\n",
			    SFDName, yytext);
		}
		break;
	      case STATE_RETVALUE:	/* waiting for first open paran */
		switch (token) {
		  case TOKEN_REGISTER:
		    fprintf(stderr, "%s: WARNING -- %s\n", Prog, SFDName);
		    fprintf(stderr,
		"    return value/function name \"%s\" looks like a register\n",
			    yytext);
		  case TOKEN_NAME:
		    if (strlen(yytext) >= NAMEMAX)
			EndGame("%s after %s: name \"%s\" too long\n",
				SFDName, FunctionName, yytext);
		    strcpy(PrevToken, yytext);
		  case TOKEN_STARS:
		    if (!IForm) {
			if(Public)writeCP(yytext);
		    }
		    break;
		  case TOKEN_OPENPARAN:
		    /* PrevToken is the function name */
		    if (!PrevToken[0])
			EndGame("%s after %s: missing function name\n",
				SFDName, FunctionName);
		    if (IForm) {
			fprintf(FDFile, "\tFUNCDEF\t%s\n", PrevToken);
		    }
		    else {
			strcpy(FunctionName, PrevToken);
			if ((!Alias) && (!VarArgs)) {
			    if (Public) fprintf(FDFile, "%s(", FunctionName);
			    else fprintf(FDFile, "%sPrivate%ld(", SysName, ++PrivateCount);
			}
			if(Public)writeCP(yytext);
		    }
		    PrevToken[0] = '\0';
		    PrevParmToken = 0;
		    FuncParm = 0;
		    FuncArg = 0;
		    OptArg = 0;
		    OptArgExists = 0;
		    state = STATE_PARMS;
		    break;
		  case TOKEN_NEWLINE:
		    break;
		  default:
		    EndGame("%s after %s: format error: \"%s\"\n",
			    SFDName, FunctionName, yytext);
		}
		break;
	      case STATE_PARMS:		/* waiting for first close paran */
		switch (token) {
		  case TOKEN_NEWLINE:
		    break;
		  case TOKEN_REGISTER:
		    fprintf(stderr, "%s: WARNING -- %s %s\n", Prog, SFDName,
			    FunctionName);
		    fprintf(stderr,
			    "    parameter name \"%s\" looks like a register\n",
			    yytext);
		  case TOKEN_NAME:
		    PrevParmToken = PrevToken[0];
		  case TOKEN_STARS:
		    if ((strcmp(yytext, "*") == 0) &&
			    (strcmp(PrevToken, "VOID") == 0))
			EndGame("%s %s: need to use APTR, not VOID *\n",
				SFDName, FunctionName);
		  case TOKEN_ELLIPSIS:
		    if (strcmp(PrevToken, "...") == 0)
			EndGame("%s %s: mispositioned vararg \"...\"\n",
				SFDName, FunctionName);
		    if (!IForm) {
			if(Public)writeCP(PrevToken);
			if (strlen(yytext) >= NAMEMAX)
			    EndGame("%s %s: name \"%s\" too long\n", SFDName,
				    FunctionName, yytext);
			strcpy(PrevToken, yytext);
		    }
		    break;
		  case TOKEN_OPENPARAN:
		    if (strcmp(PrevToken, "...") == 0)
			EndGame("%s %s: mispositioned vararg \"...\"\n",
				SFDName, FunctionName);
		    if (++FuncParm > 1)
			EndGame("%s %s: \"(\" nested too deep\n", SFDName,
				FunctionName);
		    if (!IForm) {
			if(Public)writeCP(PrevToken);
			SuppressCPSpace = 1;
			if (!FuncArg) {
			    if(Public)writeCP(" ");
			    SuppressCPSpace = 1;
			}
			if(Public)writeCP(yytext);
			SuppressCPSpace = 1;
			PrevToken[0] = '\0';
		    }
		    break;
		  case TOKEN_CLOSEPARAN:
		    if (FuncParm == 0) {
			/* final parameter closing paranthesis */
			if (OptArg)
			    EndGame("%s %s: unclosed \"]\"\n", SFDName,
				    FunctionName);
			if (!IForm) {
			    if ((!Alias) && (!VarArgs)) {
				if ((PrevToken[0] >= 'A')
					&& (PrevToken[0] <= 'Z')) {
				    fprintf(stderr,
				    "%s: WARNING -- %s %s: parameter \"%s",
					    Prog, SFDName, FunctionName,
					    PrevToken);
				    PrevToken[0] |= 0x20;
				    fprintf(stderr, "\" converted to \"%s\"\n",
					    PrevToken);
				}
			    	if (Public)fprintf(FDFile, "%s)(", PrevToken);
				else fprintf(FDFile, ")(");
			    }
			    if (strcmp(PrevToken, "...") == 0) {
				if (!VarArgs)
				    EndGame("%s %s: missing ==varargs\n",
					    SFDName, FunctionName);
				if(Public)writeCP("...");
			    }
			    else {
				if (PrevToken[0] || FuncArg) {
				    if (!PrevParmToken) {
					EndGame(
					"%s %s: untyped parameter \"%s\"\n",
						SFDName, FunctionName,
						PrevToken);
				    }
				    NumArgs++;
#if	PROTONAMES
				    if(Public)writeCP(PrevToken);
#endif
				}
				else
				    if(Public)writeCP("VOID");
			    }
			    SuppressCPSpace = 0;
			    if(Public)writeCP(");\n");
			}
			state = STATE_MIDDLE;
		    }
		    else {
			if (strcmp(PrevToken, "...") == 0)
			    EndGame("%s %s: mispositioned vararg \"...\"\n",
				    SFDName, FunctionName);
			if (!IForm) {
			    if ((!Alias) && (!VarArgs) && (!FuncArg)) {
				if ((PrevToken[0] >= 'A')
					&& (PrevToken[0] <= 'Z')) {
				    fprintf(stderr,
					"%s: WARNING -- %s %s: parameter \"%s",
					Prog, SFDName, FunctionName, PrevToken);
				    PrevToken[0] |= 0x20;
				    fprintf(stderr, "\" converted to \"%s\"\n",
					    PrevToken);
				}
				if (Public)fprintf(FDFile, "%s", PrevToken);
			    }
#if	PROTONAMES
			    if(Public)writeCP(PrevToken);
#endif
			    SuppressCPSpace = 1;
			    if(Public)writeCP(yytext);
			    PrevToken[0] = '\0';
			}
			FuncParm--;
			if (!FuncArg)
			    FuncArg = 1;
		    }
		    break;
		  case TOKEN_OPENBRACKET:
		    if (OptArg)
			EndGame("%s %s: nested \"[\" not allowed\n",
				SFDName, FunctionName);
		    OptArg++;
		    OptArgExists = 1;
		    break;
		  case TOKEN_CLOSEBRACKET:
		    OptArg--;
		    break;
		  case TOKEN_COMMA:
		    if (OptArgExists && (!OptArg))
			EndGame(
			"%s %s: all parms after first \"[\" must be optional\n",
				SFDName, FunctionName);
		    if (strcmp(PrevToken, "...") == 0)
			EndGame("%s %s: mispositioned vararg \"...\"\n",
				SFDName, FunctionName);
		    if (PrevToken[0] && (!PrevParmToken))
			EndGame("%s %s: untyped parameter \"%s\"\n",
				SFDName, FunctionName, PrevToken);
		    if (!IForm) {
			if ((!Alias) && (!VarArgs) && (!FuncArg)) {
			    if ((PrevToken[0] >= 'A')
				    && (PrevToken[0] <= 'Z')) {
				fprintf(stderr,
					"%s: WARNING -- %s %s: parameter \"%s",
					Prog, SFDName, FunctionName, PrevToken);
				PrevToken[0] |= 0x20;
				fprintf(stderr, "\" converted to \"%s\"\n",
					PrevToken);
			    }
			    if (Public)fprintf(FDFile, "%s", PrevToken);
			}
#if	PROTONAMES
			if(Public)writeCP(PrevToken);
#endif
			if(Public)writeCP(yytext);
			PrevToken[0] = '\0';
			if (!FuncParm) {
			    if ((!Alias) && (!VarArgs))
				if (Public)fprintf(FDFile, ",");
			    FuncArg = 0;
			    NumArgs++;
			}
		    }
		    break;
		  default:
		    EndGame("%s %s: format error before 1st ): \"%s\"\n",
			    SFDName, FunctionName, yytext);
		}
		break;
	      case STATE_MIDDLE:	/* waiting for second open paran */
		switch (token) {
		  case TOKEN_NEWLINE:
		    break;
		  case TOKEN_OPENPARAN:
		    state = STATE_REGS;
		    break;
		  default:
		    EndGame("%s %s: format error before 2nd (: \"%s\"\n",
			    SFDName, FunctionName, yytext);
		}
		break;
	      case STATE_REGS:		/* waiting for second close paran */
		switch (token) {
		  case TOKEN_DASH:
		    NumRegs--;		/* keep same ParmReg entry for next */
		  case TOKEN_NEWLINE:
		  case TOKEN_COMMA:
		    break;
		  case TOKEN_REGISTER:
		    i = yytext[1] - '0';
		    if ((yytext[0] | 0x20) == 'a') i += 8;
		    ParmRegs[i] = ++NumRegs;
		    break;
		  case TOKEN_CLOSEPARAN:
		    if ((!FForm) && (!IForm)) {
			/* Number of arguments must match number of registers,
			 * except that varargs functions can have one less
			 * argument, to handle cases like printf() where the
			 * first vararg is optional.
			 */
			if ( ( NumArgs != NumRegs ) &&
			     ( (!VarArgs) || ( (NumArgs+1) != NumRegs ) ) )
			    EndGame("%s %s: %ld parms vs. %ld registers\n",
				    SFDName, FunctionName, NumArgs, NumRegs);
			if (!VarArgs) {
			    /* write this entry into the library files */
			    if (!Alias) {
				if ((LVOCount & 0x7f) == 0) {
				    /* create a new section every 128 names */
				    /* so as to not overflow BLINK 5.04's */
				    /* 150 symbol limit */
				    if (LVOCount) {
					/* generate something that looks */
					/* like code so that the symbols */
					/* associated with the prev section */
					/* get flushed by the assembler */
					fprintf(AFile, "	OFFSET	0\n");
					fprintf(AFile,
						"%s_FlushLVO%d	ds.w	1\n",
						SysName, (LVOCount>>7)-1);
				    }
				    fprintf(AFile, "	SECTION	%s_LVO%d\n",
					    SysName, LVOCount>>7);
				}
				LVOCount++;
				fprintf(AFile, "	XDEF	_LVO%s\n",
					FunctionName);
				fprintf(AFile, "_LVO%s EQU	%ld\n",
					FunctionName, Bias);

				fprintf(BFile, "    e.%s = %ld\n",
					FunctionName, Bias);
			    }
			}

			/* write pragma file */
			pragmaCount = 0;
			pragmaOK = 1;
			/*     gather pragma */
			for (i = 1; i <= NumRegs; i++) {
			    for (j = 0; ParmRegs[j] != i; j++);
			    pragmaWord[NumRegs - i] = (j <= 9 ? '0' + j :
							    'A' + j-10);
			    if (j >= 14 /* a6 */)
				pragmaOK = 0;
			}
			    pragmaWord[NumRegs] = '0';
			    pragmaWord[NumRegs+1] = (NumRegs <= 9 ?
							'0' + NumRegs :
							'A' + NumRegs-10);
			    pragmaWord[NumRegs+2] = '\0';

			if ( !Public )
			{
			    fprintf(LPFile, "#ifdef %s_PRIVATE_PRAGMAS\n", CapsName);
			}

#if SYSBASEPRAGMA
			if (strcmp(BaseName, "_SysBase") == 0) {
			    if (pragmaOK)
				fprintf(LPFile,
					"#pragma syscall %s %x %s\n",
					FunctionName, -Bias, pragmaWord);
			    else
				fprintf(LPFile,
					"/*pragma syscall %s %x %s*/\n",
					FunctionName, -Bias, pragmaWord);
			}
			else
#endif
			if (BaseName[1])
			{
			    if (pragmaOK)
			    {
				if (!VarArgs)
				{
				    fprintf(LPFile,
					    "#pragma libcall %s %s %x %s\n",
					    BaseName+1, FunctionName, -Bias,
					    pragmaWord);
				}
				else
				{
				    fprintf(LPFile,
					    "#ifdef __SASC_60\n#pragma tagcall %s %s %x %s\n#endif\n",
					    BaseName+1, FunctionName, -Bias,
					    pragmaWord);
				}
			    }
			    else
			    {
				if (!VarArgs)
				{
				    fprintf(LPFile,
					    "/*pragma libcall %s %s %x %s*/\n",
					    BaseName+1, FunctionName, -Bias,
					    pragmaWord);
				}
				else
				{
				    fprintf(LPFile,
					    "/*pragma tagcall %s %s %x %s*/\n",
					    BaseName+1, FunctionName, -Bias,
					    pragmaWord);
				}
			    }
			}
			else
			{
			    if (!VarArgs)
			    {
				fprintf(LPFile,
					"/*pragma libcall  %s %x %s*/\n",
					FunctionName, -Bias, pragmaWord);
			    }
			    else
			    {
				fprintf(LPFile,
					"/*pragma tagcall  %s %x %s*/\n",
					FunctionName, -Bias, pragmaWord);
			    }
			}
			if ( !Public )
			{
			    fprintf(LPFile, "#endif\n");
			}
		    }

		    if (!IForm)
			genInterface();

		    /* decrement Bias */
		    Bias -= 6;
		    Alias = 0;
		    VarArgs = 0;

		    state = STATE_WAITNL;
		    break;

		  default:
		    EndGame("%s %s: format error before 2nd ): \"%s\"\n",
			    SFDName, FunctionName, yytext);
		}
		break;
	    }
	}
	if (state != STATE_WAITEOF)
	    EndGame("%s: missing ==end\n", SFDName);
	fclose(yyin);
	fclose(FDFile);
	if ((!FForm) && (!IForm)) {
	    fprintf(AFile, "	END\n");
	    fclose(AFile);
	    fprintf(BFile, "$)\n");
	    fclose(BFile);
	    fprintf(CPFile, "#endif   /* CLIB_%s_PROTOS_H */\n", CapsName);
	    fclose(CPFile);
	    fclose(LPFile);
	    if (!HForm) {
		fprintf(MCFile, "\n\n");
		fprintf(MCFile, "lib.timestamp:	asm.timestamp\n");
		fprintf(MCFile, "	make ");
		/* We break every 100 objects to prevent getting a
		 * "$? list too long" error, which would occur
		 * in our biggest modules.
		 */
		for ( i = 0; i <= (ObjCount-1)/100; i++ ) {
		    fprintf(MCFile, "objs%ld ", i );
		}
		fprintf(MCFile, "${MFLAGS} ${MARGS}\n");
		fprintf(MCFile, "	cat *.obj >../%s.lib\n", SysName);
		fprintf(MCFile, "	rm -f *.obj\n");
		fprintf(MCFile, "	touch lib.timestamp\n");
		fprintf(MCFile, "\n");
		fprintf(MCFile, "asm.timestamp:\n");

		/* We break every 100 objects to prevent getting a
		 * "$? list too long" error, which would occur
		 * in our biggest modules.
		 */
		for ( i = 0; i <= (ObjCount-1)/100; i++ ) {
		    fprintf(MCFile, "\n");
		    fprintf(MCFile, "objs%ld:		${OBJS%ld}\n",i,i);
		}
		fprintf(MCFile, "	rm -f *.asm\n");
		fprintf(MCFile, "	touch asm.timestamp\n");
		fprintf(MCFile, "\n");
		fprintf(MCFile, ".INCLUDE=${MAKEMETA}\n");
		fclose(MCFile);
		fprintf(MRFile, "\n\n");
		fprintf(MRFile, "lib.timestamp:	asm.timestamp\n");
		fprintf(MRFile, "	make ");
		for ( i = 0; i <= (ObjCount-1)/100; i++ ) {
		    fprintf(MRFile, "objs%ld ", i );
		}
		fprintf(MRFile, "${MFLAGS} ${MARGS}\n");
		fprintf(MRFile, "	cat *.obj >../%s.lib\n", SysName);
		fprintf(MRFile, "	rm -f *.obj\n");
		fprintf(MRFile, "	touch lib.timestamp\n");
		fprintf(MRFile, "\n");
		fprintf(MRFile, "asm.timestamp:\n");
		for ( i = 0; i <= (ObjCount-1)/100; i++ ) {
		    fprintf(MRFile, "\n");
		    fprintf(MRFile, "objs%ld:		${OBJS%ld}\n",i,i);
		}
		fprintf(MRFile, "	rm -f *.asm\n");
		fprintf(MRFile, "	touch asm.timestamp\n");
		fprintf(MRFile, "\n");
		fprintf(MRFile, ".INCLUDE=${MAKEMETA}\n");
		fclose(MRFile);
	    }
	}
    }
}
# define YYNEWLINE 10
yylex(){
int nstr; extern int yyprevious;
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:
				{ ; }
break;
case 2:
				{ return(TOKEN_NEWLINE); }
break;
case 3:
				{ return(TOKEN_ID); }
break;
case 4:
				{ return(TOKEN_BASE); }
break;
case 5:
			{ return(TOKEN_INCLUDE); }
break;
case 6:
				{ return(TOKEN_BIAS); }
break;
case 7:
			{ return(TOKEN_ALIAS); }
break;
case 8:
			{ return(TOKEN_VARARGS); }
break;
case 9:
			{ return(TOKEN_PRIVATE); }
break;
case 10:
			{ return(TOKEN_PUBLIC); }
break;
case 11:
			{ return(TOKEN_RESERVE); }
break;
case 12:
			{ return(TOKEN_VERSION); }
break;
case 13:
				{ return(TOKEN_END); }
break;
case 14:
			{ return(TOKEN_COMMENT); }
break;
case 15:
{ return(TOKEN_RCSIDHEAD); }
break;
case 16:
		{ return(TOKEN_RCSIDTAIL); }
break;
case 17:
		{ return(TOKEN_FILEPATH); }
break;
case 18:
				{ return(TOKEN_NUMBER); }
break;
case 19:
			{ return(TOKEN_REGISTER); }
break;
case 20:
case 21:
			{ return(TOKEN_BADNAME); }
break;
case 22:
case 23:
			{ return(TOKEN_NAME); }
break;
case 24:
		{ return(TOKEN_BADNAME); }
break;
case 25:
		{ return(TOKEN_NAME); }
break;
case 26:
				{ return(TOKEN_STARS); }
break;
case 27:
				{ return(TOKEN_ELLIPSIS); }
break;
case 28:
				{ return(TOKEN_OPENPARAN); }
break;
case 29:
				{ return(TOKEN_CLOSEPARAN); }
break;
case 30:
				{ return(TOKEN_OPENBRACKET); }
break;
case 31:
				{ return(TOKEN_CLOSEBRACKET); }
break;
case 32:
				{ return(TOKEN_COMMA); }
break;
case 33:
				{ return(TOKEN_DASH); }
break;
case 34:
				{ return(TOKEN_BADNAME); }
break;
case -1:
break;
default:
fprintf(yyout,"bad switch yylook %d",nstr);
} retur