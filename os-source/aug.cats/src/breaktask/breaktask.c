;/* breaktask.c - Execute me to compile me with Lattice 5.04
LC -b1 -cfistq -v -y -j73 breaktask.c
Blink FROM LIB:c.o,breaktask.o TO breaktask LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <intuition/intuition.h>

#ifdef LATTICE
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif

BOOL matchcom(struct Task *task, UBYTE *name);
struct Task *findCLITask(UBYTE *name);

#define MINARGS 2

UBYTE *vers = "\0$VER: breaktask 37.10";
UBYTE *Copyright = 
  "breaktask v37.10\nCopyright (c) 1990 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *usage = "Usage: breaktask Task/CommandName [c][d][e][f] (default is CTRL-C)";


void main(int argc, char **argv)
    {
    struct Task *task;
    int k,i;
    ULONG mask = NULL;
    UBYTE c;
    
    if(((argc)&&(argc<MINARGS))||(argv[argc-1][0] =='?'))
	{
	printf("%s\n%s\n",Copyright,usage);
	exit(RETURN_OK);
	}

    task = FindTask(argv[1]);
    if(!task) task = findCLITask(argv[1]);

    if(task)
	{
	if(argc < 3) mask = SIGBREAKF_CTRL_C;
	else
	    {
	    for(k=2; k<argc; k++)
		{
		for(i=0; i<strlen(argv[k]); i++)
		    {
		    c = (argv[k][i]) | 0x20;
		    printf("c is %c\n",c);
		    if(c=='c') mask |= SIGBREAKF_CTRL_C;
		    if(c=='d') mask |= SIGBREAKF_CTRL_D;
		    if(c=='e') mask |= SIGBREAKF_CTRL_E;
		    if(c=='f') mask |= SIGBREAKF_CTRL_F;
		    }
		}
	    }
/*	printf("mask is 0x%08lx\n",mask); */
	Signal(task,mask);
	}
    else printf("Can't find exec task or CLI command \"%s\"\n",argv[1]);
    exit(RETURN_OK);
    }


struct Task *findCLITask(UBYTE *name)
    {
    extern struct ExecBase *SysBase;
    struct Task *task;

    Disable();
    for ( task = (struct Task *)SysBase->TaskWait.lh_Head;
          (NULL != task->tc_Node.ln_Succ);
          task = (struct Task *)task->tc_Node.ln_Succ)
        {
	if(matchcom(task,name))
	    {
	    Enable();
	    return(task);
	    }
	}

    for ( task = (struct Task *)SysBase->TaskReady.lh_Head;
          (NULL != task->tc_Node.ln_Succ);
          task = (struct Task *)task->tc_Node.ln_Succ)
        {
	if(matchcom(task,name))
	    {
	    Enable();
	    return(task);
	    }
	}

    task = FindTask(NULL);
    if(matchcom(task,name))
	{
	Enable();
	return(task);
	}

    Enable();
    return(NULL);
    }

BOOL matchcom(struct Task *task, UBYTE *name)
    {
    struct Process *pr;
    struct CommandLineInterface *cli;
    UBYTE *s, *cname;

    pr = (struct Process *)task;
    cname = NULL;
    if((task->tc_Node.ln_Type == NT_PROCESS)&&(pr->pr_CLI))
            {
            cli = (struct CommandLineInterface *)(BADDR(pr->pr_CLI));
	    if(cli->cli_CommandName)
		{
            	s = (UBYTE *)(BADDR(cli->cli_CommandName)); 
            	if(s[1]) cname = &s[1];
		}
            }
    if((cname)&&(!(stricmp(cname,name)))) return(TRUE);
    return(FALSE);
    }
