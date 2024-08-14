;/* vertextcomp.c - Execute me to compile me with SAS C 6.x
SC vertextcomp.c data=near nominc strmer streq nostkchk saveds ign=73
Slink FROM LIB:c.o,vertextcomp.o TO vertextcomp LIBRARY LIB:SC.lib,LIB:Amiga.lib ND
quit
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#ifdef __SASC
#undef __chkabort
void __regargs __chkabort(void) {}      /* Disable SAS CTRL-C checking. */
#else
#ifdef LATTICE
void chkabort(void) {}                  /* Disable LATTICE CTRL-C checking */
#endif
#endif


#define MINARGS 3

UBYTE *vers = "\0$VER: vertextcomp 40.1";
UBYTE *Copyright = 
  "vertextcomp v40.1\n(c) Copyright 1993 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *usage = "Usage: vertextcomp verfile1 verfile2 (warns if verfile1 newer)";

void bye(UBYTE *s, int e);

void getverrev(UBYTE *buf, int len, int *ver, int *rev);

#define BUFSZ 256

BPTR file1 = NULL, file2 = NULL;
  
void main(int argc, char **argv)
    {
    UBYTE *filename1, *filename2;
    int result = 0, len1, len2, ver1, ver2, rev1, rev2;
    UBYTE buf1[BUFSZ], buf2[BUFSZ];

    if(((argc)&&(argc<MINARGS))||((argc>1)&&(argv[argc-1][0]=='?')))
	{
	printf("%s\n%s\n",Copyright,usage);
	bye("",RETURN_OK);
	}

    ver1 = ver2 = rev1 = rev2 = 0;

    filename1 = argv[1];
    filename2 = argv[2];

    if(!(file1 = Open(filename1,MODE_OLDFILE)))
	{
	printf("%s: file not found\n",filename1);
	bye("",0);
	}
    else
	{
	len1 = Read(file1,buf1,BUFSZ-1);
	if(len1 <= 0)
	    {
	    printf("%s: read error\n",filename1);
	    bye("",0);
	    }
	else
	    {
	    buf1[len1-1] = '\0';
	    getverrev(buf1, len1, &ver1, &rev1);
	    }
	}


    if(!(file2 = Open(filename2,MODE_OLDFILE)))
	{
	printf("%s: file not found\n",filename2);
	bye("",RETURN_WARN);	/* left is newer because right not found */
	}
    else
	{
	len2 = Read(file2,buf2,BUFSZ-1);
	if(len2 <= 0)
	    {
	    printf("%s: read error\n",filename2);
	    bye("",RETURN_WARN);
	    }
	else
	    {
	    buf2[len2-1] = '\0';
	    getverrev(buf2, len2, &ver2, &rev2);
	    }
	}

    /* Compare */
    if((ver1>ver2) || ((ver1==ver2)&&(rev1>rev2)))  result = RETURN_WARN;

    bye("",result);
    }


void getverrev(UBYTE *buf, int len, int *ver, int *rev)
    {
    int k, tver, trev;

    *ver = *rev = tver = trev = 0;


    /* search back for '.' in ver.rev, get revision */
    for(k=len-1; k>=0; k--)
	{
	if(buf[k] == '.') break;
	}

    if((k<0)||(buf[k] != '.')) return;

    trev = atoi(&buf[k+1]);
    buf[k]='\0';


    /* search back for version, get version */
    for( k=k-1; k>=0; k--)
	{
	if(buf[k] == ' ') break;
	}

    if((k<0)||(buf[k] != ' ')) return;

    tver = atoi(&buf[k+1]);

    *ver = tver;
    *rev = trev;

    }

void bye(UBYTE *s, int e)
    {
    if(*s) printf("%s\n",s);
    if(file1) 	Close(file1);
    if(file2) 	Close(file2);
    exit(e);
    }
