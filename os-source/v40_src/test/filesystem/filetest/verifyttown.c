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

char **TTfilenames, *TTfill;
ULONG TTdirs,TTfiles,TTfillsize,TTfilltype,TTfilesize,TTprotect,TTown;
BOOL TTsetprotect,TTsetown;

char TestDname[64];



VerifyTestTree(path,root,dirs,files,depth,filenames,fill,fillsize,filltype,filesize,setprotect,protect,setown,own)
	char 	*path,*root, *fill, **filenames;
	ULONG	files, dirs,depth,fillsize,filltype,filesize;
	{
	ULONG g_error;

	long pathlength = strlen(path);
	char *dirpath;
	struct FileLock *Dlock;

	TTfilenames = filenames;
	TTfill = fill;
	TTdirs = dirs;
	TTfiles = files;
	TTfillsize = fillsize;
	TTfilesize = filesize;
	TTfilltype = filltype;
	TTsetprotect = setprotect;
	TTprotect = protect;
	TTsetown = setown;
	TTown = own


	if(!(dirpath = dosalloc(pathlength + strlen(root) + 2)))
		return(OTHERERROR);
	strcpy(dirpath,path);
	if((pathlength) && ((path[pathlength - 1]) != ':'))
			strcat(dirpath,"/");
	strcat(dirpath,root);


	g_error = test_grow(dirpath,depth);

	dosfree(dirpath);
	return(g_error);

	}

test_grow(dpath,depth)
	char *dpath;
	ULONG depth;
	{
	int dir_number;
	char *gpath;


	depth--;


	for(dir_number = 0;dir_number < TTdirs; dir_number++)
		{
		int g_error = OK;

		sprintf(&TestDname[0],"D%d", dir_number);
		if((VerifyTestDir(dpath,&TestDname[0],TTfiles,TTfilenames,TTfill,TTfillsize,TTfilltype,TTfilesize,TTsetprotect,TTprotect,TTsetown ,TTown)))
			return(OTHERERROR);

		{
		long plength = strlen(dpath);

		if(!(gpath = dosalloc(plength + strlen(&TestDname[0]) + 2)))
			return(OTHERERROR);
		strcpy(gpath,dpath);
		if((plength) && ((gpath[plength - 1]) != ':'))
			strcat(gpath,"/");
		strcat(gpath,&TestDname[0]);
		}


		if(depth)
			g_error = test_grow(gpath,depth);

		dosfree(gpath);
		if(g_error != OK)
			return(g_error);


		}
	return(OK);
	}



