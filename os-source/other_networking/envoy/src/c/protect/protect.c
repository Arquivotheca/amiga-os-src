
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .  | || the authors:                             BBS: (919) 382-8265    */
/* | o  | ||   Dave Baker      Alan Beale         Jim Cooper                 */
/* |  . |//    Jay Denebeim    Bruce Drake        Gordon Keener              */
/* ======      John Mainwaring Andy Mercier       Jack Rouse                 */
/* John Toebes     Mary Ellen Toebes  Doug Walker   Mike Witcher */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**     (C) Copyright 1989 Commodore-Amiga, Inc.
 **         All Rights Reserved
 **/

/*---------------------------------------------------------------------------*/
/* Command: Protect                                                          */
/* Author:  Doug Walker                                                      */
/* Change History:                                                           */
/* Date    Person  Action                                                    */
/* -------  ------------- -----------------                                  */
/* 19MAR89  John Toebes   Initial Creation                                   */
/* 28MAR89  Doug Walker   Filled in body                                     */
/* 06NOV89  Doug Walker   Revved for dos.library 36.38                       */
/* MAR90  Doug Walker   Added ALL option, wildcards                          */
/* MAY90  Doug Walker   Added additionall error checking                     */
/* JLY92  Greg Miller   Added Envoy USER, GROUP, OTHER                       */
/* NOV92  Greg MIller   USER, GROUP, OTHER non-exclusive now                 */
/* NOV92  Greg Miller   CLONE option added                                   */
/* MAR93  Greg Miller   Fixed bug w/ not being able to reset 's' bit         */
/* Notes:                                                                    */
/*---------------------------------------------------------------------------*/

#include "internal/commands.h"
#include "protect_rev.h"

#define MSG_BADFLAG   "Invalid flag - must be one of SPARWED\n"
#define MSG_SETFAILED "Can't set protection for %s\n"
#define MSG_NOMEM     "No memory\n"
#define MSG_DONE      "...Done\n"
#define MSG_DIR       " (dir)"
#define MSG_BLANKS    "        "
#define MSG_ADDSUB    "Can't specify both ADD (+) and SUB (-)\n"
#define MSG_BADCLONE  "Can't specify flags and do CLONE\n"


#define TEMPLATE    "FILE/A,FLAGS,ADD/S,SUB/S,ALL/S,QUIET/S,USER/S,GROUP/S,OTHER/S,CLONE/S" CMDREV
#define OPT_FILE    0
#define OPT_FLAGS   1
#define OPT_ADD     2
#define OPT_SUB     3
#define OPT_ALL     4
#define OPT_QUIET   5
#define OPT_USER    6
#define OPT_GROUP   7
#define OPT_OTHER   8
#define OPT_CLONE   9
#define OPT_COUNT   10

#define NORMINDENT  3
#define TABSIZE     5

#define ALL_PROT_BITS (0x00000080|FIBF_SCRIPT|FIBF_PURE|FIBF_ARCHIVE|\
                       FIBF_READ|FIBF_WRITE|FIBF_EXECUTE|FIBF_DELETE|\
                       FIBF_GRP_READ|FIBF_GRP_WRITE|FIBF_GRP_EXECUTE|FIBF_GRP_DELETE|\
                       FIBF_OTR_READ|FIBF_OTR_WRITE|FIBF_OTR_EXECUTE|FIBF_OTR_DELETE)

#define BITFLIP(x,m) (( (x) & ~((LONG)m)) | ( ~(x) & ((LONG)m) ))

#define ANCHORNAMELEN 255
#define ANCHORSIZE    (sizeof(struct AnchorPath) + ANCHORNAMELEN)

