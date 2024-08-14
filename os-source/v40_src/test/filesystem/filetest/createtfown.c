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







char *completepath;
char *buff;





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






CreateTestFile(path,name,fill,fillsize,filltype,filesize,setprotect,protect,setown,own)
	char *path, *name, *fill;
	ULONG	fillsize,filltype,protect,filesize,own;
	BOOL 	setprotect,setown;
	{
	char *buffer;
	struct FileHandle *fh1;
	ULONG i,j,length;
        int 	rand_f = 0;





	completepath = NULL;
	buff = NULL;

	{
	ULONG pathlength = strlen(path);


	if(!(completepath = dosalloc(pathlength + strlen(name) + 2)))
		return(OTHERERROR);
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

	if(!(fh1 = (struct FileHandle *)Open(completepath,MODE_NEWFILE)))
		{


		cleanup();
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
					buff[i - 32 ] = i;
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

		buff = dosalloc(256);
		FILL = (ULONG *)buff;
		fillsize = 256;







		for(i = 0,j = fillsize > filesize ? filesize  : fillsize;i < filesize; j = (j + fillsize > filesize) ? filesize  : j + fillsize )
			{


			for(k=0; k < 64; k++)
				FILL[k] =   rand_f ? ARand() : (k  + FillWord);
			buffer = buff;



			length = Write(fh1,buffer,j - i);
			if(length != j - i)
				{
				Close(fh1);
				cleanup();
				return( (length == -1) ? WRITEERROR : NOROOM);
				}
			i = j;

			FillWord += 64;
			}





		}
	else
		{
		for(i = 0,j = fillsize > filesize ? filesize  : fillsize;i < filesize; j = (j + fillsize > filesize) ? filesize  : j + fillsize )
			{
			length = Write(fh1,buffer,j - i);
			if(length != j - i)
				{
				Close(fh1);
				cleanup();
				return( (length == -1) ? WRITEERROR : NOROOM);
				}
			i = j;
			}

		}


	Close(fh1);
	 /* printf("protect %d \n",protect); */
	if(setprotect)
		{
		SetProtection(completepath,protect);
		}
	if(setown)
		{
		SetOwner(completepath,own);
		}
	cleanup();
	return(OK);

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
VOID cleanup()
	{
	if(completepath)
		dosfree((ULONG *)completepath);
	if(buff)
		dosfree((ULONG *)buff);
	}

