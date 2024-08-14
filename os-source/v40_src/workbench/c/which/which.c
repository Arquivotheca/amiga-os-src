; /*
lc -d -j73 -O -o/obj/Which.o -i/include -v -csf Which
blink /obj/Which.o to /bin/Which sc sd nd
protect /bin/Which +p
quit
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .  | || the authors:                             BBS: (919) 481-6436    */
/* | o  | ||   John Toebes     John Mainwaring    Jim Cooper                 */
/* |  . |//    Bruce Drake     Gordon Keener      Dave Baker                 */
/* ======      Doug Walker                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*---------------------------------------------------------------------------*/
/* Command: Which                                                          */
/* Author:  Andy Finkel                                                           */
/* Change History:                                                           */
/*  Date    Person        Action                                             */
/* -------  ------------- -----------------                                  */
/* 19MAR89  Andy Finkel
/* Notes:                                                                    */
/*---------------------------------------------------------------------------*/
#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/notify.h>
#include <dos/filehandler.h>
#include <dos/var.h>

#include <string.h>
#include <stddef.h>

#include "internal/commands.h"
#include "which_rev.h"

#include "libhdr.h"

#include "clib/dos_protos.h"
#include "pragmas/dos_pragmas.h"


#define CMD_BUILTIN (-2L)
#define CMD_DISABLED (-999L)

extern void writef();

#define TEMPLATE    "FILE/A,NORES/S,RES/S,ALL/S" CMDREV
#define OPT_FILE    0
#define OPT_NORES   1
#define OPT_RES     2
#define OPT_ALL     3
#define OPT_COUNT   4

int cmd_which(void)
{
struct DosLibrary *DOSBase;
struct Library *SysBase = (*((struct Library **) 4));
long rc=RETURN_WARN,rc1;
long opts[OPT_COUNT];
struct RDargs *rdargs;
struct Path *p;
UBYTE *file;
struct Segment *seg;
struct CommandLineInterface *clip;
struct FileInfoBlock *exinfo;
UBYTE buffer[256];
BOOL ud=0;

if ((DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER))) {
   memset((char *)opts, 0, sizeof(opts));
   rdargs = ReadArgs(TEMPLATE, opts, NULL);
   exinfo=AllocVec(sizeof(struct FileInfoBlock),MEMF_PUBLIC|MEMF_CLEAR);

   if(rdargs) {
       clip=Cli();

       file=(UBYTE *)opts[OPT_FILE];
       if(!opts[OPT_NORES]) { /* search the resident list */
           if(!(seg=FindSegment(file,0,TRUE)))seg=FindSegment(file,0,0);
	   if(seg) {
		if(seg->seg_UC == CMD_BUILTIN) {
		    rc=0;
		    writef(DOSBase,"INTERNAL %s\n",file);
		}
		else if(seg->seg_UC == CMD_DISABLED) {
		    writef(DOSBase,"INTERNAL %s ;(DISABLED)\n",file);
		    /* so we want to continue with the search */
		}
	   	else {
		    writef(DOSBase,"RES %s\n",file);
	    	    rc = 0;
		}
           }
   	}
   	if( (opts[OPT_ALL] || rc)&&!opts[OPT_RES]) { /* search current dir */
	    if(file[strlen(file)-1]==':')ud=TRUE;
	    rc=testlock(DOSBase,THISPROC->pr_CurrentDir,file,buffer,exinfo,ud);
   	}

	/* if they specify the ALL option, or if we haven't */
    	/*   found anything , search the paths.  Note: we don't */
    	/*   do this if they specified an exact path to the file */

   	if( (opts[OPT_ALL] || rc) && (FilePart(file)==file) && !opts[OPT_RES]) { /* search the path */
   	    p = (struct Path *)BADDR(clip->cli_CommandDir);

   	    while (((rc || opts[OPT_ALL]) && p)) {

	       if(CheckSignal(SIGBREAKF_CTRL_C)) {
		    PrintFault(304,0);
		    goto leave;
	       }
	       rc1 = testlock(DOSBase,p->path_Lock,file,buffer,exinfo,0);

	       if(rc)rc=rc1; /* if found, change return code */
	       p = (struct Path *)BADDR(p->path_Next);
   	    }

   	    /* try c:  */
   	    if (opts[OPT_ALL] || rc) {
	    	struct DevProc *dp = NULL;
	    	struct MsgPort *ftask = GetFileSysTask();
	        do {
		    if(CheckSignal(SIGBREAKF_CTRL_C)) {
		    	PrintFault(304,0);
		    	break;
		    }
	      	    dp=GetDeviceProc("c:",dp);
	            if (dp) {
		  	SetFileSysTask(dp->dvp_Port);
		  	rc1 = testlock(DOSBase,
			   dp->dvp_Lock,file,buffer,exinfo,0);
	                if(rc)rc=rc1; /* if found, change return code */
	            }
	        /* repeat if multi-assign */
		} while ((rc || opts[OPT_ALL]) && dp && 
		(dp->dvp_Flags & DVPF_ASSIGN)&&IoErr()==ERROR_OBJECT_NOT_FOUND);
	    SetFileSysTask(ftask);
	    if (dp)FreeDeviceProc(dp);
   	    }
	}
   }
   else PrintFault(IoErr(),NULL);
leave:
   if(exinfo)FreeVec(exinfo);
   if(rdargs)FreeArgs(rdargs);
   CloseLibrary((struct Library *)DOSBase);
}
else { OPENFAIL }
return(rc);
}


LONG testlock( db, dir, file, buffer, exinfo, ud)
struct DosLibrary *db;
BPTR dir;
UBYTE *file;
UBYTE *buffer;
struct FileInfoBlock *exinfo;
BOOL ud;
{
   struct DosLibrary *DOSBase = db;

   LONG rc = RETURN_WARN;
   LONG res;
   BPTR lock;
   BPTR olddir = CurrentDir(dir);

   lock = Lock(file,SHARED_LOCK);	/* lock the object */
   if ( lock ) {	/* if we get it */
       if(res = Examine(lock,exinfo)) {	/* make sure its not a directory */
	   /* if its not a directory, and either the script bit or */
	   /* the execute bit is set ... */
           if( ((exinfo->fib_DirEntryType <0) || ud) && 
		((exinfo->fib_Protection & FIBF_SCRIPT) ||
		(!(exinfo->fib_Protection & FIBF_EXECUTE)))) {
	   	NameFromLock(lock, buffer, 255);
           	writef(db,"%s\n",buffer);
	   	rc = 0;
	   }
       }
       UnLock(lock);
   }
   CurrentDir (olddir);
   return(rc);
}

void writef( db, fmt, args )
struct DosLibrary *db;
char *fmt, *args;
{
   struct DosLibrary *DOSBase = db;

   VFWritef( Output(), fmt, (LONG *)&args );
}
