
#include "internal/commands.h"
#include "protect_rev.h"

#define MSG_BADFLAG   "Invalid flag - must be one of SPARWED\n"
#define MSG_SETFAILED "Can't set protection for %s - "
#define MSG_NOMEM     "No memory\n"
#define MSG_DONE      "done\n"
#define MSG_DIR       " (dir)"
#define MSG_BLANKS    "        "
#define MSG_ADDSUB    "Can't specify both ADD (+) and SUB (-)\n"


#define TEMPLATE    "FILE/A,FLAGS=USERFLAGS,GROUPFLAGS,OTHERFLAGS,ADD/S,SUB/S,ALL/S,QUIET/S" CMDREV
#define OPT_FILE    0
#define OPT_UFLAGS  1
#define OPT_GFLAGS  2
#define OPT_OFLAGS  3
#define OPT_ADD     4
#define OPT_SUB     5
#define OPT_ALL     6
#define OPT_QUIET   7
#define OPT_COUNT   8

#define NORMINDENT  3
#define TABSIZE     5

#define ALL_PROT_BITS (0x00000080|FIBF_SCRIPT|FIBF_PURE|FIBF_ARCHIVE|\
                       FIBF_READ|FIBF_WRITE|FIBF_EXECUTE|FIBF_DELETE|\
                       FIBF_GRP_READ|FIBF_GRP_WRITE|FIBF_GRP_EXECUTE|FIBF_GRP_DELETE|\
                       FIBF_OTR_READ|FIBF_OTR_WRITE|FIBF_OTR_EXECUTE|FIBF_OTR_DELETE)

#define BITFLIP(x,m) (( (x) & ~((LONG)m)) | ( ~(x) & ((LONG)m) ))

