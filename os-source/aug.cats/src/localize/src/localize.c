# include	<exec/types.h>
# include	<exec/memory.h>

# include	<exec/lists.h>

# include	<dos/dos.h>
# include	<dos/dosasl.h>

# include	<clib/alib_exec_protos.h>
# include	<clib/dos_protos.h>
# include	<clib/exec_protos.h>

# include	<stdlib.h>
# include	<ctype.h>
# include	<stdio.h>
# include	<string.h>

# define	PLATFORM	"(AmigaDOS 2.1)"
# define	PROGRAM	"localize"
# define	VERSION	"2.9 " PLATFORM

char		*vers = "\0$VER: " PROGRAM " " VERSION;

# define	ADOS_DEFAULT_TEMPLATE	"(catalog?%F(catalog,%I,%S):%S)"

char * make_call ( char * templ, char * func, char * id, char * dflt );
void dopatch ( char * file );
void doparse ( char * file, struct List *, struct List * );
char ** parseargs ( void );
void freeargs ( void );
int find_match ( char *, struct List * );
void drop_exceptions ( struct List * );
void add_excl_string ( struct List *, char *, int, int );
struct List * load_function_exceptions ( char * );
struct List * load_string_exceptions ( char * );


#ifndef	DEF_PREPEND_TYPE
# define	DEF_PREPEND_TYPE	0
# define	DEF_PREPEND		NULL
#endif

#ifndef	DEF_OUTPUT_DIRECTORY
# define	DEF_OUTPUT_DIRECTORY	"Localized-Source"
#endif

#ifndef	DEF_APPEND_TYPE
# define	DEF_APPEND_TYPE	0
# define	DEF_APPEND		NULL
#endif

#ifndef	STRING_FUNCTION_DEFAULT
# define	STRING_FUNCTION_DEFAULT	"GetCatalogStr"
#endif

#ifndef	STRING_EXCEPT
# define	STRING_EXCEPT		"S:Localize-except-string"
#endif

#ifndef	FUNC_EXCEPT
# define	FUNCTION_EXCEPT		"S:Localize-except-function"
#endif

char	*Append = DEF_APPEND;
char	*Directory = DEF_OUTPUT_DIRECTORY;
char	*Except_str = STRING_EXCEPT;
char	*Except_func = FUNCTION_EXCEPT;
char	*GetStrFunc = STRING_FUNCTION_DEFAULT;
char	*MergeCatalog = NULL;
char	*Prepend = DEF_PREPEND;
char	*Templ = NULL;
int	Anchored_Directory = 0;
int	Atype = DEF_APPEND_TYPE;
int	MinString = 1;
int	Mnum = 0;
int	NestOk = 0;
int	Patch = 0;
int	Ptype = DEF_PREPEND_TYPE;
int	Silent = 0;
int	SortCatalog = 0;

void main ( int argc, char * argv [] )
{
    int				i;
    char			**files;
    int				v37 = FALSE;
    struct AnchorPath		*ap = NULL;
    extern struct Library	*DOSBase;
    char			*filename;
    struct List			*slist;
    struct List			*flist;
    
    if(DOSBase->lib_Version >= 37)
	v37 = TRUE;
    files = parseargs();
    
    if (files == NULL || files[0] == NULL)
    {
	printf("No files to process\n");
	return;
    }
    
    if( v37 )
    {
	ap = (struct AnchorPath *)
	    AllocMem(sizeof(struct AnchorPath) + 1024, MEMF_PUBLIC|MEMF_CLEAR);
	    
	if (ap == NULL)
	{

	    return;
	}
    }

    for (i = 0; files[i]; i++)
    {
	if( v37 )
	{
	    ap->ap_BreakBits = SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D;
	    ap->ap_Strlen = 1024;

	    if (MatchFirst(files[i], ap))
		filename = NULL;
	    else
		filename = ap->ap_Buf;
	}
	else
	    filename = files[i];

	if(!filename)
	{
	    printf("no match for %s\n", files[i]);
	    continue;
	}

	while(filename)
	{
	    if (Patch)
		dopatch(filename);
	    else
	    {
		slist = load_string_exceptions(Except_str);
		flist = load_function_exceptions(Except_func);

		doparse(filename, slist, flist);

		drop_exceptions(slist);
		drop_exceptions(flist);
		/*
		doparse(filename, NULL, NULL);
		*/
	    }

	    if (v37 && MatchNext(ap) == 0L)
		filename = ap->ap_Buf;
	    else
		filename = NULL;
	}
    }

    FreeMem(ap, sizeof(struct AnchorPath) + 1024);

    freeargs();
}

