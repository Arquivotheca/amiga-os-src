;/* lvo.c - Execute me to compile me with SAS C 6.x
SC lvo.c data=near nominc strmer streq nostkchk saveds ign=73
Slink FROM LIB:c.o,lvo.o TO lvo LIBRARY LIB:SC.lib,LIB:Amiga.lib ND NOICONS
quit

36_15 mods: reverse bit order in wedgeline output (match wedge 36.18)
36_16 mods: add optional wedgeline addition (arg following wedgeline)
36_17 mods: remove debugging line accidentally left in
36_18 mods: bump revision to match wedge
37_1  mods: make CONTAINS public, add NameFromLVO functionality
37_2  mods: accept -0xhhhh and -$hhhh for metascope
37_3  mods: add EQUATES option
*/

#include <exec/types.h>
#include <exec/libraries.h>
#include <libraries/dos.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <stdio.h>
#include <string.h>

#ifdef __SASC
void __regargs __chkabort(void) {}      /* Disable SAS CTRL-C checking. */
#else
#ifdef LATTICE
void chkabort(void) {}                  /* Disable LATTICE CTRL-C checking */
#endif
#endif

char *vers = "\0$VER: lvo 37.3";
char *Copyright = 
  "lvo v37.3\nCopyright (c) 1990-1993 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *usage = 
"Usage: lvo library [funcname | LVO=0xhhhh,-0xhhhh,-n | ROMADDRESS=0xhhhhhhhh]\n"

"Options: [CONTAINS] [WEDGELINE [wedgeopts]]\n"
"OR: lvo library EQUATES >library_lib.i  to output _lib.i equates\n"
"Needs FD: assign to where FD's are\n";

BOOL strEqu(UBYTE *p, UBYTE *q);
BOOL strEquN(UBYTE *p, UBYTE *q, int n);
UBYTE toUpper(UBYTE c);
LONG getline(LONG file,UBYTE *buf, ULONG bufmax);
short getshortval(char *s);
LONG getval(char *s);

struct Library *libbase;
struct JumpVector { UWORD JmpInstr; ULONG JmpAddr; };

#define BUFMAX 256
UBYTE linebuf[BUFMAX];

UBYTE basename[40], libname[80], fdname[80], funcname[80], mfuncname[80];
UBYTE efuncname[80];

UBYTE *wedgefmt="run wedge %s 0x%04lx 0x%04lx 0x%04lx opt r %s \"c=%s\"\n";
UBYTE *wedgeopts = NULL;

LONG intoffs = 0, mintoffs = 0;


