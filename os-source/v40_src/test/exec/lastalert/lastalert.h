/*
 * Constants
 */

#define PROGRAM_NAME "LastAlert"              /* Program name */

#define KICKSTART_VERSION 39                    /* Minimum Kickstart version */
#define WORKBENCH_VERSION 39                    /* Minimum Workbench version */

/*
 * Prototypes
 */

/* from main.c */
void main(int argc,char *argv[]);
void goodbye(int returnCode);

/*
 * Globals
 */

#ifdef MAIN

struct RDArgs *rdArgs=NULL;                     /* dos.library ReadArgs() control structure */

#else

extern struct RDArgs *rdArgs;

#endif /* MAIN */

extern struct ExecBase *SysBase;                /* exec.library base */