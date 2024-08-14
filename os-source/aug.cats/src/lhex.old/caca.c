# include "lharc.h"
# include "args.h"
# include "string.h"

LONG    myFGetC(BPTR f)
{
        if (S_T_S)
                return _myfgetc(f);
        else
                return FGetC(f);
}
LONG    myFPutC(BPTR f, LONG c)
{
        if (S_T_S)
                return _myfputc(f, c);
        else
                return FPutC(f, c);
}
STRPTR  myFGets(BPTR f, STRPTR b, ULONG l)
{
        if (S_T_S)
                return _myfgets(f, b, l);
        else
                return FGets(f, b, l);
}
LONG   myFRead(BPTR f, STRPTR b, ULONG l, ULONG n)
{
        if (S_T_S)
                return _myfread(f, b, l, n);
        else
                return FRead(f, b, l, n);
}
LONG   myFWrite(BPTR f, STRPTR b, ULONG l, ULONG n)
{
        if (S_T_S)
                return _myfwrite(f, b, l, n);
        else
                return FWrite(f, b, l, n);
}
LONG    myFlush(BPTR f)
{
        if (S_T_S)
                return 1;
        else
                return Flush(f);
}

 LONG _myfgetc(BPTR fh)
{
	unsigned char	buf[2];
	
	if (Read(fh, buf, 1) != 1)
		return -1;
	return buf[0];
}

 LONG _myfputc(BPTR fh, LONG c)
{
	char	buf[2];
	
	buf[0] = c;
	buf[1] = 0;
	
	if (Write(fh, buf, 1) != 1)
		return -1;
	return c;
}

 STRPTR _myfgets(BPTR fh, STRPTR buffer, ULONG len)
{
	int	i = 0;
	char	inbuf = 0;

	if (Read(fh, &inbuf, 1) != 1)
		return NULL;
	
	do
		buffer[i++] = inbuf;
	while (i < (len-1) && inbuf != '\n' && Read(fh, &inbuf, 1) == 1);
	
	buffer[i] = 0;
	
	return buffer;
}

 LONG _myfread(BPTR fh, STRPTR buffer, ULONG len, ULONG num)
{
	int	i;
	int	a, b;
	
	if (len > num)
	{
	    a = len;
	    b = num;
	}
	else
	{
	    a = num;
	    b = len;
	}
	
	for (i = 0; i < b; i++)
	{
	    if (Read(fh, buffer + (i * a), a) != a)
	    	break;
	}
	return(i);
}

 LONG _myfwrite(BPTR fh, STRPTR buffer, ULONG len, ULONG num)
{
	int	i;
	int	a, b;
	
	if (len > num)
	{
	    a = len;
	    b = num;
	}
	else
	{
	    a = num;
	    b = len;
	}
	
	for (i = 0; i < b; i++)
	{
	    if (Write(fh, buffer + (i * a), a) != a)
	    	break;
	}

	return(i);
}

extern char * Keywords[];

