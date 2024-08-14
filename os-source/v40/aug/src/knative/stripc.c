#include	<stdio.h>

char *progName;

FILE *srcFile, *destFile;

/* actions */

#define aERROR     0
#define aNOP       1
#define aCOPY      2
#define aNL        3
#define aTABCOPY   4
#define aSLASH     5
#define aSLASHCOPY 6
#define aNLCOPY    7

char *iIT = "\n \t;*:\"',<>";

char *iST[] = {
    /*\n sp \t  ;  *  :  "  '  ,  <  >  X  */
    "\00\02\02\01\01\00\00\00\00\00\00\03",  /* 00 newline */
    "\00\01\01\01\01\01\01\01\01\01\01\01",  /* 01 comment */
    "\00\02\02\01\02\02\02\02\02\02\02\05",  /* 02 initial space */
    "\00\04\04\01\03\04\03\03\03\03\03\03",  /* 03 label token */
    "\00\04\04\01\05\04\04\04\04\04\04\05",  /* 04 space before opcode */
    "\00\06\06\01\05\04\05\05\05\05\05\05",  /* 05 opcode (but maybe label) */
    "\00\06\06\01\10\06\11\13\07\15\06\10",  /* 06 space before operand */
    "\00\01\01\01\10\07\11\13\07\15\07\10",  /* 07 ready for next operand */
    "\00\01\01\01\10\10\11\13\07\10\10\10",  /* 10 operand */
    "\11\11\11\11\11\11\12\11\11\11\11\11",  /* 11 inside " */
    "\00\01\01\01\12\12\11\12\07\10\10\10",  /* 12 " termination */
    "\13\13\13\13\13\13\13\14\13\13\13\13",  /* 13 inside ' */
    "\00\01\01\01\14\14\14\13\07\10\10\10",  /* 14 ' termination */
    "\15\15\15\15\15\15\15\15\15\15\10\15"   /* 15 inside <> */
};

char *iAT[] = {
    /*\n sp \t  ;  *  :  "  '  ,  <  >  X  */
    "\01\01\01\01\01\00\00\00\00\00\00\02",  /* 00 newline */
    "\01\01\01\01\01\01\01\01\01\01\01\01",  /* 01 comment */
    "\01\01\01\01\00\00\00\00\00\00\00\04",  /* 02 initial space */
    "\02\01\01\03\02\01\00\00\00\00\00\02",  /* 03 label token */
    "\02\01\01\03\04\00\00\00\00\00\00\04",  /* 04 space before opcode */
    "\02\01\01\03\02\01\00\00\00\00\00\02",  /* 05 opcode (but maybe label) */
    "\02\01\01\03\04\00\04\04\04\04\00\04",  /* 06 space before operand */
    "\02\03\03\03\02\00\02\02\02\02\00\02",  /* 07 ready for next operand */
    "\02\03\03\03\02\00\02\02\02\02\02\02",  /* 10 operand */
    "\00\02\02\02\02\02\02\02\02\02\02\02",  /* 11 inside " */
    "\02\03\03\03\00\00\02\00\02\02\02\02",  /* 12 " termination */
    "\00\02\02\02\02\02\02\02\02\02\02\02",  /* 13 inside ' */
    "\02\03\03\03\00\00\00\02\02\02\02\02",  /* 14 ' termination */
    "\00\02\02\02\02\02\02\02\02\02\02\02"   /* 15 inside <> */
};


char *hIT = "\n \t/*\"'\\";
char *hST[] = {
    /*\n  b \t  /  *  "  '  \  X  */
    "\00\00\00\01\04\11\13\04\04",        /* 00 newline, initial space */
    "\00\05\05\06\02\11\13\04\04",        /* 01 initial /? */
    "\02\02\02\02\03\02\02\02\02",        /* 02 initial /* */
    "\02\02\02\00\03\02\02\02\02",        /* 03 initial *? */
    "\00\05\05\06\04\11\13\04\04",        /* 04 body text */
    "\00\05\05\06\04\11\13\04\04",        /* 05 body space */
    "\00\05\05\06\07\11\13\04\04",        /* 06 /? */
    "\02\07\07\07\10\07\07\07\07",        /* 07 /* */
    "\02\07\07\05\10\07\07\07\07",        /* 10 *? */
    "\11\11\11\11\11\04\11\12\11",        /* 11 " */
    "\12\11\11\11\11\11\11\11\11",        /* 12 \ in " */
    "\13\13\13\13\13\13\04\14\13",        /* 13 ' */
    "\14\13\13\13\13\13\13\13\13",        /* 14 \ in ' */
};
char *hAT[] = {
    /*\n  b \t  /  *  "  '  \  X  */
    "\01\01\01\01\02\02\02\02\02",        /* 00 newline, initial space */
    "\06\05\05\05\01\06\06\06\06",        /* 01 initial /? */
    "\01\01\01\01\01\01\01\01\01",        /* 02 initial /* */
    "\01\01\01\01\01\01\01\01\01",        /* 03 initial *? */
    "\02\01\01\01\02\02\02\02\02",        /* 04 body text */
    "\02\01\01\01\04\04\04\04\04",        /* 05 body space */
    "\06\05\05\05\01\06\06\06\06",        /* 06 /? */
    "\02\01\01\01\01\01\01\01\01",        /* 07 /* */
    "\02\01\01\01\01\01\01\01\01",        /* 10 *? */
    "\00\02\02\02\02\02\02\02\02",        /* 11 " */
    "\00\02\02\02\02\02\02\02\02",        /* 12 \ in " */
    "\00\02\02\02\02\02\02\02\02",        /* 13 ' */
    "\00\02\02\02\02\02\02\02\02",        /* 14 \ in ' */
};


