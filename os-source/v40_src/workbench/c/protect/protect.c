
#include "internal/commands.h"
#include "protect_rev.h"

#define MSG_BADFLAG   "Invalid flag - must be one of SPARWED\n"
#define MSG_SETFAILED "Can't set protection for %s - "
#define MSG_NOMEM     "No memory\n"
#define MSG_DONE      "done\n"
#define MSG_DIR       " (dir)"
#define MSG_BLANKS    "        "
#define MSG_ADDSUB    "Can't specify both ADD (+) and SUB (-)\n"


#define TEMPLATE    "FILE/A,FLAGS,ADD/S,SUB/S,ALL/S,QUIET/S" CMDREV
#define OPT_FILE    0
#define OPT_FLAGS   1
#define OPT_ADD     2
#define OPT_SUB     3
#define OPT_ALL     4
#define OPT_QUIET   5
#define OPT_COUNT   6

#define NORMINDENT  3
#define TABSIZE     5

#define ALL_PROT_BITS (0x00000080|FIBF_SCRIPT|FIBF_PURE|FIBF_ARCHIVE|\
                       FIBF_READ|FIBF_WRITE|FIBF_EXECUTE|FIBF_DELETE)

#define BITFLIP(x,m) (( (x) & ~((LONG)m)) | ( ~(x) & ((LONG)m) ))

#define ANCHORNAMELEN 255
#define ANCHORSIZE    (sizeof(struct AnchorPath) + ANCHORNAMELEN)

