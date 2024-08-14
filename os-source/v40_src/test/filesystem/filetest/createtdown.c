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

CreateTestDir(path,name,files,filenames,fill,fillsize,filltype,filesize,setprotect,protect,setown,own)
	char 	*path, *name, *fill, **filenames;
	ULONG	files, fillsize,filltype,protect,filesize,own;
	BOOL 	setprotect,setown;
	{
	ULONG num_files;
	char Fname[50];
	long pathlength = strlen(path);
	char *completedpath;
	struct FileLock *Dlock;
	int ferror;



	if(filenames)
		{
		return(OTHERERROR); /* not implimented yet */
		}
	if(!(completedpath = dosalloc(pathlength + strlen(name) + 2)))
		{
		printf("Memory allocation  failure \n");
		return(OTHERERROR);
		}

	strcpy(completedpath,path);
	if((pathlength) && ((path[pathlength - 1]) != ':'))
			strcat(completedpath,"/");
	strcat(completedpath,name);

	if(!(Dlock = CreateDir(completedpath)))
		{
		dosfree(completedpath);
		printf("Could not Create Test Directory\n");
		return(OTHERERROR);
		}
	UnLock(Dlock);

	for(num_files =0;num_files < files;num_files++)
		{
		sprintf(&Fname[0],"F%d", num_files);
		if(ferror = (CreateTestFile(completedpath,&Fname[0],fill,fillsize,filltype,filesize,setprotect,protect,setown,own)))
			{
			dosfree(completedpath);
			printf("Could not create test file \n");
			return(ferror);
			}
		}
	dosfree(completedpath);
	return(OK);
	}