#define ANCHORNAMELEN 255
#define ANCHORSIZE    (sizeof(struct AnchorPath) + ANCHORNAMELEN)

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
LONG mask;
char *msg, *curflag;
LONG opts[OPT_COUNT];
BPTR lock;
STRPTR extra;
struct RDArgs *rdargs;
struct uAnchorPath __aligned ap;
ULONG bits;
ULONG impact;

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

            bits   = 0;
            impact = 0;
            if (opts[OPT_UFLAGS])
            {
                impact |= 0xff;
                for (curflag = (char *)opts[OPT_UFLAGS]; *curflag; curflag++)
                {
                    switch (*curflag)
                    {
                        case 'h': case 'H': bits |= 0x80;         break;
                        case 's': case 'S': bits |= FIBF_SCRIPT;  break;
                        case 'p': case 'P': bits |= FIBF_PURE;    break;
                        case 'a': case 'A': bits |= FIBF_ARCHIVE; break;
                        case 'r': case 'R': bits |= FIBF_READ;    break;
                        case 'w': case 'W': bits |= FIBF_WRITE;   break;
                        case 'e': case 'E': bits |= FIBF_EXECUTE; break;
                        case 'd': case 'D': bits |= FIBF_DELETE;  break;
                        case '+':           opts[OPT_ADD] = 1;    break;
                        case '-':           opts[OPT_SUB] = 1;    break;

                        default :           msg = MSG_BADFLAG;
                                            rc  = RETURN_FAIL;
                                            goto ERRORCASE;
                    }
                }
            }

            if (opts[OPT_GFLAGS])
            {
                impact |= 0xf00;
                for (curflag = (char *)opts[OPT_GFLAGS]; *curflag; curflag++)
                {
                    switch (*curflag)
                    {
                        case 'r': case 'R': bits |= FIBF_GRP_READ; break;
                        case 'w': case 'W': bits |= FIBF_GRP_WRITE; break;
                        case 'e': case 'E': bits |= FIBF_GRP_EXECUTE; break;
                        case 'd': case 'D': bits |= FIBF_GRP_DELETE; break;
                        case '+':           opts[OPT_ADD] = 1; break;
                        case '-':           opts[OPT_SUB] = 1; break;

                        default :           msg = MSG_BADFLAG;
                                            rc = RETURN_FAIL;
                                            goto ERRORCASE;
                    }
                }
            }

            if (opts[OPT_OFLAGS])
            {
                impact |= 0xf000;
                for (curflag = (char *)opts[OPT_OFLAGS]; *curflag; curflag++)
                {
                    switch (*curflag)
                    {
                        case 'r': case 'R': bits |= FIBF_OTR_READ; break;
                        case 'w': case 'W': bits |= FIBF_OTR_WRITE; break;
                        case 'e': case 'E': bits |= FIBF_OTR_EXECUTE; break;
                        case 'd': case 'D': bits |= FIBF_OTR_DELETE; break;
                        case '+':           opts[OPT_ADD] = 1; break;
                        case '-':           opts[OPT_SUB] = 1; break;

                        default :           msg = MSG_BADFLAG;
                                            rc = RETURN_FAIL;
                                            goto ERRORCASE;
                    }
                }
            }

            if (impact == 0)
            {
                msg = MSG_BADFLAG;
                rc  = RETURN_FAIL;
                goto ERRORCASE;
            }

            if (opts[OPT_ADD] && opts[OPT_SUB])
            {
                msg = MSG_ADDSUB;
                rc  = RETURN_FAIL;
                goto ERRORCASE;
            }

            memset(&ap, 0, sizeof(struct uAnchorPath));
            ap.ap.ap_Flags = APF_DOWILD;
            ap.ap.ap_Strlen = ANCHORNAMELEN;
            ap.ap.ap_BreakBits = SIGBREAKF_CTRL_C;

            apinit = TRUE;
            indent = 0;

            rc  = RETURN_FAIL;
            rc2 = MatchFirst((STRPTR)opts[OPT_FILE], &ap.ap);

            while (!rc2)
            {
                if (CheckSignal(SIGBREAKF_CTRL_C))
                {
                    rc2 = ERROR_BREAK;
                    break;
                }

                if (ap.ap.ap_Flags & APF_DIDDIR)
                {
                    /* Exiting a directory, skip this entry */
                    ap.ap.ap_Flags &= ~APF_DIDDIR;
                    indent--;
                    rc2 = MatchNext(&ap.ap);
                    continue;
                }

                isdir = (ap.ap.ap_Info.fib_DirEntryType >= 0);

                if (isdir && opts[OPT_ALL])
                {
                    /* Step into the directory */
                    ap.ap.ap_Flags |= APF_DODIR;
                    indent++;
                }

                if ((ap.ap.ap_Flags & APF_ITSWILD) || opts[OPT_ALL])
                {
                    if (!opts[OPT_QUIET])
                        for (tindent = 0; tindent < indent; tindent++)
                            WriteChars(MSG_BLANKS, TABSIZE);

                    if (!opts[OPT_QUIET])
                    {
                        if (!isdir)
                            WriteChars(MSG_BLANKS, NORMINDENT);

                        if (ap.ap.ap_Info.fib_FileName[0])
                            PutStr(ap.ap.ap_Info.fib_FileName);
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

                if (opts[OPT_ADD])
                {
                    mask = bits | BITFLIP(ap.ap.ap_Info.fib_Protection,0x000f);
                    mask = BITFLIP(mask,0x000f);
                }
                else if (opts[OPT_SUB])
                {
                    mask  = BITFLIP(ap.ap.ap_Info.fib_Protection,0x000f);
                    mask &= ~bits;
                    mask  = BITFLIP(mask,0x000f);
                }
                else
                {
                    mask = BITFLIP(ap.ap.ap_Info.fib_Protection,0x000f);
                    mask = (mask & ~impact) | bits;
                    mask = BITFLIP(mask,0x000f);
                }

                lock = CurrentDir(ap.ap.ap_Current->an_Lock);

                if ((mask != ap.ap.ap_Info.fib_Protection) && (SetProtection(ap.ap.ap_Info.fib_FileName, mask) == DOSFALSE))
                {
                    rc  = RETURN_FAIL;
                    rc2 = IoErr();
                    msg = MSG_SETFAILED;
                    extra = ap.ap.ap_Buf;
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
                    rc2 = MatchNext(&ap.ap);
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
                MatchEnd(&ap);

            SetIoErr(rc2);
        }

        CloseLibrary((struct Library *)DOSBase);
    }
    else {OPENFAIL;}

    return(rc);
}