/* Macro to get longword-aligned stack space for a structure	*/
/* Uses ANSI token catenation to form a name for the char array */
/* based on the variable name, then creates an appropriately	*/
/* typed pointer to point to the first longword boundary in the */
/* char array allocated.					*/
#define D_S(name, type) char a_##name[sizeof(type)+3]; \
			type *name = (type *)((LONG)(a_##name+3) & ~3);

struct uAnchorPath
{
   struct AnchorPath ap;
   char name[255];
};

int cmd_protect(void)
{
struct Library *SysBase = (*((struct Library **) 4));
struct DosLibrary *DOSBase;
int rc, rc2, indent, tindent, isdir;
BOOL apinit;
LONG mask, umask;
LONG oldbits;
char *msg, *curflag;
LONG opts[OPT_COUNT];
BPTR lock;
STRPTR extra;
struct RDargs *rdargs;
D_S(ap, struct uAnchorPath);

    rc = RETURN_FAIL;
    if ((DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER)))
    {
        memset((char *)opts, 0, sizeof(opts));
        if (!(rdargs = ReadArgs(TEMPLATE, &opts[0], NULL)))
        {
            PrintFault(IoErr(), NULL);
        }
        else
        {
            rc2    = 0;
            apinit = FALSE;
            msg    = NULL;
            extra  = NULL;

            umask = 0L;
            if (opts[OPT_FLAGS])
            {
                for (curflag = (char *)opts[OPT_FLAGS]; *curflag; curflag++)
                {
                    switch(*curflag)
                    {
                        case 'h': case 'H': umask |= 0x80;         break;
                        case 's': case 'S': umask |= FIBF_SCRIPT;  break;
                        case 'p': case 'P': umask |= FIBF_PURE;    break;
                        case 'a': case 'A': umask |= FIBF_ARCHIVE; break;
                        case 'r': case 'R': umask |= FIBF_READ;    break;
                        case 'w': case 'W': umask |= FIBF_WRITE;   break;
                        case 'e': case 'E': umask |= FIBF_EXECUTE; break;
                        case 'd': case 'D': umask |= FIBF_DELETE;  break;
                        case '+':           opts[OPT_ADD] = 1;    break;
                        case '-':           opts[OPT_SUB] = 1;    break;

                        default :           msg = MSG_BADFLAG;
                                            rc  = RETURN_FAIL;
                                            goto ERRORCASE;
                    }
                }

               if (umask == 0L)
               {
                   msg = MSG_BADFLAG;
                   rc  = RETURN_FAIL;
                   goto ERRORCASE;
               }
            }

            if (opts[OPT_ADD] && opts[OPT_SUB])
            {
                msg = MSG_ADDSUB;
                rc  = RETURN_FAIL;
                goto ERRORCASE;
            }

            if (opts[OPT_SUB])
                umask = ( ( (~umask) & ALL_PROT_BITS) | ~ALL_PROT_BITS);

            memset(ap, 0, sizeof(struct uAnchorPath));
            ap->ap.ap_Flags = APF_DOWILD;
            ap->ap.ap_Strlen = ANCHORNAMELEN;
            ap->ap.ap_BreakBits = SIGBREAKF_CTRL_C;

            apinit = TRUE;
            indent = 0;

            rc  = RETURN_FAIL;
            rc2 = MatchFirst((STRPTR)opts[OPT_FILE], &ap->ap);

            while (!rc2)
            {
                if (CheckSignal(SIGBREAKF_CTRL_C))
                {
                    rc2 = ERROR_BREAK;
                    break;
                }

                if (ap->ap.ap_Flags & APF_DIDDIR)
                {
                    /* Exiting a directory, skip this entry */
                    ap->ap.ap_Flags &= ~APF_DIDDIR;
                    indent--;
                    rc2 = MatchNext(&ap->ap);
                    continue;
                }

                isdir = (ap->ap.ap_Info.fib_DirEntryType >= 0);

                if (isdir && opts[OPT_ALL])
                {
                    /* Step into the directory */
                    ap->ap.ap_Flags |= APF_DODIR;
                    indent++;
                }

                if ((ap->ap.ap_Flags & APF_ITSWILD) || opts[OPT_ALL])
                {
                    if (!opts[OPT_QUIET])
                        for (tindent = 0; tindent < indent; tindent++)
                            WriteChars(MSG_BLANKS, TABSIZE);

                    if (!opts[OPT_QUIET])
                    {
                        if (!isdir)
                            WriteChars(MSG_BLANKS, NORMINDENT);

                        if (ap->ap.ap_Info.fib_FileName[0])
                            PutStr(ap->ap.ap_Info.fib_FileName);
                        else
                            PutStr((STRPTR)opts[OPT_FILE]);

                        if (isdir)
                            PutStr(MSG_DIR);

                        PutStr("..");
                    }
                }
                else
                {
                    tindent = -1;  /* We'll use tindent >= 0 below as a flag */
                }

                mask = umask;
                if (opts[OPT_ADD] || opts[OPT_SUB])
                {
                    /* Bottom four bits (RWED) are reverse-sense */
                    oldbits = BITFLIP(ap->ap.ap_Info.fib_Protection, 0x0f);

                    if (opts[OPT_ADD])
                        mask |= oldbits;
                    else /* OPT_SUB */
                        mask &= oldbits;
                }

                /* Bottom four bits (RWED) are reverse-sense */
                mask = BITFLIP(mask, 0x0f);

                lock = CurrentDir(ap->ap.ap_Current->an_Lock);

                if ((mask != ap->ap.ap_Info.fib_Protection) && (SetProtection(ap->ap.ap_Info.fib_FileName, mask) == DOSFALSE))
                {
                    rc  = RETURN_FAIL;
                    rc2 = IoErr();
                    msg = MSG_SETFAILED;
                    extra = ap->ap.ap_Buf;
                    PutStr("\n");
                }
                else
                {
                    if ((tindent >= 0) && !opts[OPT_QUIET])
                    {
                        /* If we put out the first part of the msg, put out the rest now */
                        PutStr(MSG_DONE);
                    }
                }

                CurrentDir(lock);

                if (!rc2)
                    rc2 = MatchNext(&ap->ap);
            }

            if (rc2 == ERROR_NO_MORE_ENTRIES)
            {
                rc2 = 0;
                rc  = RETURN_OK;
            }
            else if (rc2 == ERROR_BREAK)
            {
                rc = RETURN_WARN;
            }

   ERRORCASE:

            if (msg)
                VPrintf(msg, (ULONG *)&extra);

            if (rc2)
                PrintFault(rc2, NULL);

            FreeArgs(rdargs);

            if (apinit)
                MatchEnd(ap);

            SetIoErr(rc2);
        }

        CloseLibrary((struct Library *)DOSBase);
    }
    else {OPENFAIL;}

    return(rc);
}
