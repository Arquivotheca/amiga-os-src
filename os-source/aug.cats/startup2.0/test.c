;/* test.c - Execute me to compile me with Lattice 5.04
LC -b1 -cfistq -v -y -j73 test.c
Blink FROM astartup.obj,test.o TO test LIBRARY LIB:Amiga.lib,LIB:LC.lib
quit
*/

#include <exec/types.h>
#include <dos/dostags.h>

#ifdef LATTICE
#include <clib/dos_protos.h>
#include <stdlib.h>
#endif


UBYTE *bigcommand = "test \
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa \
bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb \
ccccccccccccccccccccccccccccccccccccccccccccccccc \
ddddddddddddddddddddddddddddddddddddddddddddddddd \
eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee \
fffffffffffffffffffffffffffffffffffffffffffffffff \
ggggggggggggggggggggggggggggggggggggggggggggggggg \
hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh \
iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii \
jjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj \
kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk \
lllllllllllllllllllllllllllllllllllllllllllllllll \
mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm \
nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn \
ooooooooooooooooooooooooooooooooooooooooooooooooo";

LONG doCommand(UBYTE *command, BPTR other);
VOID printf( BYTE *fmt, ... );
int strlen(UBYTE *);

#define MINARGS 1

UBYTE *vers = "\0$VER: test 36.10";
UBYTE *Copyright = 
  "test v36.10\nCopyright (c) 1990 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *usage = "Usage: test";

void main(int argc, char **argv)
    {
    int k,l;

    if(argc == 0)
	{
	exit(RETURN_OK);
	}
    else if(argc == 1)
	{
	printf("Systeming bigcommand\n");
	doCommand(bigcommand,NULL);
	}
    else
	{
    	printf("argc = %ld, and the args are:\n",argc);
    	for(k=0; k<argc; k++)
	    {
	    if((l=strlen(argv[k])) < 128)
	    	printf("argv %ld = \"%s\"\n",k,argv[k]);
	    else
	    	printf("argv %ld len=%ld (too long for amiga.lib printf)\n",k,l);

	    }
	}
    }


/*
 * Synchronous external command (wait for return)
 * Uses your Input/Output unless you supply other handle
 * Result will be return code of the command, unless the System() call
 * itself fails, in which case the result will be -1
 */
LONG doCommand(UBYTE *command, BPTR other)
    {
    struct TagItem stags[4];

    stags[0].ti_Tag = SYS_Input;
    stags[0].ti_Data = other ? other : Input();
    stags[1].ti_Tag = SYS_Output;
    stags[1].ti_Data = other ? NULL: Output();
    stags[2].ti_Tag = SYS_UserShell;
    stags[2].ti_Data = TRUE;
    stags[3].ti_Tag = TAG_DONE;
    return(System(command, stags));
    }