void dopatch ( char * filename )
{
    int		line;
    int		col;
    int		scol;
    char	type;
    char	txt[1024];
    char	linebuf[256];
    char	*dflt;
    char	*file;
    char	*fname;
    char	dest[256];
    char	*tag;
    char	*cp1;
    char	*cp2;
    char	curfile[128];
    char	outfile[128];
    char	stringfile[128];
    int		curline = 0;
    int		ccol = 0;
    FILE	*fp = NULL;
    FILE	*fpi;
    FILE	*fpo = NULL;
    FILE	*fps = NULL;
    FILE	*fpx;
    BPTR	dirhandle;
    int		bolter = 0;

    char	fps_buff[256];
    struct List	*stringlist;
    void	NewList ( struct List * );
    void	add_string ( struct List * list, char * text);
    void	dump_all ( FILE * fp, struct List * list );

    stringlist = AllocMem(sizeof(struct List), MEMF_CLEAR);
    
    if (stringlist == NULL)
    {
	printf("Not processing %s, not enough memory\n", filename);
	return;
    }
    
    NewList(stringlist);

    if (!Silent)
	printf("Processing %s...\n", filename);
	
    fpi = fopen(filename, "r");

    if (fpi == NULL)
    {
	fprintf(stderr, "%s: Cannot open \"%s\"\n", PROGRAM, filename);
	FreeMem(stringlist, sizeof(struct List));
	return;
    }
	
    if (MergeCatalog)
    {
	fps = fopen(MergeCatalog, "w");

	if (fps == NULL)
	{
	    fprintf(stderr, "%s: Cannot open \"%s\"\n", PROGRAM, MergeCatalog);
	    fclose(fpi);
	    FreeMem(stringlist, sizeof(struct List));
	    return;
	}

	fprintf(fps, ";\n; Description generated by %s %s from:\n",
		PROGRAM, VERSION);
    }

    while (!feof(fpi))
    {
	if (fgets(linebuf, 512, fpi) == NULL)
	    continue;
	
	file = linebuf;
	cp1 = linebuf;

	while (*cp1 != '|' && *cp1 != '\0')
	    cp1++;

	if (*cp1 == '\0')
	{
	    fprintf(stderr, "%s: invalid string record file \"%s\": aborted\n",
		    PROGRAM, filename);
	    bolter = 1;
	    break;
	}
	
	cp2 = cp1 - 1;
	*cp1++ = '\0';

	while (cp2 > linebuf && *cp2 != ':' && *cp2 != '/')
	    cp2--;
	
	if (cp2 == linebuf)
	    fname = file;
	else
	    fname = cp2 + 1;
	
	if (Anchored_Directory)
	    sprintf(dest, "%s", Directory);
	else
	{
	    if (cp2 == linebuf)
		sprintf(dest, "%s", Directory);
	    else
	    {
		strncpy(dest, file, cp2 - linebuf);
		sprintf(dest + (cp2 - linebuf), "/%s", Directory);
	    }
	}
	
	dirhandle = CreateDir(dest);
	if (dirhandle)
	    UnLock(dirhandle);
	
	if (dest[strlen(dest) - 1] != ':' && dest[strlen(dest) - 1] != '/')
	    strcat(dest, "/");
	
	type = *cp1++;
	*cp1++ = '\0';

	line = atoi(cp1);

	while (*cp1 != '|' && *cp1 != '\0')
	    *cp1++;

	if (*cp1 == '\0')
	{
	    fprintf(stderr, "%s: invalid string record file \"%s\": aborted\n",
		    PROGRAM, filename);
	    bolter = 1;
	    break;
	}


	*cp1++ = '\0';
	
	scol = atoi(cp1);

	while (*cp1 != '|' && *cp1 != '\0')
	    *cp1++;

	if (*cp1 == '\0')
	{
	    fprintf(stderr, "%s: invalid string record file \"%s\": aborted\n",
		    PROGRAM, filename);
	    bolter = 1;
	    break;
	}


	*cp1++ = '\0';
	
	col = atoi(cp1);

	while (*cp1 != '|' && *cp1 != '\0')
	    *cp1++;

	if (*cp1 == '\0')
	{
	    fprintf(stderr, "%s: invalid string record file \"%s\": aborted\n",
		    PROGRAM, filename);
	    bolter = 1;
	    break;
	}

	
	*cp1++ = '\0';

	tag = cp1;

	while (*cp1 != '|' && *cp1 != '\0')
	    *cp1++;

	if (*cp1 == '\0')
	{
	    fprintf(stderr, "%s: invalid string record file \"%s\": aborted\n",
		    PROGRAM, filename);
	    bolter = 1;
	    break;
	}

	
	*cp1++ = '\0';

	dflt = cp1;

	while (*cp1 != '|' && *cp1 != '\0' && *cp1 != '\n')
	    *cp1++;

	if (*cp1 == '\0')
	{
	    fprintf(stderr, "%s: invalid string record file \"%s\": aborted\n",
		    PROGRAM, filename);
	    bolter = 1;
	    break;
	}

	
	*cp1 = '\0';

	if (strcmp(file, curfile) != 0)
	{
	    /* clean up curfile */
	    if (fp != NULL)
	    {
		if (ccol > 0)
		    while (txt[ccol])
			putc(txt[ccol++], fpo);
		
		while (fgets(txt, 1024, fp) != NULL)
		    fputs(txt, fpo);
		fclose(fp);

		if (Atype == 1)
		{
		    fpx = fopen(Append, "r");
		    if (fpx != NULL)
		    {
			if (!Silent)
			    printf("  Appending the contents of %s\n",
				   Append);
			while (fgets(txt, 1024, fpx) != NULL)
			    fputs(txt, fpo);
			fclose(fpx);
		    }
		    else
			fprintf(stderr, "%s: Cannot open \"%s\"\n", PROGRAM,
				Append);
		}
		else if (Atype == 2)
		{
		    if (!Silent)
			printf("  Appending \"%s\"\n", Append);
		    fputs(Append, fpo);
		}

		if (!Silent)
		    printf("  %s done.\n", curfile);
		    
		fclose(fpo);

		if (!MergeCatalog)
		{
		    dump_all(fps, stringlist);
		    fclose(fps);
		}
	    }

	    /* open file */
	    fp = fopen(file, "r");
	    if (fp == NULL)
	    {
		fprintf(stderr, "%s: Cannot open %s", PROGRAM, file);
		curfile[0] = '\0';
		continue;
	    }
		
	    strcpy(curfile, file);
	    strcpy(outfile, dest);
	    strcat(outfile, fname);

	    fpo = fopen(outfile, "w");
	    if (fpo == NULL)
	    {
		fprintf(stderr, "%s: Cannot open %s", PROGRAM, outfile);
		fclose(fp);
		fp = NULL;
		curfile[0] = '\0';
		continue;
	    }
	    
	    strcpy(stringfile, dest);
	    strcat(stringfile, fname);
	    strcat(stringfile, ".cd");

	    if (MergeCatalog)
	    {
		fprintf(fps, ";\t\"%s\"\n", file);
	    }
	    else
	    {
		fps = fopen(stringfile, "w");
		if (fp == NULL)
		{
		    fprintf(stderr,
			    "%s: Cannot open %s", PROGRAM, stringfile);
		    fclose(fp);
		    fclose(fpo);
		    fp = NULL;
		    curfile[0] = '\0';
		    continue;
		}
		else
		{
		    fprintf(fps,
			    ";\n; Description generated by %s %s from:\n",
			    PROGRAM, VERSION, file);
		    fprintf(fps, ";\t\"%s\"\n", file);
		}
	    }

	    if (!Silent)
		printf("  %s -> %s & %s\n", curfile, outfile,
		       MergeCatalog ? MergeCatalog : stringfile);
		
	    fprintf(fpo, "/*\n**\tThis file generated by %s %s from %s\n*/\n",
		    PROGRAM, VERSION, file);
	    if (Ptype == 1)
	    {
		fpx = fopen(Prepend, "r");
		if (fpx != NULL)
		{
		    if (!Silent)
			printf("  Prepending the contents of %s\n",
			       Prepend);
		    while (fgets(txt, 1024, fpx) != NULL)
			fputs(txt, fpo);
		    fclose(fpx);
		}
		else
		    fprintf(stderr, "%s: Cannot open \"%s\"", PROGRAM,
			    Prepend);
	    }
	    else if (Ptype == 2)
	    {
		if (!Silent)
		    printf("  Prepending \"%s\"\n", Prepend);
		    
		fprintf(fpo, "%s\n", Prepend);
	    }

	    curline = 0;
	    ccol = 0;
	}
	
	if (curline < line)
	{
	    if (ccol > 0)
	    {
		while (txt[ccol])
		    putc(txt[ccol++], fpo);
		ccol = 0;
	    }

	    if (fgets(txt, 1024, fp) == NULL)
		continue;
	    
	    curline++;

	    while (curline < line)
	    {
		fputs(txt, fpo);
		if (fgets(txt, 1024, fp) == NULL)
		    break;
	    
		curline++;
	    }
	}
	

	if (!Silent)
	    printf("  patching line %d\n", line);
	    
	while (ccol < (scol - 1))
	    putc(txt[ccol++], fpo);

	if (type == 'S')
	    fputs(make_call(Templ, GetStrFunc, tag, dflt), fpo);
	else if (type == 'E')
	{
	    printf("  line %d (%s) not localized: string has global scope\n",
		   line, tag);
	    fputs(make_call("%D", "", tag, dflt), fpo);
	}
	else if (type == 'D')
	{
	    printf("  line %d (%s) not localized: string part of '#define'\n",
		   line, tag);
	    fputs(make_call("%D", "", tag, dflt), fpo);
	}
	else
	{
	    printf("  line %d (%s) not localized: unrecognized record type\n");
	    continue;
	}
	    

	if (type == 'E')
	    sprintf(fps_buff, ";\n;%s (//)\n;%s\n", tag, dflt);
	else
	    sprintf(fps_buff, ";\n%s (//)\n%s\n", tag, dflt);

	add_string(stringlist, fps_buff);

	ccol = col;
    }
    
    if (fp != NULL)
    {
	
	if (ccol > 0)
	    while (txt[ccol])
		putc(txt[ccol++], fpo);
		
	while (fgets(txt, 1024, fp) != NULL)
	    fputs(txt, fpo);

	fclose(fp);

	if (Atype == 1)
	{
	    fpx = fopen(Append, "r");
	    if (fpx != NULL)
	    {
		if (!Silent)
		    printf("  Appending the contents of %s\n",
			   Append);
		while (fgets(txt, 1024, fpx) != NULL)
		    fputs(txt, fpo);
		fclose(fpx);
	    }
	    else
		fprintf(stderr, "%s: Cannot open \"%s\"\n", PROGRAM,
			Append);
	}
	else if (Atype == 2)
	{
	    printf("  Appending \"%s\"\n", Append);
	    fputs(Append, fpo);
	}

	if (!Silent)
	    printf("  %s done.\n", curfile);
		    
	fclose(fpo);

	dump_all(fps, stringlist);
	fclose(fps);
    }
    
    if (bolter)
    {
	DeleteFile(stringfile);
	DeleteFile(outfile);
    }
    
    if (!Silent)
	printf("%s done\n", filename);
	
    fclose(fpi);

    FreeMem(stringlist, sizeof(struct List));
}

