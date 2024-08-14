
/* User header file */

#include "std_headers.h"

/* NewGadget/NewMenu UserData structure */

struct TMObjectData
  {
  BOOL (* EventFunc)();	/* Function to call */
  };

typedef struct TMObjectData TMOBJECTDATA;

#include "capture_tm.h"	/* Toolmaker header file */
#include "capture_text.h"	/* User text header file */

/* Function prototypes */

VOID  wbmain(VOID *);	/* for DICE compatibility */
VOID  main(int, char **);
VOID  cleanexit(struct TMData *, int);
UBYTE *getfilename(struct TMData *, UBYTE *, UBYTE *, ULONG, struct Window *, BOOL);

void HandleMidi( struct TMData *TMData );
void ClearBuffer( void );
void SendMIDI( struct TMData *TMData );
void SaveMIDI( struct TMData *TMData, char *filename );
void LoadMIDI( struct TMData *TMData, char *filename );
ULONG GetMidiLinkAttrs(struct MidiLink *mi, Tag tag, ...);
      
