/* $Id: other_protos.h,v 1.2 93/03/31 15:08:59 peter Exp $ */

/* Other prototypes used internally by PlayerPrefs */

VOID sigPPTask(struct PlayerPrefsBase *PlayerPrefsBase, int val);
int launchPPTask(char *taskname,LONG pri ,APTR prog, ULONG stacksize,
             struct PlayerPrefsBase *PlayerPrefsBase);

int CDG_Open(struct CDGPrefs *cdp);
void CDG_Close( void );
int UnpackSprites(struct CDGPrefs *cdgp);
int UpdateDisplay( void );
void ShuffleGrid( LONG alreadyshuffled );
void UnShuffleGrid( void );
void UpdatePlayTime( void );
void SetTrackCounter(int now);
VOID DisplayGrid( void );
void InitTrackCounter( void );