char *
suffix(name)
char *name;
{
    char *r;

    r = 0;
    while (*name) {
        if (*name == '.') r = name;
        name++;
    }
    if (r == 0) r = name;
    return(r);
}


char
stateMachine(state, input, inputTokens, stateTable, actionTable)
char *state;
char input;
char *inputTokens;
char **stateTable;
char **actionTable;
{
    short i;
    char a;

    i = 0;
    while (*inputTokens != '\0') {
        if (*inputTokens == input) break;
        i++;
        inputTokens++;
    }
    a = actionTable[*state][i];
    *state = stateTable[*state][i];
    return(a);
}

int
outChar(oc)
int oc;
{
    if (putc(oc, destFile) == EOF) return(1);
    return(0);
}


int
stripcopy(inputTokens, stateTable, actionTable)
char *inputTokens;
char **stateTable;
char **actionTable;
{
    char scanState;
    int ic;

    scanState = 0;
    while ((ic = getc(srcFile)) != EOF) {
	switch (stateMachine(&scanState, ic, inputTokens,
		stateTable, actionTable)) {
	    case aERROR:
		if (ic & 0x60) {
		    fprintf(stderr,
			    "%s Internal Error: state %ld input '%lc'\n",
			    progName, scanState, ic);
		    return(-1);
		}
		else {
		    fprintf(stderr,
			    "%s Internal Error: state %ld input 0x%lx\n",
			    progName, scanState, ic);
		    return(-1);
		}
	    case aNOP:
		break;
	    case aTABCOPY:
		if (outChar('\t')) return(-1);
	    case aCOPY:
		if (outChar(ic)) return(-1);
		break;
	    case aNL:
		if (outChar('\n')) return(-1);
		break;
	    case aSLASH:
		if (outChar('/')) return(-1);
		break;
	    case aSLASHCOPY:
		if (outChar('/')) return(-1);
		if (outChar(ic)) return(-1);
		break;
	    case aNLCOPY:
		if (outChar('\n')) return(-1);
		if (outChar(ic)) return(-1);
		break;
        }
    }
    if (ferror(srcFile)) return(-1);
    return(0);
}


int
copy()
{
    int c;

    while ((c = getc(srcFile)) != EOF)
	if (putc(c, destFile) == EOF) return(-1);
    if (ferror(srcFile)) return(-1);
    return(0);
}

main(argc, argv)
int argc;
char *argv[];
{
    int copyOption, usageFlag;
    char *b;
    int c;

    progName = *argv++;
    srcFile = destFile = NULL;

    /* get options */
    copyOption = 0;
    usageFlag = 0;
    while ((--argc > 0) && (**argv == '-')) {
	b = *argv++;
	while (c = *++b) {
	    switch (c) {
		case 'a':
		case 'i':
		    if (copyOption == 2) usageFlag = 1;
		    else copyOption = 1;
		    break;
		case 'c':
		case 'h':
		    if (copyOption == 1) usageFlag = 1;
		    else copyOption = 2;
		    break;
		default:
		    usageFlag = 1;
	    }
	}
    }

    /* get source */
    if (!usageFlag) {
	if (argc-- > 0) {
	    /* infer copy option if appropriate */
	    if (copyOption == 0) {
		b = suffix(*argv);
		if ((strcmp(b, ".i") == 0) || (strcmp(b, ".a") == 0))
		    copyOption = 1;
		else if ((strcmp(b, ".h") == 0) || (strcmp(b, ".c") == 0))
		    copyOption = 2;
	    }
	    /* get source file */
	    srcFile = fopen(*argv, "r");
	    if (srcFile == NULL) {
		fprintf(stderr,
			"Error: %s cannot open source file \"%s\".\n",
			progName, *argv);
		usageFlag = 1;
	    }
	    *argv++;
	}
	else srcFile = stdin;
    }

    /* get destination */
    if (!usageFlag) {
	if (argc-- > 0) {
	    /* infer copy option if appropriate */
	    if (copyOption == 0) {
		b = suffix(*argv);
		if ((strcmp(b, ".i") == 0) || (strcmp(b, ".a") == 0))
		    copyOption = 1;
		else if ((strcmp(b, ".h") == 0) || (strcmp(b, ".c") == 0))
		    copyOption = 2;
	    }
	    /* get destination file */
	    destFile = fopen(*argv, "w");
	    if (destFile == NULL) {
		fprintf(stderr,
			"Error: %s cannot open destination file \"%s\".\n",
			progName, *argv);
		usageFlag = 1;
	    }
	}
	else destFile = stdout;
    }

    if (argc > 0) usageFlag = 1;

    if (!usageFlag) {
	switch (copyOption) {
	    case 0:
		copy();
		break;
	    case 1:
		stripcopy(iIT, iST, iAT);
		break;
	    case 2:
		stripcopy(hIT, hST, hAT);
		break;
	}
    }
    else {
	fprintf(stderr, "Usage: %s [-achi] [<source>] [<destination>]\n",
		progName);
	fprintf(stderr, "    Flag a or i specifies assembler source.\n");
	fprintf(stderr, "    Flag c or h specifies C source.\n");
	fprintf(stderr, "    These flags are mutually exclusive.\n");
	fprintf(stderr,
	"    If no flag is specified, the file suffix is used if .[achi]\n");
    }

    if (srcFile) {
	if (srcFile != stdin) fclose(srcFile);
    }
    if (destFile) {
	if (destFile != stdout) fclose(destFile);
    }
}
