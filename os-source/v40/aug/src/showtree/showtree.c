#include    <exec/memory.h>
#include    <exec/nodes.h>
#include    <exec/lists.h>
#include    <exec/libraries.h>
#include    <dos/dos.h>
#include    <dos/dosextens.h>

#include    <clib/exec_protos.h>
#include    <pragmas/exec_pragmas.h>

#include    <clib/dos_protos.h>
#include    <pragmas/dos_pragmas.h>

#include    "showtree_rev.h"

/******************************************************************************/

/* This is the command template. */
#define TEMPLATE    "DIR,FILES/S,INDENT/K/N,MAXDEPTH/K/N,NOBLANK/S" VERSTAG

#define OPT_DIR     0
#define OPT_FILES   1
#define OPT_INDENT  2
#define OPT_DEPTH   3
#define	OPT_NOBLANK 4
#define OPT_COUNT   5

/*
 * Default values for these options
 */
#define DEF_DIR     "\0"
#define DEF_INDENT  2
#define DEF_DEPTH   64

struct  MyFIB
{
struct  MinNode         node;
        BPTR            lock;
struct  FileInfoBlock   fib;
};

#define POP_STACK   ((char *)1)

#define NEWLIST(l) {((struct MinList *)l)->mlh_Head = (struct MinNode *)&(((struct MinList *)l)->mlh_Tail);\
                    ((struct MinList *)l)->mlh_Tail = NULL;\
                    ((struct MinList *)l)->mlh_TailPred = (struct MinNode *)l;}

LONG cmd(void)
{
struct  Library     *SysBase = (*((struct Library **) 4));
struct  DosLibrary  *DOSBase;
struct  MinList     list;
        LONG        rc;

    NEWLIST(&list);

    rc=RETURN_FAIL;
    if (DOSBase = (struct DosLibrary *)OpenLibrary("dos.library",37))
    {
    struct  RDargs      *rdargs;
            LONG        opts[OPT_COUNT];
            LONG        x;
            LONG        i;

        for (x=0;x<OPT_COUNT;opts[x++]=0);

        if (rdargs=ReadArgs(TEMPLATE,opts,NULL))
        {
        struct  MyFIB   *myFIB;
                BPTR    OldDir;
                LONG    Depth=0;
                LONG    DoLine=FALSE;
                char    *dir=DEF_DIR;       /* Default directory */
                LONG    Indent=DEF_INDENT;  /* Default Indent */
                LONG    MaxDepth=DEF_DEPTH; /* Default Depth */

            if (opts[OPT_DEPTH]) MaxDepth=*((LONG *)opts[OPT_DEPTH]);
            if (opts[OPT_INDENT]) Indent=*((LONG *)opts[OPT_INDENT]);

            if (opts[OPT_DIR]) dir=(char *)opts[OPT_DIR];

            CurrentDir(OldDir=CurrentDir(NULL));

            while (dir)
            {
                myFIB=NULL;

                if (dir!=POP_STACK)
                {
                    if (myFIB=AllocVec(sizeof(struct MyFIB),MEMF_PUBLIC|MEMF_CLEAR))
                    {
                        if (myFIB->lock=Lock(dir,ACCESS_READ))
                        {
                            if (Examine(myFIB->lock,&(myFIB->fib)))
                            {
                                AddHead((struct List *)&list,(struct Node *)myFIB);
                                myFIB=NULL; /* All OK... */
                            }
                        }
                    }

                }

                dir=NULL;   /* Assume exit condition... */

                if (!myFIB)
                {
                    if (myFIB=(struct MyFIB *)RemHead((struct List *)&list))
                    {
                        dir=myFIB->fib.fib_FileName;
                        if ((!(SetSignal(0,0) & SIGBREAKF_CTRL_C)) && (ExNext(myFIB->lock,&(myFIB->fib))))
                        {
                            AddHead((struct List *)&list,(struct Node *)myFIB);
                            if ((opts[OPT_FILES]) || (myFIB->fib.fib_DirEntryType>0))
                            {
                                if ((DoLine) && (!opts[OPT_NOBLANK]))
                                {
                                    for (x=0;x<Depth;x++)
                                    {
                                        PutStr("|");
                                        for (i=0;i<Indent;i++) PutStr(" ");
                                    }
                                    PutStr("|\n");
                                    DoLine=FALSE;
                                }
                                for (x=0;x<Depth;x++)
                                {
                                    PutStr("|");
                                    for (i=0;i<Indent;i++) PutStr(" ");
                                }
                                PutStr("|");
                                for (i=0;i<Indent;i++) PutStr("-");
                                PutStr(dir);
                                if (myFIB->fib.fib_DirEntryType>0) PutStr("/");
                                PutStr("\n");
                                if ((myFIB->fib.fib_DirEntryType<0) || (Depth >= MaxDepth)) dir=POP_STACK;
                                else
                                {
                                    DoLine=TRUE;
                                    CurrentDir(myFIB->lock);
                                    Depth++;
                                }
                            }
                            else dir=POP_STACK;

                            myFIB=NULL; /* Everything is ok... */
                        }
                    }
                }

                /* If myFIB is non-NULL, we need to release it and pop up... */
                if (myFIB)
                {
                    CurrentDir(OldDir);
                    UnLock(myFIB->lock);
                    FreeVec(myFIB);
                    Depth--;
                    DoLine=TRUE;
                    dir=POP_STACK;
                }
            }

            rc=RETURN_OK;

            FreeArgs(rdargs);

            if (SetSignal(0,0) & SIGBREAKF_CTRL_C) PrintFault(ERROR_BREAK,NULL);
        }
        else PrintFault(IoErr(),NULL);

        CloseLibrary((struct Library *)DOSBase);
    }
    return(rc);
}
