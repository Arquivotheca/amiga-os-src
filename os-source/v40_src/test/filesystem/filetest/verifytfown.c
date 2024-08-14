#include "exec/types.h"
#include "exec/memory.h"
#include "exec/io.h"
#include "exec/libraries.h"
#include "exec/execbase.h"
#include "libraries/dos.h"
#include "libraries/dosextens.h"
#include "workbench/startup.h"
#include "workbench/workbench.h"
#include "create.h"


char *dosalloc(ULONG );
VOID dosfree(ULONG *);
VOID cleanup();





union IDs   {
         UWORD ID16[2];
         ULONG ID32;
         };


char *completepath;
char *buff,*buff1;

BPTR flock = NULL;
struct FileInfoBlock *fibb = NULL;



/*
 * Random number generation ARand.c by:
 *  Fred Mitchell
 *  Systems Evaluation Section
 *  Product Assurance Department
 *  Commodore Technology, Inc.
 */

/******************************************************************************
 *                                                                            *
 *                        System Include Section                              *
 *                                                                            *
 ******************************************************************************/

 /******************************************************************************
 *                                                                            *
 *                      Constant Definition Section                           *
 *                                                                            *
 ******************************************************************************/

 #define AR_SIZE 11

/******************************************************************************
 *                                                                            *
 *                        Global Variable Section                             *
 *                                                                            *
 ******************************************************************************/

static ULONG ar[AR_SIZE] ={
    2108806353,
    1664990704,
    1842421811,
    1421573258,
    149670437,
    582382964,
    380961575,
    636372782,
    1333026745,
    981656632,
    1778907227,
}, o=0; i1=1; i2=2;

/******************************************************************************
 *                                                                            *
 *                     ARand -- Return random ULONG                           *
 *                                                                            *
 ******************************************************************************/

ULONG ARand()
{
    if (++o >= AR_SIZE) o = 0;
    if (++i1 >= AR_SIZE) i1 = 0;
    if (++i2 >= AR_SIZE) i2 = 0;

    return ar[o] = ar[i1] + ar[i2];
}




