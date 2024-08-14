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

char **Tfilenames, *Tfill;
ULONG Tdirs,Tfiles,Tfillsize,Tfilltype,Tfilesize,Tprotect,Town;
BOOL Tsetprotect,Tsetown;
char Dname[64];



CreateTestTree(path,root,dirs,files,depth,filenames,fill,fillsize,filltype,filesize,setprotect,protect,setown,own)
	char 	*path,*root, *fill, **filenames;
	ULONG	files, dirs,depth,fillsize,filltype,protect,filesize,own;
	BOOL 	setprotect,setown;
	{
	ULONG g_error;

	long pathlength = strlen(path);
	char *dirpath;
	struct FileLock *Dlock;
/* printf(" filesize %d \n",filesize); */
	Tfilenames = filenames;
	Tfill = fill;
	Tdirs = dirs;
	Tfiles = files;
	Tfillsize = fillsize;
	Tfilesize = filesize;
	Tfilltype = filltype;
	Tsetprotect = setprotect;
	Tprotect = protect;
	Tsetown = setown;
	Town = own

/* printf("start .... \n"); */

	if(!(dirpath = dosalloc(pathlength + strlen(root) + 2)))
		{
		return(OTHERERROR);
		}
	strcpy(dirpath,path);
	if((pathlength) && ((path[pathlength - 1]) != ':'))
			strcat(dirpath,"/");
	strcat(dirpath,root);

	if(!(Dlock = CreateDir(dirpath)))
		{
		dosfree(dirpath);
		printf("Could not create Test Directory\n");
		return(OTHERERROR);
		}
	UnLock(Dlock);

	g_error = grow(dirpath,depth);

	dosfree(dirpath);
	return(g_error);

	}

grow(dpath,depth)
	char *dpath;
	ULONG depth;
	{
	int dir_number;
	char *gpath;
/* printf("grow\n"); */


	depth--;


	for(dir_number = 0;dir_number < Tdirs; dir_number++)
		{
		int g_error = 0;



/* printf("CTD %d\n",Tfilesize); */





		sprintf(&Dname[0],"D%d", dir_number);


		if(g_error = (CreateTestDir(dpath,&Dname[0],Tfiles,Tfilenames,Tfill,Tfillsize,Tfilltype,Tfilesize,Tsetprotect,Tprotect,Tsetown ,Town)))
			{
			printf("Could not create Test Directory \n");
			return(g_error);
			}



		{
		long plength = strlen(dpath);

		if(!(gpath = dosalloc(plength + strlen(&Dname[0]) + 2)))
			{
			/* printf("alloc failure gpath\n"); */
			return(OTHERERROR);
			}
		strcpy(gpath,dpath);
		if((plength) && ((gpath[plength - 1]) != ':'))
			strcat(gpath,"/");
		strcat(gpath,&Dname[0]);
		}


		if(depth)
			g_error = grow(gpath,depth);

		dosfree(gpath);
		if(g_error)
			return(g_error);


		}
	/* printf("return OK\n"); */
	return(OK);
	}
