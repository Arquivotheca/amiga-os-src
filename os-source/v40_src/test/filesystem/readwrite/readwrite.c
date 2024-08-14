#include <exec/types.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <libraries/dosextens.h>
#include <stdio.h>

/*#include "clib/exec_protos.h"
#include "clib/dos_protos.h"
#include "clib/intuition_protos.h"
*/


#include "create.h"

extern struct Library *DOSBase;

#define TEMPLATE    "PATH/K,NAME/K,MINSIZE/N/K,MAXSIZE/N/K,ITERATION/N/K"
#define OPT_PATH	0
#define OPT_NAME	1
#define OPT_MINSIZE	2
#define OPT_MAXSIZE     3
#define OPT_ITERATION	4
#define OPT_COUNT   	5


void errX(int);


struct RDArgs *rdargs = NULL;

long rc=RETURN_FAIL;

int opens = 10;





int main()
{
	long opts[OPT_COUNT];
	char p[] ="";
	char n[] ="TestFile";
	ULONG minsize = 16;
	ULONG maxsize = 1000000;
	ULONG iteration = 16;
	ULONG size;
	char *path,*name;
	int errorR;


	path = p;
	name = n;


	memset((char *)opts, 0, sizeof(opts));
	if(!(rdargs = ReadArgs(TEMPLATE, opts, NULL)))
		errX(20);

	if(opts[OPT_PATH])
		path=(char *)opts[OPT_PATH];
	if(opts[OPT_NAME])
		name=(char *)opts[OPT_NAME];
	if(opts[OPT_MINSIZE])
		minsize=*(ULONG *)opts[OPT_MINSIZE];
	if(opts[OPT_MAXSIZE])
		maxsize=*(ULONG *)opts[OPT_MAXSIZE];
	if(opts[OPT_ITERATION])
		iteration=*(ULONG *)opts[OPT_ITERATION];

	printf("File size \n\n");

	for(size = minsize; size < maxsize; size += iteration)
		{
		errorR = CreateTestFile(path,name,NULL,NULL,LONG_FILL,size,NULL,NULL);


		if(errorR == OPENERROR)
			Printf("Error Opening file\n");

		if(errorR == WRITEERROR)
			Printf("Error Writing file\n");

		if(errorR == NOROOM)
			Printf("file too long for device\n");

		if(errorR)
			errX(errorR);

		errorR = VerifyTestFile(path,name,NULL,NULL,LONG_FILL,size);


		if(errorR)
			{

			if(errorR == NOTEQUAL)
				Printf("Write Verify failed\n");
			else
				Printf("IO ERROR %ld\n",errorR);
			errX(errorR);
			}



		 printf("%d \x0D",size);


		}


printf("\n\n");


	errX(0);
}




void errX(int retcode)
	{

	if (rdargs)
		FreeArgs(rdargs);

	exit(retcode);
 	}