void main(int argc, char **argv)
{
LONG file = NULL, rlen;
ULONG matchaddr = 0L, closeaddr = 0L, thisaddr;
UWORD wregs, wptrs;
short askshort = 0;
int askint, k, k0, j, l;
struct JumpVector *jv;
BOOL Found=FALSE, IsResource=FALSE, All=FALSE, WedgeLine=FALSE;
BOOL Contains=FALSE, NameFromLVO=FALSE, Closer=FALSE, NameFromAddr=FALSE;
BOOL Equates=FALSE, Public=TRUE;
UBYTE c, rnum;

    if((argc < 2)||((argc>=2)&&(argv[1][0]=='?')))
	{ 
	printf("%s\n%s\n",Copyright,usage); 
	exit(0L);
	}

    strcpy(basename,argv[1]);
    for(k=0; basename[k] && basename[k]!= '.'; k++);
    basename[k] = '\0';
    k0 = k;

    strcpy(libname,basename);
    strcpy(&libname[k],".library");

    strcpy(fdname,"FD:");
    strcpy(&fdname[3],basename);
    strcpy(&fdname[k+3],"_lib.fd");

    funcname[0] = '\0';

    for(j=2; j<argc; j++)
	{
	if(WedgeLine) wedgeopts=argv[j];
	else if(strEqu(argv[j],"wedgeline")) WedgeLine = TRUE;
	else if(strEqu(argv[j],"contains")) Contains = TRUE;
	else if(strEqu(argv[j],"equates"))  Equates = TRUE;
	else if(!strnicmp(argv[j],"lvo",3))
	    {
	    askshort = getshortval(&argv[j][4]);
	    askint = askshort;
	    NameFromLVO = TRUE;
	    }
	else if(!strnicmp(argv[j],"romaddress",10))
	    {
	    matchaddr = getval(&argv[j][11]);
	    NameFromAddr = TRUE;
	    }
	else
	    {
    	    strcpy(funcname,argv[j]);
    	    k = strlen(funcname);
    	    if(funcname[k-1] == ')') funcname[k-1] = '\0';
    	    else 
	    	{
	    	funcname[k] = '(';
	    	funcname[k+1] = '\0';
	    	}
	    }
	}

    if((!funcname[0])&&(!NameFromLVO)&&(!NameFromAddr)) All = TRUE;

    if(NameFromAddr)	WedgeLine = FALSE;

    if(!(libbase = OpenLibrary(libname,0L)))
	{
	strcpy(&libname[k0],".resource");
	if(libbase = (struct Library *)OpenResource(libname)) IsResource = TRUE;
	else libname[k0] = '\0';
	}

    if(!(file = Open(fdname,MODE_OLDFILE)))
	{
	printf("Can't open FD file %s\n",fdname);
	if((libbase)&&(!IsResource)) CloseLibrary(libbase);
	exit(RETURN_FAIL);
	}


    rlen = 1;
    intoffs = 0;

    while(((rlen = getline(file,linebuf,BUFMAX)) > 0)
		&&(!(SetSignal(0,0)&SIGBREAKF_CTRL_C)))
	{
	if(linebuf[0] == '\0') 		continue;
	else if(linebuf[0] == '*')	continue;
	else if(strEquN(linebuf,"##",2))
	    {
	    if(strEquN(linebuf,"##bias",6))  intoffs = 0 - atoi(&linebuf[7]);
	    else if(strEquN(linebuf,"##public",6))   Public=TRUE;
	    else if(strEquN(linebuf,"##private",6))  Public=FALSE;
	    else if(strEquN(linebuf,"##end",6))   break;
	    }
	else if(linebuf[0])
	    {
	    if(!All)
		{
		/* If passed name, check for name match */
		if((!NameFromLVO)&&(!NameFromAddr)&&
			(strEquN(linebuf,funcname,strlen(funcname))))
		    {
		    Found = TRUE;
		    }
		/* If passed LVO, check for LVO match */
		else if((NameFromLVO)&&(intoffs == askint))
		    {
		    Found = TRUE;
		    }
		/* If passed Rom Address, check for closeness */
		else if((NameFromAddr)&&(libbase))
		    {
		    Closer=FALSE;
		    jv = (struct JumpVector *)(((LONG)libbase) + intoffs);
		    thisaddr = jv->JmpAddr;
		    if((thisaddr <= matchaddr)&&(thisaddr > closeaddr))
			{
			closeaddr = thisaddr;
			mintoffs  = intoffs;
			Closer = TRUE;
			}
		    }
		}

	    /* Make funcname if we didn't have it to start with */		
	    if((All) || ((NameFromAddr)&&(Closer)) || ((NameFromLVO)&&(Found)))
		{
		for(k=0; linebuf[k] && (linebuf[k]!='('); k++);
		j=k+1;
		c = linebuf[j];
		linebuf[j] = '\0';
		strcpy(funcname,linebuf);
		linebuf[j] = c;
		if(Closer)
		    {
		    strcpy(mfuncname,funcname);
		    }
		}

	    if((All)||(Found))
		{
		if(WedgeLine)
		    {
		    wregs = wptrs = 0x8000;  /* show a7 */
		    for(k=0; linebuf[k] && (linebuf[k] != ')'); k++);
		    for(k=k+2; linebuf[k] && (linebuf[k] != ')'); k++)
			{
			c = linebuf[k] | 0x20;
			if(c=='d')
			    {
			    k++;
			    rnum = linebuf[k] & 0x0F;
			    wregs = wregs | (0x0001 << rnum);
			    }
			else if(c=='a')
			    {
			    k++;
			    rnum = linebuf[k] & 0x0F;
			    wregs = wregs | (0x0100 << rnum);
			    wptrs = wptrs | (0x0100 << rnum);
			    }
			}
		    if(!wedgeopts) wedgeopts=" ";
		    printf(wedgefmt,basename,intoffs&0xFFFF,wregs,wptrs,
				wedgeopts,linebuf);
		    }
		else if(Equates)
		    {
		    if(Public)
			{
			strcpy(efuncname,funcname);
		    	l=strlen(funcname);
		    	if(l>1) efuncname[l-1] = '\0';
    		    	printf("_LVO%-24s\tEQU\t%ld\n", efuncname, (WORD)intoffs);
			}
		    }
		else
		    {
    		    printf("%s  LVO $%04lx %ld  %s)\n", 
	  		libname,
			(UWORD)(intoffs&0xFFFF), (WORD)intoffs,
			funcname);

		    if(!All) printf("%s\n",linebuf);

		    if((libbase)&&(Contains)) 
		    	{
		    	jv = (struct JumpVector *)(((LONG)libbase) + intoffs);
		    	printf("  (libbase=$%lx) LVO on this particular system contains $%04lx $%08lx\n",
				libbase, jv->JmpInstr, jv->JmpAddr);
		    	}
		    }
		}
	    if((Found)&&(!All)) break;
	    intoffs -= 6;
	    }
	}


    if((!All)&&(!Found))
	{
	if(NameFromLVO) printf("Can't find %s function at %ld (0x%04lx)\n",
		libname, askint, (UWORD)askshort);
	else if(NameFromAddr)
	    {
	    if(mintoffs)
		{
		printf("Closest to $%lx without going over:\n",matchaddr);
    		printf("%s  LVO $%04lx %ld  %s) jumps to $%lx on this system\n", 
	  		libname,
			(UWORD)(mintoffs&0xFFFF), (WORD)mintoffs,
			mfuncname, closeaddr);
		}
	    else printf("No close match found\n");
	    }
	else printf("Can't find %s function %s\n", libname, funcname);
	}
    Close(file);
    if((libbase)&&(!IsResource)) CloseLibrary(libbase);
    exit(RETURN_OK);
}


