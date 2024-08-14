;/* ToolMenu.c - Execute me to compile me with Lattice 5.04
LC -b1 -cfistq -v -y -j73 ToolMenu.c
Blink FROM lib:ac.obj,lib:amain.obj,toolmenu.o TO toolmenu.ld LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <dos/dosextens.h>
#include <intuition/intuition.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>

#ifdef LATTICE
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/wb_protos.h>

extern struct Library *SysBase;
extern struct Library *DOSBase;
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/wb_pragmas.h>

#include <clib/commodities_protos.h>
#include <clib/alib_protos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif


extern LONG __asm WB2CLI (register __a0 struct WBStartup * wbmsg,
		    register __d0 ULONG DefaultStack,
		    register __a1 struct Library * DOSBase);


/* v36_16 mods: pragmas, only insert filenames if [] present (even at end)
 * v36_17 mods: add help
 */

#include "toolmenu_rev.h"


/**********    debug macros     **********
#define MYDEBUG	 1
*/

void kprintf(UBYTE *fmt,...);
void dprintf(UBYTE *fmt,...);

#define DEBTIME 0
#define bug printf
#if MYDEBUG
#define D(x) (x); if(DEBTIME>0) Delay(DEBTIME);
#else
#define D(x) ;
#endif /* MYDEBUG */
/********** end of debug macros **********/

#define MINARGS 1


/* for special wmain startup  (may be deleted for c.o) */
BOOL UseMyWbCon = FALSE;
UBYTE  *MyWbCon = "";

UBYTE *vers = VERSTAG;
UBYTE *Copyright = 
  VERS "\nCopyright (c) 1990 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *usage = "Usage: Toolmenu help | name=path/command name=path/command...\n";
UBYTE *help = "ToolMenu adds commands to the Workbench Tools menu.\n"
	      "Start from Workbench or WbStartup with ToolTypes menuitem=path/command.\n"
              "Or run >NIL: from CLI with \"menuitem=path/command\" arguments.\n"
              "Use [] within or at end of command for insertion of names of selected icons.\n"
              "ToolMenu toggles.  If already running, ToolMenu will remove its menuitems.\n";

UBYTE *autocon="CON:0/30/640/270/Toolmenu Command/auto/close/wait";

VOID doExternal(UBYTE *command);
void helpexit(int);
void bye(UBYTE *s, int e);
void cleanup(void);

struct Library *IntuitionBase;
struct Library *WorkbenchBase;
struct Library *IconBase;
struct Library *CxBase;

/***********************************************************************
 *                                                                     *
 *                            COPYRIGHTS                               *
 *                                                                     *
 *   Copyright (c) 1990  Commodore-Amiga, Inc.  All Rights Reserved.   *
 *                                                                     *
 **********************************************************************/

#define MAXTT 256
#define MAXMNAME 32
#define MAXAMIS 80
#define MYNAME "CATS_ToolMenu (c) 1990 CBM"

UBYTE *oldname;

struct MyAMI {
   struct AppMenuItem *ami;
   UBYTE callit[MAXTT];
   UBYTE *doit;
   };

struct MsgPort *msgport = NULL;
struct MyAMI *myamis = NULL;
ULONG amicnt = NULL;

#define MAXCOM 512
#define CBUFSZ (MAXCOM + 32)
#define room(x,pl)	(MAXCOM - (x + pl))

#define BREAKSIG	(SIGBREAKF_CTRL_E)

