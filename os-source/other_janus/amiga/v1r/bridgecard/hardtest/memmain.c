/* testmem.c */

#include    "exec/types.h"
#include    "janus/janus.h"
#include    "janus/janusreg.h"

extern LONG stdin;

extern LONG ExpectedPattern, ActualPattern, BadAddress;

int
NextTest(testNum)
int testNum;
{
    if (WaitForChar(stdin, 10000)) {
	printf("\nEnd of Test\n");
	exit(0);
    }
    return(testNum);
}

sgetnum(si)
char *si;
{
    char *s, c;
    BOOL error;
    WORD factor;
    LONG value;

    s = si;
    value = 0;
    factor = 10;
    while (c = *s++) {
	error = TRUE;
	if (c == 'x') {
	    if (factor == 10) {
		factor = 16;
		error = FALSE; 
	    }
	}
	if ((c >= '0') && (c <= '9')) {
	    value = ULMult(value, factor);
	    value += c - '0';
	    error = FALSE;
	}
	if (factor == 16) {
	    if ((c >= 'A') && (c <= 'F')) {		     
		value = ULMult(value, factor);
		value += c - 'A' + 10;
		error = FALSE;
	    }
	    if ((c >= 'a') && (c <= 'f')) {		     
		value = ULMult(value, factor);
		value += c - 'a' + 10;
		error = FALSE;
	    }
	}
	if (error) {
	    printf("ERROR -- ill formed parameter \"%s\".\n", si);
	    exit(10);
	}
    }
    return(value);
}



main(argc, argv)
int argc;
char *argv[];
{
    LONG memlo, memhi;
    struct JanusAmiga *jbase;
    UBYTE rs, *sc1, *sc2;
    int error;

    memlo = 0x200000;
    memhi = 0;	   

    if (argc >= 2) {
	/* look for "-j" */ 
	sc1 = argv[1];
	sc2 = "-j";
	while ((*sc1++ == *sc2++) && sc1[-1]); 
	if ((sc1[-1] == 0) && (sc2[-1] == 0)) {
	    if (argc >= 5) {
		printf("USAGE: %s -j [0x][<mem-base>] [0x][<mem-size>]\n",
			argv[0]);
		exit(10);
	    }
	    /* we're testing janus memory */
	    printf("%s: janus being configured.\n", argv[0]);
	    jbase = (struct JanusAmiga *) OpenLibrary("janus.library", 0);
	    if (jbase == 0) {
		printf("ERROR -- %s: janus.library failed to open.\n",
			argv[0]);
		exit(10);
	    }
	    /* check the ram size */  
	    if (argc == 4) {
		memhi = sgetnum(argv[3]);
	    }
	    rs = (jbase->ja_IoBase)[jio_RamSize];
	    if (rs & JRAMF_EXISTS) {
		printf("%s: janus ram exists.\n", argv[0]);
	    }
	    else {
		if (memhi == 0) {
		    printf("ERROR -- ");
		}
		printf("%s: ram does not exist (bit %ld is zero).\n",
			argv[0], JRAMB_EXISTS);
		if (memhi == 0) {
		    exit(10);
		}
	    }
	    if (rs & JRAMF_2MEG) {
		printf("%s: 2 Meg ram.\n", argv[0]);
		if (memhi == 0) {
		    memhi = 0x200000;
		}
		else {
		    printf("%s: size explicitly overridden to 0x%lx.\n",
			    argv[0], memhi);
		}
	    }
	    else {
		printf("%s: 1/2 Meg ram.\n", argv[0]);
		if (memhi == 0) {
		    memhi = 0x80000;
		}
		else {
		    printf("%s: size explicitly overridden to 0x%lx.",
			    argv[0], memhi);
		}
	    }
	    /* place the janus memory */
	    if (argc >= 3) {
		memlo = sgetnum(argv[2]);
	    }
	    printf("%s: janus memory placed at 0x%06lx.\n", argv[0], memlo);
	    (jbase->ja_IoBase)[jio_RamBaseAddr] = memlo >> 16;
	} 
	else { 
	    if (argc != 3) {
		printf("USAGE: %s -j [0x][<mem-base>] [0x][<mem-size>]\n",
			argv[0]);
		printf("   or: %s [0x]<mem-low> [0x]<mem-size>\n", argv[0]);
		exit(10);
	    }
	    memlo = sgetnum(argv[1]); 
	    memhi = sgetnum(argv[2]);
	}
    }
    else {
	printf("USAGE: %s -j [0x][<mem-base>] [0x][<mem-size>]\n", argv[0]);
	printf("   or: %s [0x]<mem-low> [0x]<mem-size>\n", argv[0]);
	exit(10);
    }
    memhi += memlo - 1;
    printf("%s: initiating memory test from 0x%06lx thru 0x%06lx.\n", argv[0],
	    memlo, memhi);
    printf("    Hit RETURN to terminate test...\n");
    for (;;) {
	error = DestructiveMemTest(memlo, memhi);
	if (error != 0)
	    printf("Error #0x%lx @ 0x%06lx: Expected 0x%08lx, Got 0x%08lx.\n",
		    error, BadAddress, ExpectedPattern, ActualPattern);
    }
}