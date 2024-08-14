;/* doslist.c - A. Finkel - Execute me to compile me with Lattice 5.04
LC -b1 -cfistq -v -y -j73 -iV36:include doslist.c
Blink FROM LIB:c.o,doslist.o TO doslist LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit
*/

#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/filehandler.h>
#include <resources/filesysres.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef LATTICE
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif

char *vers = "\0$VER: doslist 37.1";
char *Copyright = 
  "doslist v37.1\nCopyright (c) 1990 Commodore-Amiga, Inc.  All Rights Reserved";

#define MYDEBUG 0
#define KDEBUG 1

#if KDEBUG
void kprintf (UBYTE *fmt,...);
#define bug kprintf
#elif DDEBUG
#define bug dprintf
#else	/* else changes all bug's to printf's */
#define bug printf
#endif

#if MYDEBUG
#define DEBTIME 50
#define D(x) (x); if(DEBTIME>0) Delay(DEBTIME)
#else
#define D(x) ;
#endif

void bp(UBYTE *);

#define QTOUPPER(c)      ((c)>='a'&&(c)<='z'?(c)-'a'+'A':(c))
#define MAXCHARS	12000

#undef  BADDR
#define BADDR(x)        ((APTR)((ULONG)x << 2))

UBYTE *buffer,*point, *base;	/* for the output buffer */
int len;

extern struct DosLibrary *DOSBase;

