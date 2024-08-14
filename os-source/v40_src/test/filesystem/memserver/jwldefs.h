#ifndef DOS_DOS_H
#include <dos/dos.h>
#endif

#ifndef CLIB_DOS_PROTOS_H
#include <clib/dos_protos.h>
#endif

/* set up by SAS startup code */
extern struct DosLibrary *DOSBase;    

/* Current OS version we're partial to... */
#define CURR_MIN_VERSION 37

/* for exiting on Workbench users */
#define EXIT_IF_WB() if (!argc) {exit(RETURN_FAIL);}

/* for EXITing on pre-2.0 users */
#define EXIT_IF_ANCIENT() if(DOSBase->dl_lib.lib_Version < CURR_MIN_VERSION) { Write(Output(),"You need AmigaDOS V37 or better!\n",33); exit(RETURN_FAIL);}

/* for killing Lattice Break-handler */
#define NUKE_SAS_BREAK int CXBRK(void) {return(0);} 
#define REALLY_NUKE_IT int ckkabort(void) {return(0);}

#define USER_HIT_CTRL_C (CheckSignal(SIGBREAKF_CTRL_C))

