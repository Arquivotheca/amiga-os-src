#include <exec/types.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <libraries/dosextens.h>
#include <stdio.h>

#include "clib/exec_protos.h"
#include "clib/dos_protos.h"
#include "clib/intuition_protos.h"

#include "create.h"


/*
extern struct Library *DOSBase;

*/

#define TEMPLATE    "PATH/K,NAME/K,SIZE/N/K,DEPTH/N/K,DIRS/N/K,FILES/N/K,ASCII/S,MOD256/S,RANDOM/S,LONG/S,FILLPATTERN/K,FILLNUMBER/K/N,ARCHIVE/S,NOREAD/S,NOWRITE/S,NOEXECUTE/S,NODELETE/S,OWNERINFO/K/N"
#define OPT_PATH	0
#define OPT_NAME	1
#define OPT_SIZE	2
#define OPT_DEPTH	3
#define OPT_DIRS	4
#define OPT_FILES	5
#define OPT_ASCII	6
#define OPT_MOD256	7
#define OPT_RANDOM   	8
#define OPT_LONG   	9
#define OPT_FILLPATTERN	10
#define OPT_FILLNUMBER	11
#define OPT_ARCHIVE	12
#define OPT_NOREAD	13
#define OPT_NOWRITE	14
#define OPT_NOEXECUTE	15
#define OPT_NODELETE	16
#define OPT_OWNERINFO	17
#define OPT_COUNT	18





void errX(int);


struct RDArgs *rdargs = NULL;

long rc=RETURN_FAIL;

int opens = 10;





int main()
{
	long opts[OPT_COUNT];
	char p[] ="";
	char n[] ="Test";
	ULONG depth = 3;
	ULONG dirs = 3;
	ULONG files = 3;
	ULONG size = 512;
	ULONG filltype = LONG_FILL;
	ULONG protect_type = 0;
	ULONG owninfo = 0;
	BOOL protect_stat = FALSE;
	BOOL own_stat = FALSE;
	char *path,*name,*pattern;
	ULONG filllong;
	ULONG fillsize = 0;

	path = p;
	name = n;


	memset((char *)opts, 0, sizeof(opts));
	if(!(rdargs = ReadArgs(TEMPLATE, opts, NULL)))
		errX(20);

	if(opts[OPT_PATH])
		path=(char *)opts[OPT_PATH];
	if(opts[OPT_NAME])
		name=(char *)opts[OPT_NAME];
	if(opts[OPT_SIZE])
		size=*(ULONG *)opts[OPT_SIZE];
	if(opts[OPT_DEPTH])
		depth=*(ULONG *)opts[OPT_DEPTH];
	if(opts[OPT_DIRS])
		dirs=*(ULONG *)opts[OPT_DIRS];
	if(opts[OPT_FILES])
		files=*(ULONG *)opts[OPT_FILES];
	if(opts[OPT_ASCII])
		filltype  = ASCII_FILL;
	if(opts[OPT_MOD256])
		filltype  = MOD_FILL;
	if(opts[OPT_RANDOM])
		filltype  = RANDOM_FILL;
	if(opts[OPT_LONG])
		filltype  = LONG_FILL;

	if(opts[OPT_FILLPATTERN])
		{
		pattern=(char *)opts[OPT_FILLPATTERN];
		fillsize = strlen(pattern);
		filltype = PATTERN_FILL;
		}

	if(opts[OPT_FILLNUMBER])
		{
		filllong =*(ULONG *)opts[OPT_FILLNUMBER];
		pattern = (char *) &filllong;
		fillsize = sizeof(filllong);
		filltype = PATTERN_FILL;
		}


	if(opts[OPT_ARCHIVE])
		{
		protect_stat = TRUE;
		protect_type    |= 16;
		}
	if(opts[OPT_NOREAD])
		{
		protect_stat = TRUE;
		protect_type    |= 8;
		}
	if(opts[OPT_NOWRITE])
		{
		protect_stat = TRUE;
		protect_type    |= 4;
		}
	if(opts[OPT_NOEXECUTE])
		{
		protect_stat = TRUE;
		protect_type    |= 2;
		}
	if(opts[OPT_NODELETE])
		{
		protect_stat = TRUE;
		protect_type    |= 1;
         	}
	if(opts[OPT_OWNERINFO])
		{
		own_stat = TRUE;
		owninfo = *(ULONG *)opts[OPT_OWNERINFO];
         	}


	VerifyTestTree(path,name,dirs,files,depth,NULL,pattern,fillsize,filltype,size,protect_stat,protect_type,own_stat,owninfo);

	errX(0);
}




void errX(int retcode)
	{

	if (rdargs)
		FreeArgs(rdargs);

	exit(retcode);
 	}


