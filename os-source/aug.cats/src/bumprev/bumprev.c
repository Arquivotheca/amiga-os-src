/* bumprev
 * 39.1 - change \n\r to \r\n in .h file (to match asm file)
 *	- strip path from program name before inserting in version strings
 * 	- use revision file
 * 39.2 - append _rev if not present
 */

#define	D(a)

#include	<exec/types.h>

#include	<dos.h>
#include	<time.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	<clib/dos_protos.h>
#include	<pragmas/dos_pragmas.h>

extern void *DOSBase;

#ifdef LATTICE
int CXBRK(void)  { return(0); }  /* Disable Lattice CTRL/C handling */
void chkabort(void) { return; }  /* really */
#endif


#define MINARGS 3

#include "bumprev_rev.h"

UBYTE vers[] = VERSTAG;

UBYTE *usage = "Usage: bumprev version basename [depend-list-file] (e.g. bumprev 39 myprog_rev)";


void main(int argc, char **argv)
    {
    FILE *dFile, *hFile, *iFile, *rFile;
    char dName[32], hName[32], iName[32], rName[32], baseName[32];
    long dTime, rTime;
    long l;
    char clock[8], *d, date[10];
    int version, revision;
    int i, upToDate;

    if(((argc)&&(argc<MINARGS))||((argc>1)&&(argv[argc-1][0]=='?')))
	{
	fprintf(stderr,"%s\n",usage);
	exit(0L);
	}

    if (sscanf(argv[1], "%d", &version) != 1) {
	fprintf(stderr, "Error: illegal <version> \"%s\"\n", argv[1]);
	exit(10);
    }

    /* Append _rev if not present */
    strcpy(baseName, argv[2]);
    i = strlen(baseName);
    if((i<4)||(stricmp(&baseName[i-4],"_rev")))  strcat(baseName,"_rev");

    strcpy(rName, baseName);
    strcat(rName, ".rev");

    strcpy(hName, baseName); 
    strcat(hName, ".h");

    strcpy(iName, baseName);
    strcat(iName, ".i");

    if(argc > 3)
	{
    	/* check dependencies */
    	dFile = fopen(argv[3], "r");
    	if (!dFile)
	    {
	    fprintf(stderr, "Error: cannot open \"%s\"\n", argv[3]);
	    exit(10);
    	    }

    	upToDate = 0;
    	rTime = getft(rName);
    	if (rTime != -1)
	    {
	    while (fscanf(dFile, "%s", dName) == 1)
		{
	    	/* ignore self-dependance */
	    	if (strncmp(baseName, dName, strlen(baseName)) != 0)
		    {
		    dTime = getft(dName);
		    if (dTime > rTime)
			{
		    	upToDate = 0;
		    	break;
			}
		    /* at least one false dependency */
		    upToDate = 1;
	    	    }
		}
    	    }

    	if (upToDate)
	    {
	    if (l = Lock(hName, ACCESS_READ))
		UnLock(l);
	    else
	    	goto bumprev;
	    if (l = Lock(iName, ACCESS_READ))
	    	UnLock(l);
	    else
	    	goto bumprev;
	    fprintf(stderr, "%s: %s is already up-to-date\n", argv[0], rName);
	    exit(0);
    	    }
	}

bumprev:
    /* check for existing .rev file */
    revision = 0;
    rFile = fopen(rName, "r+");
    if (rFile) {
	if (fscanf(rFile, "%d", &revision) != 1) {
	    fprintf(stderr, "Warning: %s file corrupt, resetting revision\n",
		    rFile);
	    revision = 0;
	}
	fseek(rFile, 0, 0);
    }
    else {
	rFile = fopen(rName, "w");
        if (!rFile) {
	    fprintf(stderr, "Error: cannot open file %s for output\n", rFile);
	    exit(20);
	}
	fprintf(stderr, "%s: creating new file %s\n", argv[0], rFile);
    }
    revision++;

    D(("rName: %s\n", rName));

    hFile = fopen(hName, "w");
    D(("hName: %s\n", hName));
    iFile = fopen(iName, "w");
    D(("iName: %s\n", iName));

    if ((!hFile) || (!iFile)) {
        if (!hFile) {
	    fprintf(stderr, "Error: cannot open file %s for output\n", hFile);
	}
        if (!iFile) {
	    fprintf(stderr, "Error: cannot open file %s for output\n", iFile);
	}
	exit(20);
    }

    getclk(clock);
    d = date;

    if (clock[3] > 9) *d++ = '0' + (clock[3]/10);
    *d++ = '0' + (clock[3]%10);
    *d++ = '.';
    if (clock[2] > 9) *d++ = '0' + (clock[2]/10);
    *d++ = '0' + (clock[2]%10);
    *d++ = '.';
    *d++ = '8' + (clock[1]/10);
    *d++ = '0' + (clock[1]%10);
    *d = '\0';

    D(("date: %s\n", date));

    /* strip any path from name before inserting in versions */
    strcpy(rName, baseName);
    for(i=strlen(rName); i >= 0; i--)
	{
	if((rName[i] == '/')||(rName[i] == ':'))
	     {
	     strcpy(rName, &rName[i+1]);
	     break;
	     }
	}

    i = strlen(rName) - 4;
    if (strcmp(rName+i, "_rev") == 0)
	rName[i] = '\0';

    /* output .rev file */
    fprintf(rFile, "%d\n", revision);

    /* output .h file */
    fprintf(hFile, "#define	VERSION		%d\n", version);
    fprintf(hFile, "#define	REVISION	%d\n", revision);
    fprintf(hFile, "#define	DATE	\"%s\"\n", date);
    fprintf(hFile, "#define	VERS	\"%s %d.%d\"\n",
	    rName, version, revision);
    fprintf(hFile, "#define	VSTRING	\"%s %d.%d (%s)\\r\\n\"\n",
	    rName, version, revision, date);
    fprintf(hFile, "#define	VERSTAG	\"\\0$VER: %s %d.%d (%s)\"\n",
	    rName, version, revision, date);

    /* output .i file */
    fprintf(iFile, "VERSION		EQU	%d\n", version);
    fprintf(iFile, "REVISION	EQU	%d\n", revision);
    fprintf(iFile, "DATE	MACRO\n");
    fprintf(iFile, "		dc.b	'%s'\n", date);
    fprintf(iFile, "	ENDM\n");
    fprintf(iFile, "VERS	MACRO\n");
    fprintf(iFile, "		dc.b	'%s %d.%d'\n",
	    rName, version, revision);
    fprintf(iFile, "	ENDM\n");
    fprintf(iFile, "VSTRING	MACRO\n");
    fprintf(iFile, "		dc.b	'%s %d.%d (%s)',13,10,0\n",
	    rName, version, revision, date);
    fprintf(iFile, "	ENDM\n");
    fprintf(iFile, "VERSTAG	MACRO\n");
    fprintf(iFile, "		dc.b	0,'$VER: %s %d.%d (%s)',0\n",
	    rName, version, revision, date);
    fprintf(iFile, "	ENDM\n");

    fprintf(stderr, "%s: bumped %s to revision %d\n", argv[0], baseName,
	    revision);

    fclose(rFile);
    Delay(100);
    D(("done\n"));
}
