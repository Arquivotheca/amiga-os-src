
/* User header file */

/* NewGadget/NewMenu UserData structure */
 
struct TMObjectData
  {
  BOOL (* EventFunc)();	/* Function to call */
  };

typedef struct TMObjectData TMOBJECTDATA;

#include "db_tm.h"	/* Toolmaker header file */
#include "db_text.h"	/* User text header file */

/* Function prototypes */
 
VOID  wbmain(VOID *);	/* for DICE compatibility */
VOID  main(int, char **);
VOID  cleanexit(struct TMData *, int);
UBYTE *getfilename(struct TMData *, UBYTE *, UBYTE *, ULONG, struct Window *, BOOL);