LONG getline(LONG file,UBYTE *buf, ULONG bufmax)
{
LONG k = 0;
LONG rlen = 1;

while(((rlen = Read(file,&buf[k],1)) > 0)&&(k < bufmax-1))
    {
    if(buf[k] == '\n')
	{ 
	buf[k] = '\0'; 
/*
	printf("intoffs was %ld, k=%ld line=%s\n",intoffs, k,buf);
*/
	return(k);
	}
    else k++;
    }
return(-1L); 
}

int atoi( char *s )
   {
   int num = 0;
   int neg = 0;

   if( *s == '+' ) s++;
   else if( *s == '-' ) {
       neg = 1;
       s++;
   }

   while( *s >= '0' && *s <= '9' ) {
       num = num * 10 + *s++ - '0';
   }

   if( neg ) return( - num );
   return( num );
   }


BOOL strEqu(UBYTE *p, UBYTE *q)
   { 
   while(toUpper(*p) == toUpper(*q))
      {
      if (*(p++) == 0)  return(TRUE);
      ++q; 
      }
   return(FALSE);
   } 

BOOL strEquN(UBYTE *p, UBYTE *q, int n)
   { 
   int k=0;

   while((k<n)&&(toUpper(*p) == toUpper(*q)))
      {
      ++p;
      ++q;
      ++k;
      }
   if(k==n) return(TRUE);
   return(FALSE);
   } 

UBYTE toUpper(UBYTE c)
   {
   UBYTE u = c;
   if(((u>='a')&&(u<='z'))||((u>=0xe0)&&(u<=0xfe))) u = u & 0xDF;
   return(u);
   }


short getshortval(char *s)
   {
   short svalue;
   int value, count;

   /* negative hex numbers for Metascope */
   if((s[0]=='-')&&((s[2]|0x20)=='x'))
	{
        count = stch_i(&s[3],&value);				/* -0xhhhh */
	value = 0 - value;
	}
   else if((s[0]=='-')&&(s[1]=='$'))
	{
        count = stch_i(&s[2],&value);				/* -$hhhh  */
	value = 0 - value;
	}

   else if((s[1]|0x20) == 'x')  count = stch_i(&s[2],&value);	/* 0xhhhh  */
   else if((s[0]|0x20) == '$')  count = stch_i(&s[1],&value);	/* $hhhh   */
   else				count = stcd_i(&s[0],&value);	/* decimal */
   svalue = value;
   return(svalue);
   }


LONG getval(char *s)
   {
   LONG value, count;

   if((s[1]|0x20) == 'x')  	count = stch_i(&s[2],&value);	/* 0xhhhhhhhh  */
   else if(s[0] == '$')		count = stch_i(&s[1],&value);	/* $hhhhhhhh   */
   else				count = stcd_i(&s[0],&value);	/* decimal     */ 
   return(value);
   }

