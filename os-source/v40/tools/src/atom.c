#define CUNIX 1
/* #define AMIGA*/

#ifdef CUNIX
#include <stdio.h>
#include <ctype.h>
#ifdef AMIGA
#include <fcntl.h>
#endif
#endif

#ifdef AMIGA
#include "exec/types.h"
#include "exec/memory.h"
#include "libraries/dos.h"
#define toupper(x) ((x)&0x5f)
#define islower(x) ((0x60<(x)) && ((x)<0x7b))
#else
#include <sys/file.h>
#endif

#include "alinkdefs.h"

#define TRUE 1
#define FALSE 0 
#define HandleBreakEnd() HasHunkName = FALSE

char copyright[] = 
  "Copyright 1987  Commodore-Amiga, Inc. All Rights Reserved";

static long filemark = 0;
static int  HasHunkName = FALSE; 
static int  HasUnitName = FALSE; 
static char* BigBuff;
static long* HunkName[5];
static long* UnitName[5];
#ifdef CUNIX
static int   infd = -1 ;
static int   outfd  = -1;
#else
static int   infd = 0;
static int   outfd  = 0;
#endif
static long  CodeMemType = -1;
static long  DataMemType = -1;
static long  BssMemType  = -1;
static int   interactive = FALSE;
static char* outfile = NULL;    


main(argc,argv)
int argc;
char* argv[];
{ 
    if(!argc)exit(0);

    printf("Amiga Object Modifier V1.1\n");
#ifdef CUNIX
    BigBuff = (char*)malloc(1000);
#else
    BigBuff = (char *)AllocMem(1000, MEMF_PUBLIC);
#endif
    if ((argc < 3) || (argc > 6)) {
	printf("Usage: Infile Outfile [-i] [-C [C|D|B]] [-F [C|D|B|]] [-P [C|D|B|]]\n");
	if(*argv[argc-1] != '?') error("Bad Args");
	else tidyup(0);
    }
    if (argc > 3) DecodeArgs(argc, argv);  

#ifdef CUNIX
    infd = open(argv[1],O_RDONLY,0644);
    if (infd == -1) error("Bad infile");

    outfd = open(argv[2],O_WRONLY|O_CREAT|O_TRUNC,0644);
    if (outfd == -1) error("Bad outfile");

#else
    if(!(infd= Open(argv[1], MODE_OLDFILE)))error("Bad infile");
    if(!(outfd= Open(argv[2], MODE_NEWFILE)))error("Bad outfile");
#endif

    outfile = argv[2];
    editfile();

#ifdef CUNIX

#else
    FreeMem(BigBuff, 1000);
#endif
    tidyup(0);
}


tidyup(n)
int n;
{
#ifdef CUNIX
    if (infd >= 0) close(infd);
    if (outfd >=0) close(outfd);
    if (n != 0) unlink(outfile);
#else
    if(infd) Close(infd);
    if(outfd) Close(outfd);
    if(n) DeleteFile(outfile);
#endif
    exit(n);
}


/* tool infile outfile [-i] [-C [C|D|B]] [-F [C|D|B|]] [-P [C|D|B|]] */

DecodeArgs(argc,argv)
int argc;
char* argv[];
{
    int i;
    for(i = 3; i < argc; i++)  
    { 
	if ((argv[i][0] != '-')|| interactive)  error("Bad Args");
	switch (islower((argv[i][1])) ? toupper((argv[i][1])) : argv[i][1])
	{ 
	    case 'I':
		if (argv[i][2] != 0) error("Bad Args");
		interactive = TRUE;
		break;
	    case 'C':
		settypes(argv, i, chipmem);
		break;
	    case 'F':
		settypes(argv, i, fastmem); 
		break;
	    case 'P':
		settypes(argv, i, pubmem);
		break;
	    default :
		error("Bad Args");
		break;
	}
    }
}


settypes(argv,i,memtype)
char* argv[];
int i;
long memtype;
{
    int j ;
    for (j = 2; j <= 4; j++)
    {
	switch (islower((argv[i][j])) ? toupper( (argv[i][j]) ) : argv[i][j])
	{ 
	    case 'C':
		settype2(&CodeMemType, memtype);
		break;
	    case 'D':
		settype2(&DataMemType, memtype); 
		break;
	    case 'B':
		settype2(&BssMemType, memtype);
		break;
	    case 0:
		if (j == 2)
		{   
		    settype2(&CodeMemType, memtype);
		    settype2(&DataMemType, memtype); 
		    settype2(&BssMemType, memtype); 
		}
		return;
		break;
            default:
		error("Bad Args");
	};
    }
}


settype2(type, memtype)
long *type;
long memtype;
{  
    if ((*type) != -1)
	error("Bad Args");
    *type = memtype;
}


