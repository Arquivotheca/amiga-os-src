
/* Notes:   This algorithm differs from the standard quick sort in the
 *          following ways.
 *  1: This routine is non-recursive.  It instead uses its own stack to
 *     keep track of the sublists.
 *  2: There is no penalty for having an already completely or partially
 *     sorted data file.
 *  3: This routine uses linked lists to keep track of the data instead
 *     of arrays.
 */

#include <internal/commands.h>
#include <dos/stdio.h>
#include <ctype.h>
#include "sort_rev.h"


/*****************************************************************************/


#define TEMPLATE    "FROM/A,TO/A,COLSTART/K,CASE/S,NUMERIC/S,DESCENDING/S" CMDREV
#define OPT_FROM     0	     /* input file name 	      */
#define OPT_TO	     1	     /* output file name	      */
#define OPT_COLSTART 2	     /* column compare start position */
#define OPT_CASE     3       /* case insensitive              */
#define OPT_NUMERIC  4       /* sort numerically              */
#define OPT_DESC     5       /* sort in descending order      */
#define OPT_COUNT    6


/*****************************************************************************/


#define MSG_ERROR_READING     "Couldn't read '%s' - "
#define MSG_ERROR_WRITING     "Couldn't write to '%s' - "
#define MSG_ERROR_COULNT_OPEN "Couldn't open output file '%s' - "
#define MSG_INPUT_FILE_EMPTY  "Input file is empty\n"


/*****************************************************************************/


/* one of these for each line being sorted */
struct Line
{
    struct Line *ln_Next;
    ULONG        ln_SortKey;
    char        *ln_Data;
};


/* this is the format for a sublist on the stack */
struct Sublist
{
    struct Sublist *sl_Next;
    struct Line    *sl_Head;
    struct Line    *sl_Tail;
};


/*****************************************************************************/


#define SWAPDATA(n1,n2) {char *temp; LONG templ; temp = n1->ln_Data; templ = n1->ln_SortKey; n1->ln_Data = n2->ln_Data; n1->ln_SortKey = n2->ln_SortKey; n2->ln_Data = temp; n2->ln_SortKey = templ; }


/*****************************************************************************/


struct GlobalData
{
    LONG            StartCol;
    APTR            Buffer;
    APTR            Pool;
    struct Line    *ListHead;
    struct Line    *ListTail;
    struct Library *DOSBase;
    struct Library *SysBase;

    BOOL            Numeric;
    BOOL            CaseSensitive;
};


typedef BOOL (*COMPFUNC)(struct Line *,struct Line *,struct GlobalData *);
static BOOL CompareStr(struct Line *, struct Line *, struct GlobalData *);
static BOOL CompareStrCase(struct Line *, struct Line *, struct GlobalData *);
static BOOL CompareNum(struct Line *, struct Line *, struct GlobalData *);
static LONG ReadInputFile(struct GlobalData *gd, STRPTR name);

#define DOSBase gd.DOSBase
#define SysBase gd.SysBase


/*****************************************************************************/