void main(int argc, char **argv)
    {
    struct Task *task;
    struct AppMenuItem *dummy;
    struct AppMessage *amsg;
    struct WBArg   *argptr;
    struct Process *proc;
    ULONG  signals;
    BOOL   Done = FALSE, HaveSome = FALSE, Insert;
    UBYTE c, **ttypes, *p;
    UBYTE cbuf[CBUFSZ];
    int             f, t, a, i, k, pl;

    proc = (struct Process *)FindTask(NULL);

    if((argc>1)&&(argv[argc-1][0] == '?')) helpexit(1);

    if((argc>1)&&(!(stricmp(argv[1],"help"))))	helpexit(0);


    if(!argc) WB2CLI((struct WBStartup *)argv,
			proc->pr_StackSize, (struct DosLibrary *)DOSBase);


    Forbid();
    if(task=FindTask(MYNAME))
	{
	Signal(task,BREAKSIG);
	Permit();
	bye("", RETURN_OK);
	}
    else
	{
	task = FindTask(NULL);
	oldname = task->tc_Node.ln_Name;
	task->tc_Node.ln_Name = MYNAME;
	}
    Permit();

    CxBase       = OpenLibrary("commodities.library",36);
    IntuitionBase= OpenLibrary("intuition.library",33);
    WorkbenchBase= OpenLibrary("workbench.library",36);
    IconBase     = OpenLibrary("icon.library",36);

    if ( ! ( IntuitionBase && CxBase && WorkbenchBase && IconBase) )
      	bye("Failed to open v36 Commodities, Intuition, Workbench, or Icon library\n",
		RETURN_FAIL);

    if( ! (msgport = CreateMsgPort()))
	bye("CreateMsgPort failed\n",RETURN_FAIL);


    if( ! (ttypes = ArgArrayInit( argc, argv )))
	{
	helpexit(argc);
	}
    else
	{
	for(k=0; ttypes[k]; k++);
	amicnt = k;
	D(bug("amicnt = %ld\n",amicnt));

        if(!( myamis =
	   (struct MyAMI *)AllocMem(amicnt * sizeof(struct MyAMI),MEMF_CLEAR)))
	    {
	    ArgArrayDone();
	    bye("Not enough memory\n",RETURN_FAIL);
	    }

	for(k=0; k < amicnt; k++)
	    {
	    strcpy(myamis[k].callit, ttypes[k]);
	    for(i=0; (c = myamis[k].callit[i]) && (c != '='); i++);
	    if(c == '=')
		{
	    	myamis[k].callit[i] = '\0';
	        myamis[k].doit = &myamis[k].callit[i+1];
		D(bug("callit=%s  doit=%s\n",myamis[k].callit,myamis[k].doit));
		}
 	    }
    	ArgArrayDone();

	if(!k) helpexit(argc);

	k=0;
	while((k<40)&&(!(dummy=AddAppMenuItem(99,NULL,"Test", msgport, NULL))))
	    {
	    Delay(25);
	    k++;
	    }

	if(dummy) RemoveAppMenuItem(dummy);
	else bye("ToolMenu: Can't add AppMenuItems - Workbench not ready\n",RETURN_FAIL);

	/* Add the AppMenuItems */
	for(k=0; k<amicnt; k++)
	    {
	    if(myamis[k].doit)
		{
	        if(myamis[k].ami =
		      AddAppMenuItem(k,(ULONG)(myamis[k].doit),
				myamis[k].callit, msgport, NULL))
			HaveSome = TRUE;
		D(bug("%s ami=%ld\n",myamis[k].callit,myamis[k].ami));
		}
	    }

	if(!HaveSome) helpexit(argc);
	
	while((HaveSome)&&(!Done))
	    {
	    signals = Wait((1 << msgport->mp_SigBit) | BREAKSIG);
	    D(bug("wakeup: signals = $%08lx\n",signals));
	    if(signals & BREAKSIG) Done = TRUE;
	    else if(signals & 1 << msgport->mp_SigBit)
		{
		while (amsg = (struct AppMessage *) GetMsg(msgport))
		    {
		    Insert=FALSE;
		    p = (UBYTE *)amsg->am_UserData;	/* the command */
		    /* copy till we hit a '[', or end (null), or are full */
		    for(f=0; (f<MAXCOM) && p[f] && (p[f]!='['); f++)
			{
			cbuf[f] = p[f];
			}
		    t=f;	/* f (from p index), t (to cbuf index) */
		    /* skip insert arg markers if that's why we stopped */
		    if((p[f]=='[')&&(p[f+1]==']')) Insert=TRUE, f+=2;

		    pl = strlen(&p[f]);	/* length of rest of command if any */
		    pl = pl + 4;	/* room for a space, two quotes, etc. */


#ifdef MYDEBUG
		    D(bug("am: appmsg=%lx, Type=%ld, ID=%ld, UserData=%s, NumArgs=%ld\n", 
			amsg, amsg->am_Type, amsg->am_ID, 
			amsg->am_UserData, amsg->am_NumArgs));
		    argptr = amsg->am_ArgList;
		    for (i = 0; i < amsg->am_NumArgs; i++) {
		        D(bug("\targ(%ld): Name='%s', Lock=%lx\n", i,
			argptr->wa_Name, argptr->wa_Lock));
		        argptr++;
		        }
#endif

		    /* add args if any, as long as we have room */
		    if(Insert)
		      {
		      argptr = amsg->am_ArgList;
		      for(a=0; (t<(MAXCOM-pl))&&(a<amsg->am_NumArgs); a++)
			{
			/* insert the names bumping up t */
		    	cbuf[t++] = ' ';
		    	cbuf[t++] = '"';
			if(NameFromLock(argptr[a].wa_Lock,&cbuf[t],MAXCOM-(pl+t)))
			    {
			    D(bug("Lock is $%lx, space is %ld,NameFromLock is %s\n",
					argptr[a].wa_Lock, MAXCOM-(pl+t), &cbuf[t]));
			    if((argptr[a].wa_Name == NULL)||
			      ((argptr[a].wa_Name)&&(AddPart(&cbuf[t],
				argptr[a].wa_Name,MAXCOM-(pl + strlen(cbuf))))))
				{
			    	t=strlen(cbuf);
		    	    	cbuf[t++] = '"';
				}
			    else	/* NameFromLock succeeded, AddPart failed */
				{
				t = t-2;
				cbuf[t]='\0';
				}
			    }
			else
			    {
			    t = t-2;	/* NameFromLock failed */
			    cbuf[t] = '\0';
			    }
			}
		      if(p[f]) cbuf[t++] = ' ';
		      }

		    /* copy rest of command string if any */		    	
		    D(bug("After insert, f=%ld, t=%ld, string at p[f] is %s\n",
				f,t,&p[f])); 
		    while(p[f] && (t < (MAXCOM-2)))
			{
			cbuf[t++] = p[f++];
			}
		    cbuf[t] = '\0';	/* null terminate command */
		    D(bug("cbuf len=%ld, string:\n%s\n",strlen(cbuf),cbuf));
		    doExternal((UBYTE *)(cbuf));
		    ReplyMsg((struct Message *) amsg);
		    }
		}		
	    }


	/* Done - Remove the AppMenuItems */
	for(k=0; k<amicnt; k++)
	    {
	    if(myamis[k].ami) RemoveAppMenuItem(myamis[k].ami);
	    }

	DeleteMsgPort(msgport);
	}
    bye("",RETURN_OK);
    }