# define	OPTIONS		"Patch/S,Dir=Directory/K,MC=MergeCatalog/K,Sort/S,Minimum=Min/K/N,Nest/S,EXF=ExceptFuncs/K,EXS=ExceptStrings/K,Func=Function/K,Temp=Template/K,PS=PrependString/K,PF=PrependFile/K,AS=AppendString/K,AF=AppendFile/K,SH=Silent/S,Files/A/M"

# define	OPT_PATCH	0
# define	OPT_DIRECTORY	1
# define	OPT_CATALOG	2
# define	OPT_SORT	3
# define	OPT_MIN		4 
# define	OPT_NEST	5 
# define	OPT_EXF         6 
# define	OPT_EXS		7 
# define	OPT_FUNCTION	8 
# define	OPT_TEMPLATE	9 
# define	OPT_PREPENDS	10
# define	OPT_PREPENDF	11
# define	OPT_APPENDS	12
# define	OPT_APPENDF	13
# define	OPT_SILENT	14
# define	OPT_FILES	15
# define	OPT_COUNT	16

struct RDArgs	*Args;
LONG		Result[OPT_COUNT];

char ** parseargs ()
{
    Args = ReadArgs(OPTIONS, Result, NULL);

    if (Args == NULL)
	return NULL;
    
    if ((Patch = (int) Result[OPT_PATCH]) != 0)
    {
	if (Result[OPT_DIRECTORY])
	{
	    Directory = (char *) Result[OPT_DIRECTORY];
	    if (strchr(Directory, ':') != NULL)
		Anchored_Directory = 1;
	}
	else
	    Directory = DEF_OUTPUT_DIRECTORY;
    
	if (Result[OPT_CATALOG])
	    MergeCatalog = (char *) Result[OPT_CATALOG];

	SortCatalog = (int) Result[OPT_SORT];

	if (Result[OPT_FUNCTION])
	    GetStrFunc = (char *) Result[OPT_FUNCTION];
    
	if (Result[OPT_TEMPLATE])
	{
	    Templ = (char *) Result[OPT_TEMPLATE];
	}
	else
	    Templ = ADOS_DEFAULT_TEMPLATE;

	if (Result[OPT_PREPENDF])
	{
	    Ptype = 1;
	    Prepend = (char *) Result[OPT_PREPENDF];
	}
    
	if (Result[OPT_PREPENDS])
	{
	    Ptype = 2;
	    Prepend = (char *) Result[OPT_PREPENDS];
	}
    
	if (Result[OPT_APPENDF])
	{
	    Atype = 1;
	    Append = (char *) Result[OPT_APPENDF];
	}
    
	if (Result[OPT_APPENDS])
	{
	    Atype = 2;
	    Append = (char *) Result[OPT_APPENDS];
	}
    }
    else
    {
	if (Result[OPT_EXF])
	    Except_func = (char *)Result[OPT_EXF];

	if (Result[OPT_EXS])
	    Except_str = (char *)Result[OPT_EXS];

	if (Result[OPT_MIN])
	{
	    MinString = (int) ((long *)Result[OPT_MIN])[0] + 1;

	    if (MinString < 1)
		MinString = 1;
	}

	NestOk = (int)Result[OPT_NEST];
	
    }
    
    Silent = (int) Result[OPT_SILENT];

    return (char **) Result[OPT_FILES];
}

