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


VerifyTestDir(path,name,files,filenames,fill,fillsize,filltype,filesize,setprotect,protect,setown,own)
	char 	*path, *name, *fill, **filenames;
	ULONG	files, fillsize,filltype,filesize,protect,own;
	BOOL	setprotect,setown;
	{
	ULONG num_files;
	char Fname[50];
	long pathlength = strlen(path);
	char *completedpath;
	struct FileLock *Dlock;

	if(filenames)
		return(OTHERERROR); /* not implimented yet */

	if(!(completedpath = dosalloc(pathlength + strlen(name) + 2)))
		return(OTHERERROR);
	strcpy(completedpath,path);
	if((pathlength) && ((path[pathlength - 1]) != ':'))
			strcat(completedpath,"/");
	strcat(completedpath,name);

	for(num_files =0;num_files < files;num_files++)
		{
		sprintf(&Fname[0],"F%d", num_files);
		if((VerifyTestFile(completedpath,&Fname[0],fill,fillsize,filltype,filesize,setprotect,protect,setown,own)))
			{
			printf("Verify failed\n");
			dosfree(completedpath);
			return(OTHERERROR);
			}
		}
	dosfree(completedpath);
	return(OK);
	}