VerifyTestFile(path,name,fill,fillsize,filltype,filesize,setprotect,protect,setown,own)
	char *path, *name, *fill;
	ULONG	fillsize,filltype,filesize,protect,own;
	BOOL 	setprotect,setown;
	{
	char *buffer;
	struct FileHandle *fh1;
	long i,j,length;
        int 	rand_f = 0;



	completepath = NULL;
	buff = NULL;
	buff1 = NULL;

	{
	long pathlength = strlen(path);


	if(!(completepath = dosalloc(pathlength + strlen(name) + 2)))
		{
		Printf("Memory Allocation Failure\n");
		return(OTHERERROR);
		}
	strcat(completepath,path);
	if((pathlength) && ((path[pathlength - 1]) != ':'))
			strcat(completepath,"/");
	}

	if(filltype == RANDOM_FILL)
		{
		rand_f = !0;
	        filltype = LONG_FILL;
		}


	strcat(completepath,name);


	if(!(fibb = AllocDosObject(DOS_FIB,NULL)))
		{
		testcleanup();
		printf("Memory Allocation Failure\n");
		return(OTHERERROR);
		}

	if(!(flock = Lock(completepath,ACCESS_READ)))
		{
		testcleanup();
		printf("Could not find file\n");
		return(OPENERROR);
		}


 	if(!(Examine(flock,fibb)))
		{
		testcleanup();
		printf("Error Examining file\n");
		return(OPENERROR);
		}


	if(fibb->fib_Size != filesize)
		{
		testcleanup();
		printf("File size Error\n");
		return(NOTEQUAL);
		}



	if(setprotect)
		{
		if(fibb->fib_Protection != protect)
			{
			testcleanup();
			printf("File protection Error\n");
			return(NOTEQUAL);
			}
		}

	if(setown)
		{
		union IDs fID;

		fID.ID16[0] = fibb->fib_OwnerUID;
		fID.ID16[1] = fibb->fib_OwnerGID;


		if(fID.ID32 != own)
			{
			testcleanup();
			printf("File Ownership Error\n");
			return(NOTEQUAL);
			}
		}



	UnLock(flock);
	flock = NULL;

	FreeDosObject(DOS_FIB,fibb);
	fibb = NULL;




	if(!(fh1 = Open(completepath,MODE_OLDFILE)))
		{
		testcleanup();
		printf("Error opening file\n");
		return(OPENERROR);
		}
	if(fillsize)
		buffer = fill;
	else
		{
		if(filltype == MOD_FILL)
			{
			buff = dosalloc(256);
			fillsize = 256;
			for(i=0; i < 256; i++)
				buff[i] = i;
			buffer = buff;
			}
		else
			{
			if(filltype == ASCII_FILL)
				{
				buff = dosalloc(96);
				fillsize = 96;
				for(i=32; i < 128; i++)
					buff[i - 32] = i;
				buffer = buff;
				}
			else if (filltype != LONG_FILL) /* fill with fill character */
				{
				buff = dosalloc(512);
				fillsize = 512;
				for(i=0; i < 512; buff[i++] = filltype);
				buffer = buff;
				}
			}
		}


	if(filltype == LONG_FILL)
		{
		ULONG FillWord = 0;
		ULONG *FILL;
		ULONG k;

		char *read_buf,*com_buf;
		int ii;


		buff = dosalloc(256);
		FILL = (ULONG *)buff;
		fillsize = 256;



		for(i=0; i < 64; i++)
			FILL[i] = rand_f ? ARand() : (i + FillWord);
		buffer = buff;




		buff1 = dosalloc(fillsize);

		for(i = 0,j = fillsize > filesize ? filesize  : fillsize;i < filesize; j = (j + fillsize > filesize) ? filesize  : j + fillsize )
			{
			com_buf = buffer;
			read_buf = buff1;


			length = Read(fh1,buff1,j - i);
			if(length != j - i)
				{
				Close(fh1);
				testcleanup();
				printf("Read Error\n");
				return( (length == -1) ? IoErr() : TOOSMALL);
				}


			for(ii = 0;ii < (j - i);ii++)
				{
				if(*read_buf++ != *com_buf++)
					{
					Close(fh1);
					testcleanup();
					printf("File Verify failed\n");
					return(NOTEQUAL);
					}
				}


			FillWord += 64;


			i = j;
			read_buf = buff1;

			for(ii=0; ii < 64; ii++)
				FILL[ii] = rand_f ? ARand() : (ii + FillWord);
			buffer = buff;
			}





		}

	else
		{
		buff1 = dosalloc(fillsize);
		for(i = 0,j = fillsize > filesize ? filesize  : fillsize;i < filesize; j = (j + fillsize > filesize) ? filesize  : j + fillsize )
			{
			char *read_buf,*com_buf;
			int ii;

			com_buf = buffer;
			read_buf = buff1;
			length = Read(fh1,buff1,j - i);
			if(length != j - i)
				{
				Close(fh1);
				testcleanup();
				printf("Read Error\n");
				return((length == -1) ? IoErr(): TOOSMALL);
				}
			for(ii = 0;ii < (j - i);ii++)
				{
				if(*read_buf++ != *com_buf++)
					{
					Close(fh1);
					testcleanup();
					printf("File Verify failed\n");
					return(NOTEQUAL);
					}
				}
			i = j;
			}
		}


	Close(fh1);
	testcleanup();
	printf("File Verified\n");
	return(OK);

	}

testcleanup()
	{
	if(flock)
		{
		UnLock(flock);
		flock = NULL;
		}
	if(fibb)
		{
		FreeDosObject(DOS_FIB,fibb);
		fibb = NULL;
		}
	if(completepath)
		dosfree(completepath);
	if(buff)
		dosfree(buff);
	if(buff1)
		dosfree(buff1);
	}



char *dosalloc(bytes)
register ULONG bytes;
{
    register ULONG *ptr;

    bytes += 4;
    ptr = (ULONG *)AllocMem(bytes, MEMF_PUBLIC|MEMF_CLEAR);
    *ptr++ = bytes;
    return((char *)ptr);
}

VOID dosfree(ptr)
register ULONG *ptr;
{
    --ptr;
    FreeMem(ptr, *ptr);
}
