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

#define TEMPLATE    "PATH/K,NAME/K,SIZE/N/K,ASCII/S,MOD256/S,RANDOM/S,LONG/S,FILLPATTERN/K,FILLNUMBER/K/N,ARCHIVE/S,NOREAD/S,NOWRITE/S,NOEXECUTE/S,NODELETE/S,OWNERINFO/K/N"
#define OPT_PATH	0
#define OPT_NAME	1
#define OPT_SIZE	2
#define OPT_ASCII	3
#define OPT_MOD256	4
#define OPT_RANDOM   	5
#define OPT_LONG   	6
#define OPT_FILLPATTERN	7
#define OPT_FILLNUMBER	8
#define OPT_ARCHIVE	9
#define OPT_NOREAD	10
#define OPT_NOWRITE	11
#define OPT_NOEXECUTE	12
#define OPT_NODELETE	13
#define OPT_OWNERINFO	14
#define OPT_COUNT	15



void errX(int);


struct RDArgs *rdargs = NULL;

long rc=RETURN_FAIL;

int opens = 10;





int main()
{
	long opts[OPT_COUNT];
	char p[] ="";
	char n[] ="TestFile";
	ULONG size = 1024;
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


	CreateTestFile(path,name,pattern,fillsize,filltype,size,protect_stat,protect_type,own_stat,owninfo);

	errX(0);
}




void errX(int retcode)
	{

	if (rdargs)
		FreeArgs(rdargs);

	exit(retcode);
 	}


