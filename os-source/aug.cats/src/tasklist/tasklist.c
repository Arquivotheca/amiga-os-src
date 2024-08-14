;/* tasklist.c - Execute me to compile me with Lattice 5.04
LC -b1 -cfistq -v -y -j73 tasklist.c
Blink FROM LIB:c.o,tasklist.o TO tasklist LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit
*/

/* TaskList.c - lists tasks in system
 *
 * Copyright (c) 1990 Commodore-Amiga, Inc.
 *
 * This example is provided in electronic form by Commodore-Amiga, Inc. for
 * use with the 1.3 revisions of the Addison-Wesley Amiga reference manuals. 
 * The 1.3 Addison-Wesley Amiga Reference Manual series contains additional
 * information on the correct usage of the techniques and operating system
 * functions presented in this example.  The source and executable code of
 * this example may only be distributed in free electronic form, via bulletin
 * board or as part of a fully non-commercial and freely redistributable
 * diskette.  Both the source and executable code (including comments) must
 * be included, without modification, in any copy.  This example may not be
 * published in printed form or distributed with any commercial product.
 * However, the programming techniques and support routines set forth in
 * this example may be used in the development of original executable
 * software products for Commodore Amiga computers.
 * All other rights reserved.
 * This example is provided "as-is" and is subject to change; no warranties
 * are made.  All use is at your own risk.  No liability or responsibility
 * is assumed.
 */

#include "exec/types.h"
#include "exec/execbase.h"
#include "exec/tasks.h"
#include "dos/dos.h"
#include "dos/dosextens.h"
#ifdef LATTICE
#include <clib/exec_protos.h>
#include <stdlib.h>
#include <stdio.h>
int CXBRK(void) { return(0); }		/* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }	/* really */
#endif

UBYTE *commandname(struct Task *task);

char *vers = "\0$VER: tasklist 37.1";
char *Copyright = 
  "tasklist v37.1\nCopyright (c) 1990 Commodore-Amiga, Inc.  All Rights Reserved";


#define ARRAYSIZE 128L
extern struct ExecBase *SysBase;

void main(int argc,char **argv)
    {
    struct Task *task;
    LONG count=0, i=0, readi=0,thisi=0;
    char *names[ARRAYSIZE];
    UBYTE *comname;
    struct Task *tasks[ARRAYSIZE];

    if(argc > 1) printf("%s\n",Copyright), exit(0L);

    Disable();
    for ( task = (struct Task *)SysBase->TaskWait.lh_Head;
          (NULL != task->tc_Node.ln_Succ) && (count < ARRAYSIZE);
          task = (struct Task *)task->tc_Node.ln_Succ)
        {
	tasks[count] = task;
        names[count++] = task->tc_Node.ln_Name;
        }
    readi=count;
    for ( task = (struct Task *)SysBase->TaskReady.lh_Head;
          (NULL != task->tc_Node.ln_Succ) && (count < (ARRAYSIZE-1));
          task = (struct Task *)task->tc_Node.ln_Succ)
        {
	tasks[count] = task;
        names[count++] = task->tc_Node.ln_Name;
        }
    thisi=count;
    task = FindTask(NULL);
    tasks[count] = task;
    names[count++] = task->tc_Node.ln_Name;
    Enable();

    if (count == ARRAYSIZE) names[count-1]="error: array overflow"; 


    printf("WAITING Tasks:\n");
    for (i = 0; i < count; i++)
        {
        if(i==readi)  printf("READY tasks:\n");
        if(i==thisi)  printf("THIS task:\n");
        printf(" $%08lx  pri=%ld\tF=%ld D=%ld\t%s",
		tasks[i],
		tasks[i]->tc_Node.ln_Pri,
		tasks[i]->tc_TDNestCnt,
		tasks[i]->tc_IDNestCnt,
		names[i]);
    	if(comname=commandname(tasks[i]))	printf(" (%s)\n",comname);
	else printf("\n");
        }
    }


UBYTE *commandname(struct Task *task)
    {
    struct CommandLineInterface *cli;
    struct Process *pr;
    UBYTE  *s, *name;

    pr = (struct Process *)task;
    name = NULL;
    if((task->tc_Node.ln_Type == NT_PROCESS)&&(pr->pr_CLI))
            {
            cli = (struct CommandLineInterface *)(BADDR(pr->pr_CLI));
	    if(cli->cli_CommandName)
		{
            	s = (UBYTE *)(BADDR(cli->cli_CommandName)); 
            	if(s[0]) name = &s[1];
		}
            }
    return(name);
    }