LONG cmd_sort(VOID)
{
LONG              rc;
LONG              opts[OPT_COUNT];
struct RDArgs    *rdargs;
LONG              i;
struct Sublist   *stack;   /* stack for the sublists	          */
struct Sublist   *s;       /* current stack sublist in use  	  */
struct Sublist   *cur;     /* next/current sublist going on stack */
struct Line      *ptr;     /* generic pointer to use as needed    */
struct Line      *div;     /* sublist divider		          */
struct Line      *modiv;   /* sublist divider minus one	          */
struct GlobalData gd;
COMPFUNC          compare;
BPTR              file;
LONG              errorCode;

    rc  = RETURN_FAIL;
    memset(&gd, 0, sizeof(struct GlobalData));

    SysBase = (*((struct Library **) 4));

    if (DOSBase = OpenLibrary("dos.library", 39))
    {
        memset(opts, 0, sizeof(opts));
        if (rdargs = ReadArgs(TEMPLATE, opts, NULL))
        {
            if (gd.Pool = CreatePool(MEMF_ANY,4096,512))
            {
                /* from here on out it anything bad happens it's an error */
                rc = RETURN_ERROR;

                if (opts[OPT_COLSTART])
                {
                    StrToLong((STRPTR)opts[OPT_COLSTART], &gd.StartCol);
                    if (gd.StartCol > 0)
                        gd.StartCol--;

                    if (gd.StartCol < 0)
                        gd.StartCol = 0;
                }

                if (opts[OPT_NUMERIC])
                {
                    gd.Numeric = TRUE;
                    compare = CompareNum;
                }
                else
                {
                    if (opts[OPT_CASE])
                    {
                        compare = CompareStrCase;
                        gd.CaseSensitive = TRUE;
                    }
                    else
                    {
                        compare = CompareStr;
                    }
                }

                if (!(errorCode = ReadInputFile(&gd,(STRPTR)opts[OPT_FROM])))
                {
                    if (!gd.ListHead)
                    {
                        Printf(MSG_INPUT_FILE_EMPTY,opts[OPT_FROM]);
                    }
                    else
                    {
                        /* allocate initial stack space */
                        if (stack = AllocPooled(gd.Pool,sizeof(struct Sublist)))
                        {
                            if (file = Open((STRPTR)opts[OPT_TO], MODE_NEWFILE))
                            {
                                /* initialize the stack */
                                stack->sl_Next = NULL;
                                stack->sl_Head = gd.ListHead;
                                stack->sl_Tail = gd.ListTail;

                                /* sort the list */
                                while (stack)
                                {
                                    /* check for CTRL-C */
                                    if (CheckSignal(SIGBREAKF_CTRL_C))
                                    {
                                        errorCode = ERROR_BREAK;
                                        break;
                                    }

                                    /* get next sublist off the stack */
                                    s = stack;
                                    stack = stack->sl_Next;

                                    /* places everyone; its curtain time */
                                    modiv = div = s->sl_Head;
                                    ptr = s->sl_Head->ln_Next;

                                    /* process this sublist */
                                    if (opts[OPT_DESC])
                                    {
                                        while (ptr)
                                        {
                                            if (!(*compare)(s->sl_Head, ptr, &gd))
                                            {
                                                /* swap these two data pointers */
                                                modiv = div;
                                                div = div->ln_Next;
                                                SWAPDATA(div,ptr);
                                            }

                                            if (ptr == s->sl_Tail)
                                                break;

                                            ptr = ptr->ln_Next;
                                        }
                                    }
                                    else
                                    {
                                        while (ptr)
                                        {
                                            if ((*compare)(s->sl_Head, ptr, &gd))
                                            {
                                                /* swap these two data pointers */
                                                modiv = div;
                                                div = div->ln_Next;
                                                SWAPDATA(div,ptr)
                                            }

                                            if (ptr == s->sl_Tail)
                                                break;

                                            ptr = ptr->ln_Next;
                                        }
                                    }

                                    /* swap the comparer and the divider */
                                    SWAPDATA(div,s->sl_Head);

                                    /* adjust the stack */
                                    if (s->sl_Head != modiv)
                                    {
                                        if (!(cur = AllocPooled(gd.Pool,sizeof(struct Sublist))))
                                        {
                                            errorCode = ERROR_NO_FREE_STORE;
                                            break;
                                        }

                                        cur->sl_Next = stack;
                                        cur->sl_Head = s->sl_Head;
                                        cur->sl_Tail = modiv;
                                        stack        = cur;
                                    }
                                    div = div->ln_Next;

                                    if (div && (div != s->sl_Tail) && (!s->sl_Tail || (div != s->sl_Tail->ln_Next)))
                                    {
                                        if (!(cur = AllocPooled(gd.Pool,sizeof(struct Sublist))))
                                        {
                                            errorCode = ERROR_NO_FREE_STORE;
                                            break;
                                        }

                                        cur->sl_Next = stack;
                                        cur->sl_Head = div;
                                        cur->sl_Tail = s->sl_Tail;
                                        stack        = cur;
                                    }

                                    /* free up the current sublist stack frame */
                                    FreePooled(gd.Pool,s,sizeof(struct Sublist));
                                }

                                if (!stack)
                                {
                                    SetVBuf(file,NULL,BUF_FULL,4096);

                                    /* print out the data */
                                    for (ptr = gd.ListHead; ptr; ptr = ptr->ln_Next)
                                    {
                                        /* check for CTRL-C */
                                        if (CheckSignal(SIGBREAKF_CTRL_C))
                                        {
                                            errorCode = ERROR_BREAK;
                                            break;
                                        }

                                        /* change the newline back from a null to a newline */
                                        i = strlen(ptr->ln_Data);
                                        ptr->ln_Data[i] = '\n';
                                        if (FWrite(file, ptr->ln_Data, i+1, 1) != 1)
                                        {
                                            errorCode = IoErr();
                                            Printf(MSG_ERROR_WRITING,opts[OPT_TO]);
                                            break;
                                        }
                                    }
                                }

                                if (Close(file))
                                {
                                    rc = RETURN_OK;
                                }
                                else
                                {
                                    errorCode = IoErr();
                                    Printf(MSG_ERROR_WRITING,opts[OPT_TO]);
                                }
                            }
                            else
                            {
                                errorCode = IoErr();
                                Printf(MSG_ERROR_COULNT_OPEN,opts[OPT_TO]);
                            }
                        }
                        else
                        {
                            errorCode = ERROR_NO_FREE_STORE;
                        }
                    }
                }
                else
                {
                    Printf(MSG_ERROR_READING,opts[OPT_FROM]);
                }

                DeletePool(gd.Pool);
            }
            else
            {
                errorCode = ERROR_NO_FREE_STORE;
            }

            FreeArgs(rdargs);
        }
        else
        {
            errorCode = IoErr();
        }

        if (errorCode)
        {
            PrintFault(errorCode,NULL);

            if (errorCode == ERROR_BREAK)
                rc = RETURN_WARN;
        }

        CloseLibrary(DOSBase);
    }
    else
    {
        OPENFAIL;
    }

    return(rc);
}


