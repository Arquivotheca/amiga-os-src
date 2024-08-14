
/* User header file */

struct TMObjectData
  {
  BOOL (* EventFunc)();
  };

typedef struct TMObjectData TMOBJECTDATA;

#include "CAMDPrefs_tm.h"
#include "CAMDPrefs_text.h"

VOID  wbmain(VOID *);
VOID  main(int, char **);
VOID  cleanexit(struct TMData *, int);
UBYTE *getfilename(struct TMData *, UBYTE *, UBYTE *, ULONG, struct Window *, BOOL);

void scan_devices(struct TMData *TMData);

#include <midi/midiprefs.h>

struct DriverNode {
	struct Node			node;					/* list of all nodes		*/
	char				listname[32];			/* name to appear in list	*/
	struct MidiUnitDef 	unit;					/* info about this unit		*/
	char				version_string[8];		/* version string for unit	*/
};

void free_driver_nodes(struct List *);
WORD AlphaInsertNode(struct List *l, struct Node *n);
void SetStrings(struct TMData *TMData, struct DriverNode *dr);
BOOL GetMidiPrefs(char *filename);
BOOL PutMidiPrefs(char *filename, BOOL do_icon);
void update_from_strings( struct TMData *TMData );
