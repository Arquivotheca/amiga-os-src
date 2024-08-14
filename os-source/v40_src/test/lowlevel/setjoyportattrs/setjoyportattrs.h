/*
 * Constants
 */

#define PROGRAM_NAME "SetJoyPortAttrs"          /* Program name */

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

struct Library *LowLevelBase=NULL;              /* lowlevel.library base */

#else

extern struct RDArgs *rdArgs;

extern struct Library *LowLevelBase;

#endif /* MAIN */