/*****************************************************************************/


#undef DOSBase
#undef SysBase
#define DOSBase gd->DOSBase
#define SysBase gd->SysBase


/*****************************************************************************/


static LONG ReadInputFile(struct GlobalData *gd, STRPTR name)
{
BPTR                           file;
struct FileInfoBlock __aligned fib;
struct Line                   *tail;
BOOL                           atEnd;
struct Line                   *line;
char                          *ptr;
char                          *start;
LONG                           bytes;
LONG                           i;

    if (file = Open(name, MODE_OLDFILE))
    {
        if (ExamineFH(file,&fib))
        {
            if (gd->Buffer = AllocPooled(gd->Pool,fib.fib_Size+1))
            {
                ((UBYTE *)gd->Buffer)[fib.fib_Size] = 0;

                if (Read(file,gd->Buffer,fib.fib_Size) == fib.fib_Size)
                {
                    atEnd = TRUE;
                    tail  = (struct Line *)&gd->ListHead;
                    ptr   = gd->Buffer;
                    start = ptr;
                    bytes = fib.fib_Size;
                    while (bytes--)
                    {
                        if (*ptr == '\n')
                        {
                            *ptr = '\0';

                            if (!(line = AllocPooled(gd->Pool,sizeof(struct Line))))
                            {
                                SetIoErr(ERROR_NO_FREE_STORE);
                                break;
                            }

                            line->ln_Data = start;

                            i = gd->StartCol;
                            while (*start && i--)
                               start++;

                            if (gd->Numeric)
                            {
                                StrToLong(start, &line->ln_SortKey);
                            }
                            else
                            {
                                line->ln_SortKey = 0;
                                for (i = 0; i < 4; i++)
                                {
                                    if (gd->CaseSensitive)
                                        line->ln_SortKey = line->ln_SortKey*256 + (*start);
                                    else
                                        line->ln_SortKey = line->ln_SortKey*256 + toupper(*start);

                                    if (*start)
                                        start++;
                                }
                            }

                            /* Alternate between storing new lines at the
                             * head, or at the tail of the list. This scrambles
                             * data which is already sorted, which improves
                             * performance of the quick sort algorithm.
                             */

                            if (atEnd)
                            {
                                tail->ln_Next = line;
                                tail          = line;
                                atEnd         = FALSE;
                            }
                            else
                            {
                                line->ln_Next  = gd->ListHead;
                                gd->ListHead = line;
                                atEnd          = TRUE;
                            }
                            start = ptr + 1;
                        }
                        ptr++;
                    }

                    if (line)
                    {
                        tail->ln_Next = NULL;
                        SetIoErr(0);
                    }
                }
            }
            else
            {
                SetIoErr(ERROR_NO_FREE_STORE);
            }
        }

        Close(file);
    }

    /* if we're doing strings, then skip over the data stored in the
     * sort key. No need to compare that data twice...
     */
    if (!gd->Numeric)
        gd->StartCol += 4;

    return(IoErr());
}


/*****************************************************************************/


static BOOL CompareStr(struct Line *l1, struct Line *l2, struct GlobalData *gd)
{
STRPTR s1, s2;
LONG   i;

    if (l1->ln_SortKey > l2->ln_SortKey)
        return (TRUE);

    if (l1->ln_SortKey < l2->ln_SortKey)
        return (FALSE);

    s1 = l1->ln_Data;
    s2 = l2->ln_Data;

    if (i = gd->StartCol)
    {
        while (*s1 && i--) s1++;

        i = gd->StartCol;
        while (*s2 && i--) s2++;
    }

    return ((BOOL)(stricmp(s1, s2) > 0));
}


/*****************************************************************************/


static BOOL CompareStrCase(struct Line *l1, struct Line *l2, struct GlobalData *gd)
{
STRPTR s1, s2;
LONG   i;

    if (l1->ln_SortKey > l2->ln_SortKey)
        return (TRUE);

    if (l1->ln_SortKey < l2->ln_SortKey)
        return (FALSE);

    s1 = l1->ln_Data;
    s2 = l2->ln_Data;

    if (i = gd->StartCol)
    {
        while (*s1 && i--) s1++;

        i = gd->StartCol;
        while (*s2 && i--) s2++;
    }

    return ((BOOL)(strcmp(s1, s2) > 0));
}


/*****************************************************************************/


static BOOL CompareNum(struct Line *l1, struct Line *l2, struct GlobalData *gd)
{
    return ((BOOL)(l1->ln_SortKey > l2->ln_SortKey));
}
