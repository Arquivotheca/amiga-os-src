/*
 *	This module is derived from a Usenet posting by Bryce Nesbitt
 *	called stack_test.c, dated 4-Dec-88.  Thanks Bryce!
 *
 *	We check the current stack size and issue a warning if the
 *	size seems too small.  Our definition of too small is rather
 *	arbitrary.  Bru's stack use is dependent upon how deeply nested
 *	the tree is that it is working with, and there is a minimum of
 *	about 4K, which is dangeriously close to the default for the
 *	Amiga.  Most reasonable tree structures will cause about 10K or
 *	less of stack to be used.  We set our warning point at 20K, which
 *	catches the case of the user not changing it from the default
 *	(and thus probably not being aware that more stack may be 
 *	necessary) and also catches the case where the user did set
 *	the stack but may not have set it high enough for general use.
 *
 */

#include "globals.h"

#define	MINSTACK (20 * 1024)

void stackcheck (void)
{
    struct Process *Process;
    struct CommandLineInterface *CLI;
    long stack;

    DBUG_ENTER ("stackcheck");
    Process = (struct Process *) FindTask (0L);
    if (CLI = (struct CommandLineInterface *) (Process -> pr_CLI << 2)) {
	DBUG_PRINT ("stack", ("this is a CLI process"));
	stack = CLI -> cli_DefaultStack << 2;
    } else {
	DBUG_PRINT ("stack", ("this is not a CLI process"));
	stack = Process -> pr_StackSize;
    }
    DBUG_PRINT ("stack", ("actual stack allocated is %ld bytes", stack));
    if (stack < MINSTACK) {
	bru_message (MSG_STACK, stack, MINSTACK);
    }
    DBUG_VOID_RETURN;
}