void freeargs ( void )
{
    FreeArgs(Args);
}

char * make_call ( char * templ, char * func, char * id, char * dflt )
{
    static char	buf[512];
    char	*cp = buf;
    char	*tp = templ;
    int		perc = 0;

    while (*tp)
    {
	if (perc)
	{
	    switch(*tp)
	    {
	      case '%':
		*cp++ = *tp;
		break;
		
	      case 'D':
		*cp++ = '"';
		strcpy(cp, dflt);
		cp += strlen(dflt);
		*cp++ = '"';
		break;
		
	      case 'F':
		strcpy(cp, func);
		cp += strlen(func);
		break;
		
	      case 'I':
		strcpy(cp, id);
		cp += strlen(id);
		break;
		
	      case 'S':
		strcpy(cp, id);
		cp += strlen(id);
		*cp++ = '_';
		*cp++ = 'S';
		*cp++ = 'T';
		*cp++ = 'R';
		break;

	      default:
		*cp++ = '%';
		*cp++ = *tp;
		break;
	    }
	    perc = 0;
	    tp++;
	    continue;
	}
	
	if (*tp == '%')
	{
	    perc = 1;
	    tp++;
	    continue;
	}

	*cp++ = *tp++;
    }
    *cp = '\0';
    
    return buf;
}


