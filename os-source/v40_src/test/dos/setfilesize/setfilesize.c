/* SetFileSize.c
 * 
 * Test for dos.library/SetFileSize()
 * 
 * 
 */

/* Includes --------------------------------------------- */
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <stdlib.h>
#include <string.h>


/* Defines ------------------------------------------ */
#define PROGNAME "SetFileSize"
#define ERRMSG_LIBNOOPEN "Couldn't open %s V%ld (or better)!\n"
#define TEMPLATE "FILE/A,SIZE/A/N,MODE/K,SEEKTO/K/N,SEEKMODE/K"
#define OFFSET_BAD 55     /* valid offsets are -1,0,1 */
#define ERRMSG_BADMODE "Bad SEEKMODE '%s', must be one of {(B)egin, (E)nd, (C)urrent}!"
#define SETFILESIZE_ERROR -1L
#define SEEK_ERROR        -1L

/* Structs ------------------------------ */
struct Opts {
    STRPTR file;            /* file to resize */
    LONG *size;             /* new size */
    STRPTR mode;            /* mode (beginning, current, end) */
    LONG *seekTo;           /* seek to this point before resize */
    STRPTR seekMode;        /* mode for seek */
};

/* Protos ------------------------------------------ */
static VOID GoodBye(int);
static LONG doInit(VOID);
static int  getMode(STRPTR);
static STRPTR modeToEnglish(int);

/* Globals --------------------------------------- */
struct Opts     opts;
struct RDArgs  *rdargs;
BPTR            fh;
extern struct DosLibrary *DOSBase;

VOID main(int argc, UBYTE *argv[]) {
    LONG mode, newSize;

    if (!argc) {
        GoodBye(RETURN_FAIL);   /* no workbench usage */
    }

    if (!doInit()) {
        GoodBye(RETURN_FAIL);
    }

    mode = getMode(opts.mode);
    if (mode == OFFSET_BAD) {
        Printf(ERRMSG_BADMODE, opts.mode);
        GoodBye(RETURN_FAIL);
    }

    fh = Open(opts.file, MODE_OLDFILE);
    if (!fh) {
        Printf("Can't open '%s', creating it...\n", opts.file);
        fh = Open(opts.file, MODE_NEWFILE);
        if (!fh) {
            PrintFault(IoErr(), PROGNAME);
            Printf("Can't create file '%s'!\n", opts.file);
            GoodBye(RETURN_FAIL);
        }
    }

    if (opts.seekTo) {
        int seekMode;
        LONG err;
        
        seekMode = getMode(opts.seekMode);
        if (seekMode == OFFSET_BAD) {
            Printf(ERRMSG_BADMODE, opts.seekMode);
            GoodBye(RETURN_FAIL);
        }
        err = Seek(fh, *opts.seekTo, seekMode);
        if (err == SEEK_ERROR) {
            PrintFault(IoErr(), PROGNAME);
            PutStr("Error on Seek()!\n");
            GoodBye(RETURN_ERROR);
        }
        else {
            Printf("Successful Seek() from %ld to %ld from offset_%s\n",
                    err, *opts.seekTo, modeToEnglish(seekMode));
        }
    }


    Printf("Calling SetFileSize($%08lx, %ld, %s)...\n",
            fh, *opts.size, modeToEnglish(mode));

    /* ACTUAL WORK IS DONE HERE ........ */
    newSize = SetFileSize(fh, *opts.size, mode);

    if (newSize == SETFILESIZE_ERROR) {
        PrintFault(IoErr(), PROGNAME);
        Printf("Error %ld on SetFileSize!\n", newSize);
        GoodBye(RETURN_FAIL);
    }
    else {
        Printf("Success.  Position of new EOF: %ld\n", newSize);
    }

    GoodBye(RETURN_OK);
}

/* modeToEnglish =============================================
   returns string describing offset.
 */
static STRPTR modeToEnglish(int mode) {
    switch (mode) {
        case OFFSET_BEGINNING:
            return("beginning");
            break;
        case OFFSET_END:
            return("end");
            break;
        case OFFSET_CURRENT:
            return("current");
            break;
        default:
            break;
    }
    return("invalid");
}

/* getMode ======================================================
   looks at first character of string provided, returns 
   appropriate OFFSET_#?.  NULL string returns OFFSET_BEGINNING.
 */
static int  getMode(STRPTR amode) {
    if (!amode) {
        return(OFFSET_BEGINNING);
    }
    switch (*amode) {
        case 'b':
        case 'B':
            return(OFFSET_BEGINNING);
            break;
        case 'C':
        case 'c':
            return(OFFSET_CURRENT);
            break;
        case 'e':
        case 'E':
            return(OFFSET_END);
            break;
        default:
            return(OFFSET_BAD);
            break;
    }
    return(OFFSET_BAD);
}

/* GoodBye ===============================================
   Clean-exit routine.
 */
static VOID GoodBye(int rc) {
    if (rdargs) {
        FreeArgs(rdargs);
    }

    if (fh) {
        Close(fh);
    }

    exit(rc);
}

/* doInit =============================================
 * Open libraries, call ReadArgs() if necessary.
 * Returns TRUE for success, FALSE otherwise.
 */
static LONG doInit(VOID) {

    if (DOSBase->dl_lib.lib_Version < 37L) { 
        Write(Output(), "You need AmigaDOS V37 or better!\n", 33); 
        return(FALSE);
    }

    rdargs = ReadArgs(TEMPLATE, (LONG *)&opts, NULL);
    if (!rdargs) {
        PrintFault(IoErr(), PROGNAME);
        return(FALSE);
    }

    return(TRUE);
}