struct RDArgs *
_myparseargs(int argc, char ** argv, LONG Results[] )
{
    int		i;
    int		k;
    char	*p;
    char	*p2;
    int		want = OPT_ARCHIVE;
    char	**files = NULL;
    long	n;
    
    files = Init_List(files);
    
    for (i = 1; i < argc; i++)
    {
	for (k = 0; k < OPT_COUNT; k++)
	    if (strnicmp(argv[i], Keywords[k], strlen(Keywords[k])) == 0)
		break;
			
	/*
	** Got a match
	**	if k is between Files and Directory
	**		It's a switch,
	**			make sure len(arg) == len(keyword)
	**			set flag
	*/
	if (k > OPT_FILES && k < OPT_DIRECTORY)
	{
	    if (strlen(Keywords[k]) != strlen(argv[i]))
	    {
		Free_List(files);
		return NULL;
	    }
	    else
		Result[k] = 1;
	}
	/*
	**	else if k is DIRECTORY
	**		its the directory keyword.
	**		expect either '=' followed by a string
	**		or grab the next arg & bump i by 1
	*/
	else if (k == OPT_DIRECTORY)
	{
	    p = strchr(argv[i], '=');
	    if (p)
	    {
		p++;
	    }
	    else
	    {
		p = NULL;
		want = k;
	    }
	    Result[k] = (LONG)p;
	}
	/*
	**	else if k is HEADER
	**		its the header keyword.
	**		expect either '=' followed by a number
	**		or grab the next arg & bump i by 1
	*/
	else if (k == OPT_DIRECTORY)
	{
	    p = strchr(argv[i], '=');
	    if (p)
	    {
		p++;
		n = strtol(p, &p2, 0);
		if (p == p2 || *p2 != '\0')
		{
		    /*
		    **	Error!
		    **
		    **	Either p does not point to a number, (e.g., p = "two")
		    **	or the number has trailing garbage (e.g., p = "2junk")
		    */
		    Free_List(files);
		    return NULL;
		}
		p = xmalloc(sizeof(long));
		*((long *) p) = n;
	    }
	    else
	    {
		p = NULL;
		want = k;
	    }
	    Result[k] = (LONG)p;
	}
#if	defined(IDEBUG)
	/*
	**	else if k is DEBUG
	**		its the directory keyword.
	**		expect either '=' followed by a string
	**		or grab the next arg & bump i by 1
	*/
	else if (k == OPT_DEBUG)
	{
	    Result[k] = 1;
	}
#endif
	/*
	**	else if (k == 0)
	**		its the archive keyword.
	**		expect either '=' followed by a string
	**		or grab the next arg & bump i by 1
	*/
	else if (k == OPT_ARCHIVE)
	{
	    p = strchr(argv[i], '=');
	    if (p)
	    {
		p++;
		if (!*p)
		    p = NULL;
	    }
	    else
	    {
		p = NULL;
	    }
				
	    if (Result[k] == NULL)
		Result[k] = (LONG)p;
	    else
	    {
		Free_List(files);
		return NULL;
	    }
	}
	/*
	**	else if (k == 1)
	**		its the file keyword.
	**		expect either '=' followed by a string
	**		or grab any *unrecognized* args & bumping i
	*/
	else if (k == OPT_FILES)
	{
	    p = strchr(argv[i], '=');
	    if (p)
	    {
		p++;
		if (*p)
		    files = Add_List(files, p);
	    }
	    else
	    {
		want = k;
	    }
	}
	/*
	**	It's not a keyword, so it must be a file or directory name
	*/
	else
	{
	    switch (want)
	    {
#ifndef LHEXTRACT
	      case OPT_HEADER:
		p = argv[i];
		n = strtol(p, &p, 0);
		if (p == p2 || *p2 != '\0')
		{
		    /*
		    **	Error!
		    **
		    **	Either p does not point to a number, (e.g., p = "two")
		    **	or the number has trailing garbage (e.g., p = "2junk")
		    */
		    Free_List(files);
		    return NULL;
		}
		p = xmalloc(sizeof(LONG));
		*((LONG *) p) = n;
		Result[want] = (LONG)p;
		break;
#endif						
	      case OPT_ARCHIVE:
	      case OPT_DIRECTORY:
		Result[want] = (LONG)argv[i];
		break;
						
	      case OPT_FILES:
		files = Add_List(files, argv[i]);
		break;
	    }
				
	    if (Result[OPT_ARCHIVE])
		want = OPT_FILES;
	    else
		want = OPT_ARCHIVE;
	}
    }

    if (Length_List(files) > 0)
	Results[OPT_FILES] = (LONG)files;
    return (struct RDArgs *)files;
}


void
_myfreeargs(struct RDArgs * ralist)
{
    char	**files = (char **)ralist;

    Free_List(files);
}