# define	IDLE		0
# define	COMMENT1	1
# define	COMMENT2	2
# define	COMMENT3	3
# define	COMMENT		4
# define	SQUOTE		5
# define	ESC_SQUOTE	6
# define	DQUOTE		7
# define	ESC_DQUOTE	8
# define	MAYBE_PRE_PROC	9
# define	PRE_PROC	10
# define	C_TOKEN		11
# define	SKIP_STATEMENT	12

# define	MAXSTRLEN	1024
# define	MAXSTATE	8

# define	PUSH(X)	\
    if (stackindex >= MAXSTATE) \
    { \
	fprintf(stderr, \
		"%s: Parse stack overflow (too many nested comments?)\n", \
		PROGRAM); \
	fclose(fpi); \
	return; \
    } \
    else \
    { \
        statestack[stackindex++] = state; \
	state = X; \
    }

# define	POP()	\
    if (stackindex <= 0) \
    { \
	fprintf(stderr, \
		"%s: Parse stack underflow (notify david.miller on BIX)\n", \
		PROGRAM); \
	fclose(fpi); \
	return; \
    } \
    else \
        state = statestack[--stackindex];


void doparse ( char * file, struct List * slist, struct List * flist )
{
    int		c;
    int		line = 0;
    int		col = 1;
    int		scol = -1;
    int		state = IDLE;
    int		statestack[MAXSTATE];
    int		stackindex = 0;
    int		snarf = 0;
    char	txt[MAXSTRLEN];
    char	token[33];
    char	*tokp = token;
    char	*tp = txt;
    int		level = 0;
    FILE	*fpi;
    int		saw_newline = 1;
    int		saw_define = 0;
    int		tlen = 0;
    int		errcnt = 0;
    int		toolate = 0;
    int		plevel;
    int		instr;
    int		escape;

    fpi = fopen(file, "r");
	
    if (fpi == NULL)
    {
	fprintf(stderr, "%s: Cannot open \"%s\"\n", PROGRAM, file);
	return;
    }
	
    memset(txt, 0, 1024);
    
    while ((c = getc(fpi)) != EOF)
    {
	if (saw_newline)
	{
	    saw_define = saw_newline = 0;
	    line ++;
	    col = 1;
	    toolate = 0;
	}
	else
	    col ++;

	if (c == '\n')
	    saw_newline = 1;
	
	if (snarf)
	{
	    *tp++ = c;
	    if (saw_newline)
	    {
		fprintf(stderr, "%s: line %d, newline in string contxt\n",
			PROGRAM, line);
		errcnt++;
		tp = txt;
		tlen = 0;
		snarf = 0;
		state = IDLE;
	    }
	    if (++tlen >= MAXSTRLEN)
	    {
		fprintf(stderr, "%s: line %d, string too long!\n",
			PROGRAM, line);
		errcnt++;
		tp = txt;
		tlen = 0;
		snarf = 0;
	    }

	    if (errcnt > 10)
	    {
		fprintf(stderr, "%s: cascading errors, aborting file\n",
			PROGRAM);
		fclose(fpi);
		return;
	    }

	}
	
	switch (state)
	{
	  case IDLE:
	    switch(c)
	    {
	      case '/':
		PUSH(COMMENT1);
		break;

	      case '{':
		level++;
		toolate = 1;
		break;

	      case '}':
		level--;
		if (level < 0)
		    level = 0;
		toolate = 1;
		break;

	      case '\'':
		PUSH(SQUOTE);
		toolate = 1;
		break;

	      case '"':
		PUSH(DQUOTE);
		snarf = 1;
		scol = col;
		toolate = 1;
		break;
		
	      case '#':
		if (!toolate)
		    PUSH(MAYBE_PRE_PROC);
		break;

	      case ' ':
	      case '\t':
		break;
		
	      default:
		if (isalpha(c) || c == '_' || c == '$')
		{
		    PUSH(C_TOKEN);
		    tokp = token;
		    *tokp++ = c;
		}
		toolate = 1;
		break;
	    }
	    break;
	    
	  case COMMENT1:	/* TRANSIENT */
	    switch(c)
	    {
	      case '*':
		state = COMMENT;
		break;

	      case '/':
		state = COMMENT3;
		break;

	      default:
		POP();
		break;
	    }
	    
	  case COMMENT:
	    switch(c)
	    {
	      case '/':
		if (NestOk)
		    PUSH(COMMENT1);
		break;
		
	      case '*':
		state = COMMENT2;
		break;
	    }
	    break;
	    
	  case COMMENT2:	/* TRANSIENT */
	    switch(c)
	    {
	      case '/':
		POP();
		break;
		
	      case '*':
		break;

	      default:
		state = COMMENT;
		break;
	    }
	    break;
	    
	  case COMMENT3:
	    switch(c)
	    {
	      case '\n':
		POP();
		break;
	    }
	    break;

	  case SQUOTE:
	    switch(c)
	    {
	      case '\'':
		POP();
		break;
		
	      case '\\':
		state = ESC_SQUOTE;
		break;
	    }
	    break;
	    
	  case ESC_SQUOTE:	/* TRANSIENT */
	    state = SQUOTE;
	    break;
	    
	  case DQUOTE:
	    switch(c)
	    {
	      case '"':
		*(tp - 1) = '\0';
		if (snarf && tlen >= MinString)
		{
		    if (slist == NULL || find_match(txt, slist) == 0)
#ifdef	CLEVER
		    {
			if (saw_define)
			{
			    sprintf(buff, "%s|%d|%d|%d|%s|MSG_%04d|%s\n",
				    file,
				    line,
				    scol,
				    col,
				    define_name,
				    Mnum++,
				    txt);
			    addstring(plist, buf, 1);
			}
			else
			{
			    sprintf(buff, "%s|%d|%d|%d|%c|MSG_%04d|%s\n",
				    file,
				    line,
				    scol,
				    col,
				    (level ? ('S') : ('E')),
				    Mnum++,
				    txt);
			    addstring(plist, buf, 0);
			}
#else
			printf("%s|%c|%d|%d|%d|MSG_%04d|%s\n",
			       file,
			       (level ? ('S') : (saw_define ? ('D') : ('E'))),
			       line,
			       scol,
			       col,
			       Mnum++,
			       txt);
#endif
		}
		
		snarf = 0;
		scol = -1;
		tp = txt;
		*tp = '\0';
		tlen = 0;
		POP();
		break;
		
	      case '\\':
		state = ESC_DQUOTE;
		break;
	    }
	    break;

	  case ESC_DQUOTE:	/* TRANSIENT */
	    state = DQUOTE;
	    break;

	  case MAYBE_PRE_PROC:	/* TRANSIENT */
	    switch (c)
	    {
	      case ' ':
	      case '\t':
		break;
		
	      case 'd':
		saw_define = 1;
		POP();
		break;

	      default:
		state = PRE_PROC;
		break;
	    }
	    break;

	  case PRE_PROC:
	    if (c == '\n')
		POP();
	    break;

	  case C_TOKEN:
	    if (isalnum(c) || c == '_' || c == '$')
		*tokp++ = c;
	    else
	    {
		ungetc(c, fpi);
		col--;
		*tokp = '\0';
		
		POP();
		
		if (FindName(flist, token))
/*
		if (strcmp(token, "foo") == 0)
*/
		{
		    escape = instr = plevel = 0;
		    PUSH(SKIP_STATEMENT);
		}
	    }
	    break;

	  case SKIP_STATEMENT:
	    if (instr)
	    {
		switch(c)
		{
		  case '\\':
		    escape = !escape;
		    break;
		    
		  case '"':
		    if (escape)
			escape = 0;
		    else
			instr = 0;
		    break;

		  default:
		    escape = 0;
		    break;
		}
	    }
	    else
	    {
		if (isspace(c))
		    break;

		if (c == '(')
		    plevel++;

		if (c == ')')
		    plevel--;

		if (c == '"')
		    instr = 1;
		
		if (plevel <= 0)
		{
		    ungetc(c, fpi);
		    col--;
		    POP();
		    break;
		}
	    }
	    break;
	}
    }

    fclose(fpi);
}

void dump_all ( FILE * fp, struct List * list )
{
    struct Node	*next;
    struct Node	*node;

    if (list->lh_TailPred != (struct Node *)list)
	for (node = list->lh_Head; node->ln_Succ; node = next)
	{
	    next = node->ln_Succ;

	    fputs(node->ln_Name, fp);
	
	    Remove(node);
	    
	    FreeMem(node->ln_Name, strlen(node->ln_Name) + 1);
	    FreeMem(node, sizeof(struct Node));
	}
}

void add_string ( struct List * list, char * text )
{
    struct Node	*newnode;
    struct Node	*node;
    char	*a, *b;
    
    /*
    **	Position after ";\n"
    */
    a = text + 2;
    while (a[0] == ';')
	a++;
    
    for (node = list->lh_Head; node->ln_Succ; node = node->ln_Succ)
    {
	b = node->ln_Name + 2;
	while (b[0] == ';')
	    b++;
    
	if (strcmp(b, a) == 0)
	    return;
    }

    newnode = (struct Node *)AllocMem(sizeof(struct Node), MEMF_CLEAR);

    if (newnode == NULL)
	return;
    
    newnode->ln_Name = AllocMem(strlen(text) + 1, MEMF_CLEAR);

    if (newnode->ln_Name == NULL)
    {
	FreeMem(newnode, sizeof(struct Node));
	return;
    }
    
    strcpy(newnode->ln_Name, text);

    if (SortCatalog)
	for (node = list->lh_Head; node->ln_Succ; node = node->ln_Succ)
	{
	    b = node->ln_Name + 2;
	    while (b[0] == ';')
		b++;
    
	    if (strcmp(b, a) > 0)
	    {
		Insert(list, newnode, node->ln_Pred);
		return;
	    }
	}

    AddTail(list, newnode);
}

char	txt[1024];
char	dst[2080];
char	buff[1024];

struct List * load_string_exceptions ( char * file )
{
    FILE	*fpi;
    char	*bp;
    char	*cp;
    int		exact;
    int		nocase;
    int		ok;
    int		line = 0;
    int		state;
    char	*tp;
    int		result;
    int		done;
    struct List	*slist;
    
    slist = AllocMem(sizeof(struct List), MEMF_CLEAR);
    
    if (slist == NULL)
    {
	printf("Not enough memory\n");
	return NULL;
    }
    
    NewList(slist);

    fpi = fopen(file, "r");
    
    if (fpi == NULL)
	return slist;
    
    while ((bp = fgets(buff, sizeof(buff), fpi)) != NULL)
    {
	line++;
	
	while (isspace(*bp) && *bp != '\0')
	    bp++;
	
	if (*bp == '\0')
	    continue;
	
	cp = bp;

	if (*cp == ';')
	    continue;
	
	while (!isspace(*cp) && *cp != '\0')
	    cp++;
	
	if (*cp == '\0')
	    continue;
	
	*cp++ = '\0';

	ok = 0;
	
	if (stricmp(bp, "match") == 0)
	{
	    exact = 1;
	    nocase = 1;
	    ok++;
	}
		    
	if (stricmp(bp, "cmatch") == 0)
	{
	    exact = 1;
	    nocase = 0;
	    ok++;
	}

	if (stricmp(bp, "contains") == 0)
	{
	    exact = 0;
	    nocase = 1;
	    ok++;
	}
	
	if (stricmp(bp, "ccontains") == 0)
	{
	    exact = 0;
	    nocase = 0;
	    ok++;
	}
	
	if (ok == 0)
	{
	    fprintf(stderr, "%s: %s:%d - unrecognized keyword\n",
		    PROGRAM, file, line);
	}

	if (!exact)
	{
	    strcpy(txt, "#?");
	    tp = txt + 2;
	}
	else
	    tp = txt;
		
	for (state = IDLE, done = 0; !done && *cp; cp++)
	    switch (state)
	    {
	      case IDLE:
		if (*cp == '"')
		    state = DQUOTE;
		break;
	    
	      case DQUOTE:
		if (*cp == '"')
		{
		    *tp = '\0';
		    done = 1;
		}
		else
		{
		    if (*cp == '\\')
			state = ESC_DQUOTE;

		    *tp++ = *cp;
		}
		break;

	      case ESC_DQUOTE:
		*tp++ = *cp;
		state = DQUOTE;
		break;
	    }

	if (!exact)
	    strcat(txt, "#?");

	if (nocase)
	{
	    result = ParsePatternNoCase(txt, dst, sizeof(dst));
	}
	else
	{
	    result = ParsePattern(txt, dst, sizeof(dst));
	}

	if (result == -1)
	{
	    fprintf(stderr,
		    "%s: string exclusion pattern in %s, line %d too long\n",
		    PROGRAM, file, line);
	}
	else
	{
	    add_excl_string(slist, dst, strlen(dst), nocase);
	}
    }
    fclose(fpi);
    
    return slist;
}

struct List * load_function_exceptions ( char * file )
{
    FILE	*fpi;
    char	*bp;
    char	*cp;
    int		len;
    struct List	*flist;
    
    flist = AllocMem(sizeof(struct List), MEMF_CLEAR);
    
    if (flist == NULL)
    {
	printf("Not enough memory\n");
	return NULL;
    }
    
    NewList(flist);

    fpi = fopen(file, "r");
    
    if (fpi == NULL)
	return flist;
    
    while ((bp = fgets(buff, sizeof(buff), fpi)) != NULL)
    {
	while (isspace(*bp) && *bp != '\0')
	    bp++;
	
	if (*bp == '\0')
	    continue;
	
	cp = bp;

	while (!isspace(*cp) && *cp != '\0')
	    cp++;
	
	if (*cp != '\0')
	    *cp = '\0';

	len = strlen(bp);
	
	if (len > 0)
	    add_excl_string(flist, bp, len, 0);
    }

    return flist;
}

void add_excl_string ( struct List * list, char * text, int len, int pri )
{
    struct Node	*newnode;
    struct Node	*node;

    node = FindName(list, text);
    
    if (node)
    {
	if (node->ln_Pri == pri)
	    return;
    }
    
    newnode = (struct Node *)AllocMem(sizeof(struct Node), MEMF_CLEAR);

    if (newnode == NULL)
	return;
    
    newnode->ln_Name = AllocMem(len + 1, MEMF_CLEAR);

    if (newnode->ln_Name == NULL)
    {
	FreeMem(newnode, sizeof(struct Node));
	return;
    }
    
    strcpy(newnode->ln_Name, text);

    newnode->ln_Pri = pri;

    AddTail(list, newnode);
}

void drop_exceptions ( struct List * list )
{
    struct Node	*next;
    struct Node	*node;

    if (list->lh_TailPred != (struct Node *)list)
	for (node = list->lh_Head; node->ln_Succ; node = next)
	{
	    next = node->ln_Succ;

	    Remove(node);
	    
	    FreeMem(node->ln_Name, strlen(node->ln_Name) + 1);
	    FreeMem(node, sizeof(struct Node));
	}

    FreeMem(list, sizeof(struct List));
}

int find_match ( char * txt, struct List * slist )
{
    struct Node	*node;
    int		result;

    if (slist->lh_TailPred != (struct Node *)slist)
	for (node = slist->lh_Head; node->ln_Succ; node = node->ln_Succ)
	{
	    if (node->ln_Pri)
		result = MatchPatternNoCase(node->ln_Name, txt);
	    else
		result = MatchPattern(node->ln_Name, txt);

	    if (result)
		return 1;
	}
    return 0;
}