editfile()
{ 
    long type;
    long  size;

    if ((type=gettype()) == 0)
	error("empty input"); 
    do switch ((int)(type & typemask))
    { 
	case hunk_break  :
	case hunk_end    :  HandleBreakEnd();	break;
	case hunk_overlay:
	case hunk_header :  WrongObject();	break;
	case hunk_code   :
	case hunk_data   :    
	case hunk_bss    :  ReadCodeDataOrBss(type, TRUE); break;
	case hunk_ext    :  ReadExternals(TRUE); break;
	case hunk_reloc32:
	case hunk_reloc16:
	case hunk_reloc8 :  ReadReloc(TRUE);	break;
	case hunk_name   :  ReadHunkName();	break;
	case hunk_unit   :  ReadUnitName();	break;
	case hunk_symbol :  ReadSymbols(TRUE);	break;
	case hunk_debug  :  ReadDebug();	break;
	default          :  error("Bad Type %8lx",type); break; 
    } while(!((type=gettype())==0));
}


ReadDebug()
{
    long size;
    getlong(&size, 1, TRUE);
    mygetwords(size, TRUE);
}


ReadSymbols(flag)
int flag;
{
    long size;
    long val;

    for(;;)
    { 
	getlong(&size,1,flag);
	if (size == 0) return;
	mygetwords(size,flag);
	getwords(&val,1,flag);
    }
}


ReadHunkName()
{
    long size;
    int i;

    getlong(&size,1,TRUE);
    if (size != 0)
	getwords(HunkName,size,TRUE);
    HunkName[size]=0;
    HasHunkName = TRUE;
}


ReadUnitName()
{
    long size;
    int i;

    HasUnitName = FALSE;
    getlong(&size, 1, TRUE);
    if (size != 0)
    {
	getwords(UnitName,size,TRUE);
	HasUnitName = TRUE;
    }
    UnitName[size] = 0;
}


ReadReloc(flag)
int flag;
{ 
    for(;;)
    {
	long size;
	long hunk;
	getlong(&size, 1, flag);
	if (size == 0) return;
	getwords(&hunk, 1, flag);
	mygetwords(size, flag);
    }
}


ReadCodeDataOrBss(type, flag)
long type;
int flag;
{
    long size;
    long type1 = type & typemask;

    if((flag==TRUE) && interactive) 
    {
	int ch; 
#ifdef CUNIX
	int mark = lseek(infd, 0, 1);
#else
	int mark = Seek(infd, 0, OFFSET_CURRENT);
#endif
	DisplayHunkInfo(type);
	for(;;)
	{   
	    printf("\nDisplay Symbols? [Y/N] ");
	    ch = getchar(); 
            ch = islower(ch) ? toupper(ch) : ch;
	    if (ch == 'Y')
	    {
		DisplaySymbolInfo(type);
		break;
	    }
	    if (ch == 'N') break;
	    if (ch == 'W')
	    {
		interactive = FALSE;
		goto done;
	    }
	    if (ch == 'Q') tidyup(5);
	}

	for(;;)
	{ 
	    while( ch != '\n') ch = getchar();
	    printf("\nMemory Type? [F|C|P] ");
	    ch = getchar(); 
            ch = islower(ch) ? toupper(ch) : ch;
	    switch (ch)
	    {
		case 'F' : type = type1 | fastmem ; goto done; 
		case 'C' : type = type1 | chipmem ; goto done; 
		case 'P' : type = type1 | pubmem  ; goto done;
		case 'Q' : tidyup(5);
		case 'W' : interactive = FALSE ; goto done;
		case 'N' : goto done;
		default : break;
	    }
	    printf("\nPlease enter F for fast\n");
	    printf("             C for Chip\n");
	    printf("             P for Public\n");
	    printf("             Q to  quit\n");
	    printf("             W to  windup\n");
	    printf("             N for Next hunk\n");
	}

done:   while( ch != '\n') ch = getchar();

#ifdef CUNIX
	lseek(infd, mark, 0);
#else
	Seek(infd, mark, OFFSET_BEGINNING);
#endif

#ifdef IBM 
	type = swap(&type);
#endif

#ifdef CUNIX
	write(outfd, &type,4);
#else
	Write(outfd, &type,4);
#endif
    }
    else
    {
	switch ((int)type1)
	{
	    case hunk_code:
		if(CodeMemType != -1) type = type1 | CodeMemType;
		break;
	    case hunk_data:
		if(DataMemType!= -1) type = type1 | DataMemType;
		break;
	    case hunk_bss:
		if(BssMemType  != -1) type = type1 | BssMemType;
		break;
	    default:
		error("Bad type %08lx",type);
	}
	if (flag)
	{
#ifdef IBM 
	    type = swap(&type);
#endif

#ifdef CUNIX
	    write(outfd, &type, 4);
#else
	   Write(outfd,&type,4);
#endif
	}
    }
  
    getlong(&size,1,flag);
    if (!((type & typemask) == hunk_bss) ) mygetwords(size,flag);
}


DisplayHunkInfo(type)
long type;
{
    long memtype = type & memmask;

    type = type & typemask;
    printf("\nUnit Name %s\n",HasUnitName ? (char*)UnitName : "None");
    printf("Hunkname %s\n", HasHunkName ? (char*)HunkName : "None");
    printf("Hunk Type %s\n", (type == hunk_code) ? "CODE"
				  :(type == hunk_data) ? "DATA"
				  :(type == hunk_bss ) ? "BSS"
				  :"Unknown");
    printf("Memory Allocation %s \n", (memtype == chipmem) ? "Chip"
					   :(memtype == fastmem) ? "Fast"
					   :(memtype == pubmem) ? "Public"
					   :"Unknown");
}


