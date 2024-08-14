/*
 *	stripf <amiga-file> <stripped>
 *
 *	strip all symbols and debug information from an amiga executable,
 *	copy other files straight.
 */
#define	DP(f)

#include	<stdio.h>
#include	<setjmp.h>

#define hunk_unit     999
#define hunk_name    1000
#define hunk_code    1001
#define hunk_data    1002
#define hunk_bss     1003
#define hunk_reloc32 1004
#define hunk_reloc16 1005
#define hunk_reloc8  1006
#define hunk_ext     1007
#define hunk_symbol  1008
#define hunk_debug   1009
#define hunk_end     1010
#define hunk_header  1011
#define hunk_cont    1012
#define hunk_overlay 1013
#define hunk_break   1014
#define	hunk_dreloc32 1015
#define	hunk_dreloc16 1016
#define	hunk_dreloc8 1017
#define	hunk_lib_hunk 1018
#define	hunk_lib_index 1019


#define ext_symb	0
#define ext_defr	1
#define ext_defa	2
#define ext_ref32     129
#define ext_common    130
#define ext_ref16     131
#define ext_ref8      132
#define	ext_dref32    133
#define	ext_dref16    134
#define	ext_dref8     135

#define	TYPEMASK	0x3fffffff
#define	MEMMASK		0xc0000000

char *Prog;
char *File;
FILE *InFile;
FILE *OutFile;

#define COPYBUFSIZE	1024
char CopyBuffer[COPYBUFSIZE];

jmp_buf Env;

#define	error_usage	1
#define	error_iopen	2
#define	error_ierr	3
#define	error_iseek	4
#define	error_ieof	5
#define	error_oopen	6
#define	error_oerr	7
#define	error_oseek	8

char *ErrorMsgs[] = {
    (char *) 0, "",
    "Cannot Open <executable> Input File %s",
    "Input File Error",
    "Input Seek Error",
    "Premature End of Input File",
    "Cannot Open <stripped> Output File %s",
    "Output File Error",
    "Output Seek Error"
};

struct HunkName {
    char   *hn_Name;
    int     hn_Value;
} HunkNames[] = {
    { "unit", hunk_unit },
    { "name", hunk_name },
    { "reloc16", hunk_reloc16 },
    { "reloc8", hunk_reloc8 },
    { "ext", hunk_ext },
    { "dreloc32", hunk_dreloc32 },
    { "dreloc16", hunk_dreloc16 },
    { "dreloc8", hunk_dreloc8 },
    { "lib_hunk", hunk_lib_hunk },
    { "lib_index", hunk_lib_index }
};

char HunkString[16];

char *
hunkstring(hunk)
int hunk;
{
    short i;

    for (i = 0; i < (sizeof(HunkNames)/sizeof(struct HunkName)); i++) {
	if (HunkNames[i].hn_Value == hunk) {
	    strcpy(HunkString, HunkNames[i].hn_Name);
	    return(HunkString);
	}
    }
    sprintf(HunkString, "unknown (%d)", hunk);
    return(HunkString);
}


void
warnhunk(hunk)
int hunk;
{
    fprintf(stderr, "%s: WARNING -- skipping %s hunk\n", Prog,
	    hunkstring(hunk));
}


long
getlong()
{
    long result;

    fread((char *) &result, sizeof(result), 1, InFile);
    if (feof(InFile))
	longjmp(Env, error_ieof);
    if (ferror(InFile))
	longjmp(Env, error_ierr);
    return(result);
}


void
putlong(data)
long data;
{
    fwrite((char *) &data, sizeof(data), 1, OutFile);
    if (ferror(OutFile))
	longjmp(Env, error_oerr);
}


void
tosslong(size)
long size;
{
    if (fseek(InFile, size*4, 1) == -1)
	longjmp(Env, error_iseek);
}


long
copylong(size)
long size;
{
    long data, zeroDetect;

    zeroDetect = 0;
    for (; size > 0; size--) {
	fread((char *) &data, sizeof(data), 1, InFile);
	fwrite((char *) &data, sizeof(data), 1, OutFile);
	zeroDetect |= data;
    }
    if (feof(InFile))
	longjmp(Env, error_ieof);
    if (ferror(InFile))
	longjmp(Env, error_ierr);
    if (ferror(OutFile))
	longjmp(Env, error_oerr);

    return(zeroDetect);
}


void
copyfile()
{
    int actual;

    do {
	actual = fread(CopyBuffer, 1, COPYBUFSIZE, InFile);
	if (actual != 0)
	    fwrite(CopyBuffer, 1, actual, OutFile);
    }
	while (actual != 0);
    exit(0);
}