void main(int argc,char **argv)
{
ULONG type;
BPTR seglist;
struct MsgPort *task;
UBYTE *name, *handler, *devicename;
ULONG  deviceunit,deviceflags;
int i;
BOOL flags[4];		/* used for argument parsing */

struct  DosLibrary   *dosLibrary;
struct  RootNode     *RootNode;
struct  DosInfo      *dosInfo;
struct  DosList      *DevInfo;
struct  FileSysStartupMsg *startup;
struct  DosEnvec *env;
UBYTE b[255];

    if(*argv[argc-1]=='?') {
	printf("Usage: %ls DEVS|VOLS|DIRS\n",argv[0]);
	exit(0);
    }

    if(argc == 1) {
	flags[0]=TRUE;
	flags[1]=TRUE;
	flags[2]=TRUE;
    }
    else {
	flags[0]=0;
	flags[1]=0;
	flags[2]=0;

        for(i=1; i<argc; i++) {
	    if (!(strcmpi("devs",argv[i])))flags[0]=TRUE;
	    if (!(strcmpi("vols",argv[i])))flags[1]=TRUE;
	    if (!(strcmpi("dirs",argv[i])))flags[2]=TRUE;
	}
    }
dosLibrary = (struct DosLibrary *)DOSBase;
RootNode = (struct RootNode *)dosLibrary->dl_Root;
dosInfo = (struct DosInfo *)BADDR(RootNode->rn_Info);
DevInfo = (struct DosList *)BADDR(dosInfo->di_DevInfo);

D(bug("did args\n"));

if(!(buffer=(UBYTE *)AllocMem(MAXCHARS,MEMF_PUBLIC|MEMF_CLEAR)))exit(20);
point = buffer;

Forbid();
/* walk the device list */
while ((DevInfo != NULL) && (!(SetSignal(0,0) & SIGBREAKF_CTRL_C))) { 

    type = DevInfo->dol_Type;
    task = DevInfo->dol_Task;
    name = (char *)BADDR(DevInfo->dol_Name)+1;

    D(bug("in loop, name = %s\n",name));

    if((type == DLT_DEVICE) && (flags[0])) {
        sprintf(b,"Device: %ls at %lx, task is %lx ",name,DevInfo,task);
	bp(b);
	if(task)bp("(resident)\n");
	else {
	    bp(" (not resident)\n");

	    seglist = DevInfo->dol_misc.dol_handler.dol_SegList;
	    if(seglist==0) {
		sprintf(b,"   code not in memory\n");
		bp(b);
	    }
	    else {
		sprintf(b,"  code is at %lx\n",BADDR(seglist));
		bp(b);
	    }

	    handler=(char *)BADDR(DevInfo->dol_misc.dol_handler.dol_Handler)+1;
	    if(handler != (UBYTE *)1) {
		sprintf(b,"  handler is  %s\n",handler);
		bp(b);
	    }
	    sprintf(b,"  stacksize is %ld\n",
		DevInfo->dol_misc.dol_handler.dol_StackSize);
	    bp(b);
	    sprintf(b,"  priority is %ld\n",
		DevInfo->dol_misc.dol_handler.dol_Priority);
	    bp(b);
	    sprintf(b,"  startup is $%lx\n",
		DevInfo->dol_misc.dol_handler.dol_Startup);
	    bp(b);

	    sprintf(b,"  globvec is  $%lx\n",
		BADDR(DevInfo->dol_misc.dol_handler.dol_GlobVec));
	    bp(b);
	}

	if(DevInfo->dol_misc.dol_handler.dol_Startup > 2) {
	    startup=(struct FileSysStartupMsg *)
		BADDR(DevInfo->dol_misc.dol_handler.dol_Startup);
	    devicename  = (char *)BADDR(startup->fssm_Device)+1;
	    env  = (struct DosEnvec *)(BADDR(startup->fssm_Environ));
	    deviceunit  = startup->fssm_Unit;
	    deviceflags = startup->fssm_Flags;
	    sprintf(b,"  Environment vector size %ld\n",(ULONG)env->de_TableSize);
	    bp(b);
	    sprintf(b,"|     Device name is %s, unit is %ld\n",
		devicename,deviceunit);
	    bp(b);
	    sprintf(b,"|     Surfaces = %ld, BlocksPerTrack = %ld\n",
		env->de_Surfaces,env->de_BlocksPerTrack);
	    bp(b);
	    sprintf(b,"|     SizeBlock = %ld, SecOrg = %ld, SecPerBlock = %ld\n",
		env->de_SizeBlock,env->de_SecOrg,env->de_SectorPerBlock);
	    bp(b);
	    sprintf(b,"|     PreAlloc = %ld, Interleave = %ld\n",
		env->de_PreAlloc,env->de_Interleave);
	    bp(b);
	    sprintf(b,"|     Reserved = %ld, LowCyl=%ld, HighCyl=%ld\n",
		env->de_Reserved,env->de_LowCyl,env->de_HighCyl);
	    bp(b);
	    sprintf(b,"|     NumBuffers = %ld, BufMemType = %ld\n",
		env->de_NumBuffers,env->de_BufMemType);
	    bp(b);
	    if(env->de_TableSize > 12) {
	        sprintf(b,"|     MaxTransfer = %ld, Mask = %lx, BootPri = %ld\n",
			env->de_MaxTransfer,env->de_Mask, env->de_BootPri);
		bp(b);
	        sprintf(b,"|     DosType is %lx\n",env->de_DosType);
		bp(b);
	    }
	}
	else bp("  No environment vector\n");
    }
    else if ((type == DLT_VOLUME) & (flags[1])) {
        sprintf(b,"Volume: %ls at %lx, File task is %lx ",name,DevInfo,task);
	bp(b);
	if (task)bp("[Mounted]\n");
	else bp("[Not Mounted]\n");
	sprintf(b,"  LockList is %lx\n",
		BADDR(DevInfo->dol_misc.dol_volume.dol_LockList));
	bp(b);
        sprintf(b,"  DiskType is %lx\n",
		DevInfo->dol_misc.dol_volume.dol_DiskType);
	bp(b);
    }
    else if ((type == DLT_DIRECTORY) && (flags[2])) {
        sprintf(b,"Directory: %ls at %lx, FileSystem task is %lx, lock is %lx\n",name,DevInfo,task,DevInfo->dol_Lock);
	bp(b);
    }
    else if (type == DLT_LATE) {
        sprintf(b,"Late Assign: %ls at %lx, FileSystem task is %lx, lock is %lx\n",name,DevInfo,task,DevInfo->dol_Lock);
	bp(b);
    }
    else if (type == DLT_NONBINDING) {
        sprintf(b,"Nonbinding Assign: %ls at %lx, FileSystem task is %lx, lock is %lx\n",name,DevInfo,task,DevInfo->dol_Lock);
	bp(b);
    }

    else {
        sprintf(b,"UNRECOGNIZED LIST ENTRY: type %ld, name %ls, at %lx, FileSystem task is %lx, lock is %lx\n",
		type, name,DevInfo,task,DevInfo->dol_Lock);
	bp(b);
    }

    DevInfo = (struct DosList *)BADDR(DevInfo->dol_Next);
}

Permit();
    len = point-buffer;
    point = buffer;
    base = buffer;
    for(i=0; i<len && (!(SetSignal(0,0)&SIGBREAKF_CTRL_C)); i++) {
	if((UBYTE)*point++ == 0) {
	    printf("%s",base);
	    base = point;
	}
    }

FreeMem(buffer,MAXCHARS);
exit(0);
}

/*
strcmpi(str1, str2)
char *str1,*str2;
{
        UBYTE *astr =str1;
	UBYTE *bstr =str2;
	UBYTE c;

    while ( (c = QTOUPPER(*astr)) && (c == QTOUPPER(*bstr)))
    astr++, bstr++;
    if( ! c ) return ( 0 );
    if( c < *bstr ) return( -1 );
    return( 1 );
}

strlen( str )
UBYTE *str;
{
    UBYTE *pt ;

    for ( pt=str ; *pt != '\0' ; pt++ );
    return( pt-str );
}
*/

void bp(s)
UBYTE *s;
{
int i;

i=strlen(s);
if(i>255) s="UH-OH - a string was too long - we may have trashed ourselves\n";
if((point+i) < (buffer+MAXCHARS))sprintf(point,s);
point += (i+1);
}