VOID doExternal(UBYTE *command)
    {
    struct TagItem stags[5];
    BPTR file;

    if(file = Open(autocon,MODE_OLDFILE))
	{
	stags[0].ti_Tag = SYS_Input;
	stags[0].ti_Data = file;
	stags[1].ti_Tag = SYS_Output;
	stags[1].ti_Data = NULL;
	stags[2].ti_Tag = SYS_Asynch;
	stags[2].ti_Data = TRUE;
	stags[3].ti_Tag = SYS_UserShell;
	stags[3].ti_Data = TRUE;
	stags[4].ti_Tag = TAG_DONE;
        System(command, stags);
	}
    }

void helpexit(int argc)
    {
    printf("%s\n%s\n",Copyright,usage);
    if(!argc) printf(help);
    bye("",RETURN_OK);
    }

void bye(UBYTE *s, int e)
    {
    if(*s) printf(s);
    cleanup();
    exit(e);
    }

void cleanup()
    {
    if(myamis) FreeMem(myamis, amicnt * sizeof(struct MyAMI));	
    if(CxBase)		CloseLibrary(CxBase);
    if(IntuitionBase)	CloseLibrary(IntuitionBase);
    if(WorkbenchBase)	CloseLibrary(WorkbenchBase);
    if(IconBase)	CloseLibrary(IconBase);
    if(oldname) FindTask(NULL)->tc_Node.ln_Name = oldname;
    }

