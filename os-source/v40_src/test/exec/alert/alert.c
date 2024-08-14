/* Alert.c
 * 
 * By default, does nothing. :-)
 * Optionally:  Pops an Alert(), a fatal alert, crashes the machine,
 *              shows LastAlert, and/or sets LastAlert.
 * 
 */

/* Includes --------------------------------------------- */
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <exec/tasks.h>
#include <exec/alerts.h>
#include <exec/execbase.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/rdargs.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <stdlib.h>
#include <clib/utility_protos.h>


/* Defines ------------------------------------------ */
#define PROGNAME "Alert"
#define ERRMSG_LIBNOOPEN "Couldn't open %s V%ld (or better)!\n"
#define TEMPLATE "ALERT,LAST=LASTALERT/K/N,FATAL/S,CRASH/K/N,SHOWLAST/S"
#define TRASHSIZE 50000

/* Structs ------------------------------ */
struct Opts {
    STRPTR alert;   /* hex alert to pop */
    LONG *last;     /* new value for execbase's lastalert */
    LONG fatal;     /* OR 0x8000000 to specified alert */
    LONG *crash;    /* generate specific crash */
    LONG showlast;  /* just show what's in SysBase->LastAlert[] */
};
struct lookup {
    STRPTR name;
    ULONG  value;
};

#include "lookup.h"   /* alert/english lookup table */

/* Protos ------------------------------------------ */
static VOID GoodBye(int);
static LONG doInit(VOID);
extern VOID WriteChunkyPixels(ULONG,ULONG,ULONG,ULONG,ULONG,ULONG,ULONG);
#include "alert_protos.h"

/* Globals --------------------------------------- */
struct Opts     opts;
struct RDArgs  *rdargs;
struct Library *UtilityBase;
struct Library *GfxBase;            /* try to prevent SAS from opening it.  Growl */
extern struct ExecBase *SysBase;

/* see also lookup.h */


/* main ========================================================================================
        the main attraction/course/event/etc.
*/
VOID main(int argc, UBYTE *argv[]) {
    int i;
    ULONG alert=0, stackSize, tmpVal;
    struct Task *us;
    struct Process *pr;
    BOOL didSomething = FALSE;

    if (!doInit()) {
        GoodBye(RETURN_FAIL);
    }

    /* switch LastAlert value */
    if (opts.last || opts.showlast) {
        didSomething = TRUE;
        for (i = 0; i<4; i++)
            Printf("LastAlert[%ld]: $%08lx\n", i, SysBase->LastAlert[i]);
        if (opts.last) {
            SysBase->LastAlert[3] = *opts.last;
        }
    }

    /* Crash the machine */
    if (opts.crash) {
        didSomething = TRUE;
        us = FindTask(NULL);
        pr = (struct Process *)us;
        switch (*opts.crash) {
            case 1:
                PutStr("Calling graphics.library/WriteChunkyPixels(bad args) without opening graphics!\n");
                Delay(100L);
                WriteChunkyPixels(NULL,NULL,NULL,NULL,NULL,NULL,NULL);
                PutStr("Are we dead yet?\n");
                break;

            case 2:
                PutStr("Adding one to tc_SPReg\n");
                Delay(100L);
                tmpVal = (ULONG)(us->tc_SPReg) + 1UL;
                us->tc_SPReg = (APTR)(tmpVal);
                {
                    auto LONG foo = 3;
                    foo++;
                }
                PutStr("Are we dead yet?\n");
                break;

            case 3:
                PutStr("Dividing by zero...\n");
                Delay(100L);
                {
                    LONG foo = 3L;
                    foo /= 0L;
                }
                PutStr("Are we dead yet?\n");
                break;

            default:
                stackSize = (ULONG)(us->tc_SPUpper - us->tc_SPLower);
                Printf("Trashing stackspace---using %ld bytes of %ld stack!\n", TRASHSIZE, stackSize);
                Delay(100L);
                {
                    auto ULONG trashStack[TRASHSIZE];
                    for (i = 0; i < TRASHSIZE; i++)
                        trashStack[i] = GetUniqueID();
                }
                PutStr("You still there?\n");
                break;
        }
        PutStr("If you see this message, either this program or the OS is broken.\n");
    }

    /* Pop an alert */
    if (opts.alert) {
        STRPTR an_Name;

        didSomething = TRUE;
        if (!xStrToULong(opts.alert, &alert)) {
            alert = engToAlert(opts.alert);
            if (alert == 0xFFFFFFFF) {
                Printf("Bad alert name or number: '%s'\n", opts.alert);
                GoodBye(RETURN_FAIL);
            }
        }
        if (opts.fatal) {
            alert |= AT_DeadEnd;
        }
        an_Name = alertToEng(alert);
        if (!an_Name)
            an_Name = " ";
        Printf("Doing Alert($%08lx) (%s)...\n", alert, an_Name);
        Delay(100L);
        Alert(alert);
    }

    if (!didSomething) {
        Printf("Usage: %s %s\n", PROGNAME, TEMPLATE);
    }

    GoodBye(RETURN_OK);
}

