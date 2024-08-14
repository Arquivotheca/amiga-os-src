/*
 *	symba <object> [<object>]
 *
 *	reduce .ld file(s) to one with just the symbols for wack
 */
#define	D(f)

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

#define ext_symb	0
#define ext_defr	1
#define ext_defa	2
#define ext_ref32     129
#define ext_common    130
#define ext_ref16     131
#define ext_ref8      132

#define	TYPEMASK	0x3fffffff
#define	MEMMASK		0xc0000000

char *Prog;
char *FileName;
int Hunks;
FILE *OutFile;
jmp_buf Env;
char InSymbol[256];
char OutSymbol[256];

getlong(inFile, dest)
FILE *inFile;
long *dest;
{
    *dest = getw(inFile);
    if (feof(inFile))
	longjmp(Env, "End of File");
    if (ferror(inFile))
	longjmp(Env, "File Error");
}

tosslongs(inFile, size)
FILE *inFile;
long size;
{
    if (fseek(inFile, size*4, 1) == -1)
	longjmp(Env, "Seek Error");
}

putlong(source)
long source;
{
    putw(source, OutFile);
}

copylongs(inFile, size)
FILE *inFile;
long size;
{
    long target, i;

    for (i = 0; i < size; i++) {
	getlong(inFile, &target);
	putlong(target);
    }
}

scan(inFile, pass)
FILE *inFile;
int pass;
{
    char *error, *outSymbol;
    long hunkType, i, size, ext, value;

    D(("scan(\"%s\", %d);\n", FileName, pass));

    if (outSymbol = (char *) rindex(FileName, '/'))
	strcpy(OutSymbol, outSymbol+1);
    else
    strcpy(OutSymbol, FileName);
    while (outSymbol = (char *) rindex(OutSymbol, '.'))
	*outSymbol = '\0';
    outSymbol = OutSymbol + strlen(OutSymbol);
    *outSymbol++ = '_';

    for (;;) {
	if (error = (char *) setjmp(Env)) {
	    return;
	}

	getlong(inFile, &hunkType);
	
	if (error = (char *) setjmp(Env)) {
	    printf("%s  %s. ABEND\n", FileName, error);
	    return;
	}

	hunkType = hunkType & TYPEMASK;

	D(("hunk %ld", hunkType));
	switch (hunkType) {
	    case hunk_unit:
	    case hunk_name:
	    case hunk_debug:
		getlong(inFile, &size);
		tosslongs(inFile, size);
		break;

	    case hunk_code:
	    case hunk_data:
		getlong(inFile, &size);
		tosslongs(inFile, size);
		putlong(hunkType);
		putlong(0);
		break;

	    case hunk_bss:
		getlong(inFile, &size);
		putlong(hunk_bss);
		putlong(0);
		break;

	    case hunk_reloc32:
	    case hunk_reloc16:
	    case hunk_reloc8:
		do {
		    getlong(inFile, &size);
		    if (size != 0)
			tosslongs(inFile, size+1);
		}
		    while (size != 0);
		break;

	    case hunk_ext:
	    case hunk_symbol:
		putlong(hunkType);
		do {
		    getlong(inFile, &ext);
		    if (ext != 0) {
			size = ext & 0xffffff;
			switch ((ext>>24) & 0xff) {
			    case ext_symb:
			    case ext_defr:
			    case ext_defa:
				for (i = 0; i < size; i++)
				    getlong(inFile, ((long *) InSymbol) + i);
				getlong(inFile, &value);
				if (InSymbol[0] == '.')
				    /* a useless compiler symbol */
				    break;
				InSymbol[size*4] = '\0';
				strcpy(outSymbol, InSymbol);
				size = strlen(OutSymbol);
				for (i = size + 1; i & 3; i++)
				    OutSymbol[i] = '\0';
				size = (size + 3) >> 2;
				putlong((ext & 0xff000000) + size);
				for (i = 0; i < size; i++)
				    putlong(((long *) OutSymbol)[i]);
				putlong(value);
				break;
			    case ext_ref32:
			    case ext_ref16:
			    case ext_ref8:
				tosslongs(inFile, size);
				getlong(inFile, &size);
				tosslongs(inFile, size);
				break;
			    case ext_common:
				tosslongs(inFile, size+1);
				getlong(inFile, &size);
				tosslongs(inFile, size);
				break;
			}
		    }
		}
		    while (ext != 0);
		putlong(0);
		break;

	    case hunk_header:
		do {
		    getlong(inFile, &size);
		    if (size != 0)
			tosslongs(inFile, size);
		}
		    while (size != 0);
		getlong(inFile, &size);
		if (pass == 0)
		    Hunks += size;
		getlong(inFile, &size);
		getlong(inFile, &ext);
		tosslongs(inFile, ext - size + 1);
		if (pass == 0)
		    return;
		break;

	    case hunk_cont:
		tosslongs(inFile, 2);
		getlong(inFile, &size);
		tosslongs(inFile, size);
		putlong(hunk_cont);
		putlong(0);
		putlong(0);
		putlong(0);
		break;

	    case hunk_overlay:
		getlong(inFile, &size);
		getlong(inFile, &ext);
		tosslongs(inFile, size+ext-1);
		putlong(hunk_overlay);
		putlong(0);
		putlong(0);
		break;
		
	    case hunk_end:
	    case hunk_break:
		putlong(hunkType);
		break;
	}
    }
}


main(argc, argv)
int argc;
char *argv[];
{
    FILE *inFile;
    int file, files, hunk, pass;

    Prog = *argv;
    D(("%s\n", Prog));

    Hunks = 0;
    if (argc <= 2) {
	fprintf(stderr, "Usage: %s <out-object> [<in-object>]+\n", Prog);
	exit(1);
    }
    OutFile = fopen(argv[1], "w");
    if (OutFile == NULL) {
	fprintf(stderr, "%s: ERROR cannot open output object file \"%s\"\n",
		Prog, argv[1]);
	exit(1);
    }
    for (pass = 0; pass < 2; pass++) {
	for (file = 2; file < argc; file++) {
	    FileName = argv[file];
	    inFile = fopen(FileName, "r");
	    if (inFile == NULL) {
		fprintf(stderr, "%s: ERROR cannot open object file \"%s\"\n",
			Prog, FileName);
	    }
	    else {
		scan(inFile, pass);
		fclose(inFile);
	    }
	}
	if (pass == 0) {
	    /* output new header */
	    putlong(hunk_header);
	    putlong(0);
	    putlong(Hunks);
	    putlong(0);
	    putlong(Hunks-1);
	    for (hunk = 0; hunk < Hunks; hunk++)
		putlong(0);
	}
    }
}