/* Macro to get longword-aligned stack space for a structure    */
/* Uses ANSI token catenation to form a name for the char array */
/* based on the variable name, then creates an appropriately    */
/* typed pointer to point to the first longword boundary in the */
/* char array allocated.                                        */
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
    int rc, rc2, apinit, indent, tindent, isdir;
    LONG mask, umask, tmask;
    LONG oldbits;
    char *msg, *curflag;
    LONG opts[OPT_COUNT];
    BPTR lock;
    struct RDArgs *rdargs;
    D_S(ap, struct uAnchorPath);


    rc = RETURN_FAIL;
    if ((DOSBase = (struct DosLibrary *) OpenLibrary(DOSLIB, DOSVER)))
    {
        memset((char *) opts, 0, sizeof(opts));
        if (!(rdargs = ReadArgs(TEMPLATE, &opts[0], NULL)))
        {
            PrintFault(IoErr(), NULL);
        }
        else
        {
            if (!(opts[OPT_GROUP] || opts[OPT_OTHER]))
                opts[OPT_USER] = TRUE;
            rc = rc2 = apinit = 0;
            msg = NULL;

            umask = 0L;
            if (opts[OPT_FLAGS])
            {
                if (opts[OPT_CLONE])
                {
                    msg = MSG_BADCLONE;
                    rc = RETURN_FAIL;
                    goto ERRORCASE;
                }
                for (curflag = (char *) opts[OPT_FLAGS]; *curflag; curflag++)
                {
                    if ((opts[OPT_USER]) || (!(opts[OPT_GROUP] || opts[OPT_OTHER])))
                    {
                        switch (*curflag)
                        {
                            case 'h':
                            case 'H':
                                umask |= 0x80;
                                break;
                            case 's':
                            case 'S':
                                umask |= FIBF_SCRIPT;
                                break;
                            case 'p':
                            case 'P':
                                umask |= FIBF_PURE;
                                break;
                            case 'a':
                            case 'A':
                                umask |= FIBF_ARCHIVE;
                                break;
                            case 'r':
                            case 'R':
                                umask |= FIBF_READ;
                                break;
                            case 'w':
                            case 'W':
                                umask |= FIBF_WRITE;
                                break;
                            case 'e':
                            case 'E':
                                umask |= FIBF_EXECUTE;
                                break;
                            case 'd':
                            case 'D':
                                umask |= FIBF_DELETE;
                                break;
                            case '+':
                                opts[OPT_ADD] = 1;
                                break;
                            case '-':
                                opts[OPT_SUB] = 1;
                                break;
                            default:
                                msg = MSG_BADFLAG;
                                rc = RETURN_FAIL;
                                goto ERRORCASE;
                        }
                    }
                    if (opts[OPT_GROUP])
                    {
                        switch (*curflag)
                        {
                            case 'r':
                            case 'R':
                                umask |= FIBF_GRP_READ;
                                break;
                            case 'w':
                            case 'W':
                                umask |= FIBF_GRP_WRITE;
                                break;
                            case 'e':
                            case 'E':
                                umask |= FIBF_GRP_EXECUTE;
                                break;
                            case 'd':
                            case 'D':
                                umask |= FIBF_GRP_DELETE;
                                break;
                            case '+':
                                opts[OPT_ADD] = 1;
                                break;
                            case '-':
                                opts[OPT_SUB] = 1;
                                break;
                            default:
                                msg = MSG_BADFLAG;
                                rc = RETURN_FAIL;
                                goto ERRORCASE;
                        }
                    }
                    if (opts[OPT_OTHER])
                    {
                        switch (*curflag)
                        {
                            case 'r':
                            case 'R':
                                umask |= FIBF_OTR_READ;
                                break;
                            case 'w':
                            case 'W':
                                umask |= FIBF_OTR_WRITE;
                                break;
                            case 'e':
                            case 'E':
                                umask |= FIBF_OTR_EXECUTE;
                                break;
                            case 'd':
                            case 'D':
                                umask |= FIBF_OTR_DELETE;
                                break;
                            case '+':
                                opts[OPT_ADD] = 1;
                                break;
                            case '-':
                                opts[OPT_SUB] = 1;
                                break;
                            default:
                                msg = MSG_BADFLAG;
                                rc = RETURN_FAIL;
                                goto ERRORCASE;
                        }
                    }
                }
                if (umask == 0L)
                {
                    msg = MSG_BADFLAG;
                    rc = RETURN_FAIL;
                    goto ERRORCASE;
                }
            }

            if (opts[OPT_ADD] && opts[OPT_SUB])
            {
                msg = MSG_ADDSUB;
                rc = RETURN_FAIL;
                goto ERRORCASE;
            }

            if (opts[OPT_SUB])
                umask = (((~umask) & ALL_PROT_BITS) | ~ALL_PROT_BITS);


            memset(ap, 0, sizeof(struct uAnchorPath));
            ap->ap.ap_Flags = APF_DOWILD;
            ap->ap.ap_Strlen = ANCHORNAMELEN;
            ap->ap.ap_BreakBits = SIGBREAKF_CTRL_C;

            if (rc2 = MatchFirst((char *) opts[OPT_FILE], &ap->ap))
            {
                rc = RETURN_FAIL;
                if (rc2 != ERROR_NO_MORE_ENTRIES)
                    PrintFault(rc2, NULL);
                goto ERRORCASE;
            }

            apinit = 1;
            indent = 0;

            do
            {
                if (CheckSignal(SIGBREAKF_CTRL_C))
                {
                    PrintFault(ERROR_BREAK, NULL);
                    rc = RETURN_FAIL;
                    rc2 = ERROR_BREAK;
                    goto ERRORCASE;
                }

                if (ap->ap.ap_Flags & APF_DIDDIR)
                {
                    /* Exiting a directory, skip this entry */
                    ap->ap.ap_Flags &= ~APF_DIDDIR;
                    indent--;
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
                    for (tindent = 0; tindent < indent; tindent++)
                    {
                        if (!opts[OPT_QUIET])
                            WriteChars(MSG_BLANKS, TABSIZE);
                    }
                    if (!opts[OPT_QUIET])
                    {
                        if (!isdir)
                            WriteChars(MSG_BLANKS, NORMINDENT);
                        PutStr(ap->ap.ap_Info.fib_FileName);
                        if (isdir)
                            PutStr(MSG_DIR);
                    }
                }
                else
                    tindent = -1;       /* We'll use tindent >= 0 below as a
                                         * flag */

                mask = umask;

                /* Bottom four bits (RWED) are reverse-sense */
                /* As are now the Group and Other bits */
                oldbits = BITFLIP(ap->ap.ap_Info.fib_Protection, 0xff0f);

                tmask = 0;
                if (opts[OPT_GROUP])   /* If 'group' specifier, we modify
                                         * group & hspa bits */
                    tmask |= 0xf00;
                if (opts[OPT_OTHER])   /* If 'other' specifier, we modify
                                         * other & hspa bits */
                    tmask |= 0xf000;
                if (opts[OPT_USER])    /* If 'user' specifier, we modify user
                                         * & hspa bits */
                    tmask |= 0xff;
                tmask = ~tmask & (ALL_PROT_BITS);
                mask |= (oldbits & tmask);

                if (opts[OPT_CLONE])
                {
                    if (opts[OPT_GROUP])        /* If CLONE and GROUP, copy
                                                 * USER->GROUP */
                        mask |= ((oldbits & 0xF) << 8);
                    if (opts[OPT_OTHER])        /* If CLONE and OTHER, copy
                                                 * USER->OTHER */
                        mask |= ((oldbits & 0xF) << 12);
                    if (opts[OPT_USER]) /* A possible, albeit senseless
                                         * combination */
                        mask |= ((oldbits & 0xF));
                }

                if (opts[OPT_ADD])
                    mask |= oldbits;
                if (opts[OPT_SUB])
                    mask &= oldbits;

                /* Bottom four bits (RWED), Group, and Other are reverse-sense */
                mask = BITFLIP(mask, 0xff0f);

                lock = CurrentDir(ap->ap.ap_Current->an_Lock);

                if (SetProtection(ap->ap.ap_Info.fib_FileName, mask) == DOSFALSE)
                {
                    rc = RETURN_FAIL;
                    rc2 = IoErr();
                    msg = MSG_SETFAILED;
                }

                if ((tindent >= 0) && !opts[OPT_QUIET])
                {
                    /*
                     * If we put out the first part of the msg, put out the
                     * rest now
                     */
                    PutStr(MSG_DONE);
                }

                CurrentDir(lock);
            }
            while (!rc && !(rc2 = MatchNext(&ap->ap)));

            if (rc2 == ERROR_NO_MORE_ENTRIES)
                rc2 = 0;
            else if (!rc)
            {
                rc = RETURN_FAIL;
                PrintFault(rc2, NULL);
            }

          ERRORCASE:

            if (msg)
                VPrintf(msg, opts);

            if (rdargs)
                FreeArgs(rdargs);

            if (apinit)
                MatchEnd((struct AnchorPath *) ap);

            SetIoErr(rc2);
        }

        CloseLibrary((struct Library *) DOSBase);
    }
    else
    {
        OPENFAIL;
    }
    return (rc);
}
