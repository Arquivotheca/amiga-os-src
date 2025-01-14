
/*
 *  ENVIROMENT.C
 *
 *  bool = GetDEnv(name,value,max)
 *  bool = SetDEnv(name, str)    (0=failure, 1=success)
 *
 */

#include "exec/memory.h"
#include "functions.h"
#include "xfunctions.h"
#include "string.h"

long GetDEnv(char *name,char *value,long max)
{
    short		nlen = strlen(name) + 5;
    char		*ptr = AllocMem(nlen, MEMF_PUBLIC);
    BPTR		fh;
	long 		len,result = 0;

    if (ptr)
	{	strcpy(ptr,"ENV:");
		strcat(ptr,name);
		if (fh = Open(ptr, 1005))
		{	len = (Seek(fh, 0L, 1), Seek(fh, 0L, -1));
			if (len <= max && len >= 0)
			{	if (Read(fh, value, len) == len)
				{	value[len] = 0; result = 1; }
	    	}
	    	Close(fh);
		}
		FreeMem(ptr, nlen);
    }
    return result;
}

long SetDEnv(char *name,char *str)
{
    short 		nlen = strlen(name) + 5;
    short 		slen = strlen(str);
    long 		res = 0;
    char 		*ptr = AllocMem(nlen, MEMF_PUBLIC);
    BPTR		fh;

    if (ptr)
	{	strcpy(ptr, "ENV:");
		strcat(ptr, name);
		if (fh = Open(ptr, 1006))
		{	if (Write(fh, str, slen) == slen) res = 1;
	    	Close(fh);
		}
		FreeMem(ptr, nlen);
    }

    return res;
}