/* GoodBye ===============================================
   Clean-exit routine.
 */
static VOID GoodBye(int rc) {
    if (rdargs) {
        FreeArgs(rdargs);
    }

    if (UtilityBase) {
        CloseLibrary(UtilityBase);
    }

    exit(rc);
}

/* doInit =============================================
 * Open libraries, call ReadArgs() if necessary.
 * Returns TRUE for success, FALSE otherwise.
 */
static LONG doInit(VOID) {

    rdargs = ReadArgs(TEMPLATE, (LONG *)&opts, NULL);
    if (!rdargs) {
        PrintFault(IoErr(), PROGNAME);
        return(FALSE);
    }

    UtilityBase = OpenLibrary("utility.library", 37L);
    if (!UtilityBase) {
        Printf(ERRMSG_LIBNOOPEN, "utility.library", 37L);
        return(FALSE);
    }
    return(TRUE);
}


/* alertToEng =====================================
   give string for alert number
*/
STRPTR alertToEng(ULONG alert) {
    int i;

    for (i = 0; theAlerts[i].name != NULL; i++) {
        if (theAlerts[i].value == alert)
            return(theAlerts[i].name);
    }

    return(NULL);
}

/* engToAlert =====================================================
   gives number for alert name, -1 for bad name
*/
ULONG engToAlert(STRPTR alert) {
    int i;

    if (!alert)
        return(0xFFFFFFFF);

    for (i = 0; theAlerts[i].name != NULL; i++) {
        if (!Stricmp(theAlerts[i].name,alert))
            return(theAlerts[i].value);
    }

    return(0xFFFFFFFF);   /* an invalid alert number */
}


/* xStrToULong ===============================================
   Converts a hex string to a ULONG.
   Inputs:
         STRPTR - string of form $nnnnnnnn, 0xnnnnnnnn,
                  Xnnnnnnnn, or xnnnnnnnn.
                  There must be 1 to 8 valid hex digits
                  after the introductory character(s).
                  String MUST be null-terminated!
                  String won't be altered.
         ULONG *- pointer to ULONG in which you want the result.
                  ULong will be altered most of the time, whether
                  this function succeeds or fails.

   Returns:
         TRUE if conversion succeeded.  ULONG contains converted number.
         FALSE otherwise (number too large, not hex, whatever)
               ULONG contains garbage.
 */
BOOL xStrToULong(STRPTR str, ULONG *result) {
    STRPTR s = str, tmp;
    int len=0, shifter;

    /* Fail if NULL args were given */
    if (!str || !result)
        return(FALSE);

    /* look at str[1] */
    s++;

    /* position s to point at the first hex digit */    
    switch (*str) {
        case '0':
            if ((*s != 'x') && (*s != 'X'))
                return(FALSE);
            s++;
            break;
        case '$':
        case 'x':
        case 'X':
            break;
        default:
            return(FALSE);
            break;
    }

    /* find out how many hex digits we have */
    tmp = s;
    while (*tmp++)
        len++;

    /* fail if we have too many or too few */
    if ((len > 8) || (len == 0))
        return(FALSE);

    /* zap out whatever was in *result */
    *result = 0UL;

    /* turn string into a number */
    while (*s) {
        shifter = (--len) * 4;
        switch (*s++) {
            case '0':
                break;
            case '1':
                *result += (1UL<<shifter);
                break;
            case '2':
                *result += (2UL * (1UL<<shifter));
                break;
            case '3':
                *result += (3UL * (1UL<<shifter));
                break;
            case '4':
                *result += (4UL * (1UL<<shifter));
                break;
            case '5':
                *result += (5UL * (1UL<<shifter));
                break;
            case '6':
                *result += (6UL * (1UL<<shifter));
                break;
            case '7':
                *result += (7UL * (1UL<<shifter));
                break;
            case '8':
                *result += (8UL * (1UL<<shifter));
                break;
            case '9':
                *result += (9UL * (1UL<<shifter));
                break;
            case 'a':
            case 'A':
                *result += (10UL * (1UL<<shifter));
                break;
            case 'b':
            case 'B':
                *result += (11UL * (1UL<<shifter));
                break;
            case 'c':
            case 'C':
                *result += (12UL * (1UL<<shifter));
                break;
            case 'd':
            case 'D':
                *result += (13UL * (1UL<<shifter));
                break;
            case 'e':
            case 'E':
                *result += (14UL * (1UL<<shifter));
                break;
            case 'f':
            case 'F':
                *result += (15UL * (1UL<<shifter));
                break;
            default:
                /* fail if we find a bad digit */
                return(FALSE);
                break;
        }
    }
    return(TRUE);
}