main(argc, argv)
int argc;
char *argv[];
{
    int errorCode;
    long hunkLong, hunkType, size, ext;

    Prog = *argv++;

    DP(("before setjmp\n"));
    if (errorCode = setjmp(Env)) {
	if (errorCode > error_usage) {
	    fprintf(stderr, "ERROR: ");
	    fprintf(stderr, ErrorMsgs[errorCode], File);
	    fprintf(stderr, "\n");
	}
	fprintf(stderr, "Usage: %s <executable> <stripped>\n", Prog);
	exit(1);
    }

    DP(("arg checking %d\n", argc));
    if (argc != 3) {
	longjmp(Env, error_usage);
    }
    DP(("open input\n"));
    File = *argv++;
    InFile = fopen(File, "r");
    if (InFile == NULL) {
	longjmp(Env, error_iopen);
    }
    DP(("open output\n"));
    File = *argv;
    OutFile = fopen(File, "w");
    if (OutFile == NULL) {
	longjmp(Env, error_oopen);
    }

    /* inspect for executable file */
    if (setjmp(Env)) {
	/* empty file */
	exit(0);
    }

    hunkLong = getlong();

    if ((hunkLong & TYPEMASK) != hunk_header) {
	/* copy non-executable file */
	fseek(InFile, 0, 0);
	copyfile();
    }

    DP(("loop\n"));
    for (;;) {

	if (errorCode = setjmp(Env)) {
	    fprintf(stderr, "WARNING: ");
	    fprintf(stderr, ErrorMsgs[errorCode]);
	    fprintf(stderr, "\n    copying to %s unstripped\n", File);
	    fseek(InFile, 0, 0);
	    fseek(OutFile, 0, 0);
	    /* copy file again, unstripped */
	    copyfile();
	}

	hunkType = hunkLong & TYPEMASK;

	DP(("hunk %ld", hunkType));
	switch (hunkType) {
	    case hunk_unit:
	    case hunk_name:
		warnhunk(hunkType);
	    case hunk_debug:
		size = getlong();
		tosslong(size);
		break;

	    case hunk_code:
		putlong(hunkLong);
		size = getlong();
		putlong(size);
		copylong(size);
		break;

	    case hunk_data:
		putlong(hunkLong);
		size = getlong();
		putlong(size);
		if (copylong(size) == 0) {
		    /* convert to hunk_bss */
		    if (fseek(OutFile, -((size+2)*4), 1) == -1)
			longjmp(Env, error_iseek);
		    putlong((hunkLong & MEMMASK) | hunk_bss);
		    putlong(size);
		}
		break;

	    case hunk_bss:
		putlong(hunkLong);
		copylong(1);
		break;

	    case hunk_reloc32:
		putlong(hunkType);
		do {
		    size = getlong();
		    putlong(size);
		    if (size != 0)
			copylong(size+1);
		}
		    while (size != 0);
		break;

	    case hunk_reloc16:
	    case hunk_reloc8:
	    case hunk_dreloc32:
	    case hunk_dreloc16:
	    case hunk_dreloc8:
		warnhunk(hunkType);
		do {
		    size = getlong();
		    if (size != 0)
			tosslong(size+1);
		}
		    while (size != 0);
		break;

	    case hunk_ext:
		warnhunk(hunkType);
	    case hunk_symbol:
		do {
		    ext = getlong();
		    if (ext != 0) {
			size = ext & 0xffffff;
			switch ((ext>>24) & 0xff) {
			    case ext_symb:
			    case ext_defr:
			    case ext_defa:
				tosslong(size+1);
				break;
			    case ext_ref32:
			    case ext_ref16:
			    case ext_ref8:
			    case ext_dref32:
			    case ext_dref16:
			    case ext_dref8:
				tosslong(size);
				size = getlong();
				tosslong(size);
				break;
			    case ext_common:
				tosslong(size+1);
				size = getlong();
				tosslong(size);
				break;
			}
		    }
		}
		    while (ext != 0);
		break;

	    case hunk_header:
		putlong(hunkLong);
		do {
		    size = getlong();
		    putlong(size);
		    if (size != 0)
			copylong(size);
		}
		    while (size != 0);
		copylong(1);
		size = getlong();
		putlong(size);
		ext = getlong();
		putlong(ext);
		copylong(ext-size+1);
		break;

	    case hunk_cont:
		putlong(hunkLong);
		copylong(2);
		size = getlong();
		putlong(size);
		copylong(size);
		break;

	    case hunk_overlay:
		putlong(hunkLong);
		size = getlong();
		putlong(size);
		copylong(size);
		break;
		
	    case hunk_end:
	    case hunk_break:
		putlong(hunkLong);
		break;
	}

	/* get next hunk */
	if (setjmp(Env)) {
	    if ((hunkLong == hunk_end) || (hunkLong == hunk_break))
		/* honorable exit */
		exit(0);
	    fprintf(stderr, "%s: WARNING -- Ended on %s hunk\n", Prog,
		    hunkstring(hunkType));
	    exit(1);
	}

	hunkLong = getlong();
    }
}

