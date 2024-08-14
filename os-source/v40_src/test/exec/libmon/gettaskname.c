#include <exec/types.h>
#include <exec/memory.h>
#include <exec/nodes.h>
#include <exec/tasks.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include <stdlib.h>
#include <string.h>

extern struct Library *SysBase;
extern struct DosLibrary *DOSBase;

/* protos ----------------- */
STRPTR getTaskName(VOID);


/* getTaskName ===========================================================
   A Misnomer, really: gets you a copy of the task (or process) name.
   Null for error; string pointer if successful.
   YOU MUST FREEVEC THE RETURNED POINTER.
 */
STRPTR getTaskName(VOID) {
    struct Task *task;
    struct Process *proc = NULL;
    STRPTR srcStr, retStr=NULL, tmp, tmp2;
    struct CommandLineInterface *cli;
    STRPTR b_cmdName;
    LONG i;

    task = FindTask(NULL);
    
    if (task->tc_Node.ln_Type == NT_PROCESS) {
        proc = (struct Process *)task;
        if (proc->pr_CLI) {
            cli = BADDR(proc->pr_CLI);
            if (cli) {
                b_cmdName = BADDR(cli->cli_CommandName);
                if (tmp = (STRPTR)b_cmdName) {
                    i = (LONG)(*tmp);
                    retStr = AllocVec((ULONG)(i) + 1UL, MEMF_CLEAR);
                    if (retStr) {
                        tmp++;
                        tmp2 = retStr;
                        while (i--) {
                            *tmp2++ = *tmp++;
                        }
                        /* done: copy of command name into retStr */
                    }
                }
            }
            return(retStr);
        }
        else {
            srcStr = task->tc_Node.ln_Name;
            if (!srcStr) {
                return(NULL);  /* nothing we can ID as a name :-( */
            }
        }
    }
    else {
        srcStr = task->tc_Node.ln_Name;
        if (!srcStr) {
            return(NULL);  /* nothing we can ID as a name :-( */
        }
    }
    tmp = srcStr;
    i = (LONG)strlen(tmp);
    if (!i) {
        return(NULL);  /* zero-length string... */
    }
    retStr = AllocVec((ULONG)(i) + 1UL, MEMF_CLEAR);
    if (tmp2 = retStr) {
        while (i--) {
            *tmp2++ = *tmp++;
        }
    }

    return(retStr);
}

#ifdef TESTIT
VOID main(int argc, UBYTE *argv[]) {
    STRPTR foo;

    foo = getTaskName();
    if (foo) {
        Printf("Taskname: '%s'\n", foo);
        FreeVec(foo);
    }
    else {
        PutStr("Function failed!\n");
    }
    exit(0);
}
#endif