ReadExternals(flag)
int flag;
{ 
    for(;;)
    {
	long code;
	long t;
	long size;
	getlong(&code, 1, flag);
	if (code == 0) return;
	size = code & 0x00FFFFFF;
	t = (code >> 24) & 0x000000FF;
	mygetwords(size,flag);
	switch ((int)t)
	{
	    case ext_ref32: 
	    case ext_ref16:
	    case ext_ref8:
		getlong(&size,1,flag);
		mygetwords(size,flag);
		continue;
	    case ext_def:
	    case ext_abs:
	    case ext_res:
		getlong(&size,1,flag);
		continue;
	    default :
		error("ReadExternals");
	}
    }
}


DisplayExternals()
{ 
    for(;;)
    {
	long code;
	long t;
	long size;
	getlong(&code,1,FALSE);
	if (code == 0) return;
	size = code & 0x00FFFFFF;
	t = (code >> 24) & 0x000000FF;
	PrintSymbol(size);
	switch ((int)t)
	{
	    case ext_ref32: 
	    case ext_ref16:
	    case ext_ref8:
		getlong(&size,1,FALSE);
		mygetwords(size,FALSE);
		continue;
	    case ext_def:
	    case ext_abs:
	    case ext_res:
		getlong(&size,1,FALSE);
		continue;
	    default:
		error("ReadExternals");
	}
    }
}


getw(v, n, flag)
char* v;
int   n;
int  flag;
{ 
#if CUNIX
    int res = (read(infd,v,n) == n);
    if (flag && res) write(outfd, v, n);
#else
    int res = (Read(infd,v,n) == n);
    if (flag && res)Write(outfd, v, n);
#endif
    return(res);
}


long gettype()
{ 
    long type; 

#ifdef CUNIX
    int res = read(infd, &type, 4);
#else
    int res = Read(infd, &type, 4);
#endif
    if (res !=4 ) return(0);
    switch ((int)(type & typemask))
    {
	case hunk_code:
	case hunk_data:
	case hunk_bss:
	    break;
	default : 
#ifdef CUNIX
	    write(outfd,&type,4);
#else
	    Write(outfd,&type,4);
#endif
	    break;
    }
#ifdef SUN
    return(type);
#endif

#ifdef AMIGA
    return(type);
#endif

#ifdef IBM
    return swap(&type);
#endif
}


union u {
    long a;
    char b[4];
};
  

long swap(val)
union u *val;
{
    union u temp;
    temp.b[0]=val->b[3];
    temp.b[1]=val->b[2];
    temp.b[2]=val->b[1];
    temp.b[3]=val->b[0];
    return((long)temp.a);
}


getlong(v, n, flag)
char* v;
int n;
int flag;
{ 
    long type = getwords(v,n,flag);
#ifdef SUN
    return(type);
#endif
#ifdef IBM
    return swap(&type);
#endif
}


getwords(v,n,flag)
char* v;
int n;
int flag;
{
    if (!getw(v, n << 2, flag)) error("premature end of file");
}


/* just reads size longwords of data staright through */
/* in 32k chunks */
mygetwords(size,flag)
long size;
int flag;
{
    int nwords; 
    while (size > 0)
    {
	nwords = size > 250 ? 250 : size;
	getwords(BigBuff,nwords,flag);
	size -= nwords;
    }
}


error(s,a)
char* s;
long a;
{
    printf("\nError "); 
    printf(s,a);
    putchar('\n');
    tidyup(20);
}


DisplaySymbolInfo(type)
long type ;
{ 
    long size;
    int  flag = FALSE;
   ReadCodeDataOrBss(type, FALSE);
   getlong(&type, 1, FALSE);
   do
   switch((int)(type & typemask))
   {
	case hunk_reloc32: 
	case hunk_reloc16:
	case hunk_reloc8 :
	    ReadReloc(FALSE);
	    getlong(&type,1,FALSE);
	    break;
	case hunk_ext:
	    DisplayExternals(); 
	    getlong(&type,1,FALSE);
	    break;
	default:
	    break;
    } while(!((type == hunk_symbol)||(type == hunk_end)||
	(type == hunk_debug)));
    if (type != hunk_symbol) return(FALSE); 
    for(;;)
    {
	getlong(&size,1,FALSE);
	if (size == 0) break; 
	flag = TRUE;
	PrintSymbol(size);
	getlong(&size,1,FALSE);
    }

    return flag;
}


PrintSymbol(size)
long size;
{
    int i,j;
    mygetwords(size, FALSE);
    for(i = 0; i <= ((size << 2) - 1); i++)
	putchar((BigBuff[i]==0)?'.':BigBuff[i]);

    putchar('\n');
}


WrongObject()
{ 
    error("This Utility can only be used on files that have NOT\n been passed through ALINK"); 

}
