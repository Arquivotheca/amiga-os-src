
                                                            Jerry Horanoff
                                                            2/10/93
                        P R E L I M I N A R Y !

                       C O N F I D E N T I A L !


                    Specification subject to change!



    The "nonvolatile.library" is a library that will attempt to save a
small amount of information to a non-volatile device.  Non-volatile devices
are:

Floppy Drives
Hard Drives
Credit Card
Non-Volatile RAM

    There are two different methods for storing nonvolatile entries - on
disk and in real NV RAM (a credit card is considered a disk).  The library
always prefers to store entries on disk if possible.  NV RAM is always the
last choice.

When an NV entry is saved on disk, it is saved in file-format.  A directory
in the root directory of a device will be created called "nonvolatile". 
Within this directory application directories are created.  Within those
directories, nonvolatile entry files are created.  When saving to NV RAM,
the NV item is saved in a list format within the NV RAM.

    


The nonvolatile.library contains 6 functions:

struct NVInfo *GetNVInfo(void);
char          *GetNVList(char *AppName);
void          *GetCopyNV(char *AppName, char *ItemName);
void           FreeNVData(void *Data);
UWORD          StoreNV(char *AppName, char *ItemName, void *Data, ULONG Length);
UWORD          DeleteNV(char *AppName, char *ItemName);


********************************************************************************

struct NVInfo *GetNVInfo(void)

struct NVInfo {

    ULONG MaxStorage;
    ULONG FreeStorage;
    ULONG pad[6];
    };

    This function will return information about the size of the
nonvolatile device, and the amount of free space available.

    The first thing that an application should do (if it wishes to use
nonvolatile.library) is to do a GetNVInfo() call.  If there is not enough
NV storage available to save 1 game, the game warns the user to free
storage before beginning the game if he/she intends on saving information.

    After a game is saved, a check for free space should immediately follow
and a warning should be given if there is not enough space for another game
to be saved.  The user can then quit, free up space, and continue at the
point of the last save.

    This approach will hopefully eliminate the need for the application to
do any complex error checking, requesting, disk formatting, etc....

    As a rule, the maximum entry size should not be more than 25% of
NVInfo.MaxStorage.

    To free the data returned by this function, use FreeNVData().



********************************************************************************

char *GetNVList(char *AppName)

    This function will return a pointer to a list of NULL terminated
strings of Items saved by this application or NULL if out of memory.  A
NULL string will terminate the list.  To free this list, use FreeNVData().




********************************************************************************

void *GetCopyNV(char *AppName, char *ItemName)

    GetCopyNV will search nonvolatile disk devices for the nonvolatile
directory.  If the search succeeds, The search then begins for a
non-volatile entry with matching application and item names.  If found, a
pointer to a copy of this entry will be returned.  If an entry cannot be
found on a disk device, NV RAM will be searched (if it exists).  If the
search fails, a NULL pointer will be returned.  To free the item's data,
use FreeNVData().



********************************************************************************

void   FreeNVData(void *Data)

    FreeNVData will free the memory for the data returned by GetNVInfo(),
GetNVList(), or GetCopyNV().



********************************************************************************

UWORD  StoreNV(char *AppName, char *ItemName, void *Data, ULONG Length)

    When an application wants to create a nonvolatile entry, the StoreNV
call is used.  Along with the application and item names, a pointer to the
data to be stored and the length of the data are passed in.  Application
and item names should be short, but unique.  The longer the name, the more
memory it takes to store the entry.  Names should follow disk filename
rules. 

    Entry size needs will vary from application to application, but
compactness of data is essential.  Simple arcade-like games that may want
to save high scores should only use small amounts of memory.  Example: (3
letter initials + 3 byte score) times 10 high scores = 60 bytes.

    If the GetNVInfo() function reveals that a significant amount of
nonvolatile storage is available, your entries can be much larger.

    The StoreNV() function returns four possible values:

    0               - Everything went ok.
    NVERR_BADNAME   - Bad application/item name.
    NVERR_WRITEPROT - Disk, file, or directory is write protected.
    NVERR_FAIL      - Could not write the data (device full or not found).
    NVERR_FATAL     - Something horrible has happened (read/write error on
                      disk or something nasty like that).


********************************************************************************

BOOL DeleteNV(char *AppName, char *ItemName);

    DeleteNV will delete an existing nonvolatile entry.  If the entry was
deleted, TRUE is returned.  If the entry does not exist, FALSE is returned.




********************************************************************************

    Just like the language selection will appear if the user presses the B
button during the startup animation, the A button will bring up a
file requester for NV RAM allowing the user to selectively delete items.




Example 1 (saving high score information):


void FreeStorageWarning(int MaxStore) {

 struct NVInfo *Info;

    if ((Info = GetNVInfo())->FreeStorage < MaxStore) {

        /* Notify user to free up at least (MaxStore          */
        /* - Info->FreeStorage) bytes of nonvolatile storage. */
        /* If he/she wishes to save high-score information.   */
        }

    FreeNVData(Info);
    }


void Game(void) {

 struct ScoreEntry = {                   /* Each high-score entry is 6 bytes */

        char  Name[3];
        UBYTE Score[3];
        }

 struct ScoreEntry ScoreTable[10];       /* Allow 10 high-score entries */


    if (Scores = GetCopyNV("AppName", "HighScores")) {

        /* Copy data to ScoreTable[] */

        FreeNVData(Scores);
        }

    else {

        FreeStorageWarning(sizeof(ScoreTable));             /* Make sure there is enough space to save a high score table */
                                                            /* (or we could just blow it off if the save fails)           */
        /* Initialize ScoreTable[] to be empty */
        }


    /* Play the game */

    if (Player_Did_Real_Well) {     /* Game over, save high scores */

        /* Modify high score list. */

        switch (StoreNV("AppName", "HighScores", ScoreTable, sizeof(ScoreTable[]))) {

            /* You can blow off this error checking if you'd like.  After all, it's just         */
            /* a high-score list.  If NVERR_FAIL is received, then this must be a different      */
            /* disk than when we started the game (since we checked that there was enough room). */

            case NVERR_WRITEPROT: /* Device is write protected.  Prompt Retry/Cancel */ break;
            case NVERR_FAIL:      /* Please insert correct disk. Prompt Retry/Cancel */ break;
            case NVERR_FATAL:     /* Fatal error, could not save.                    */ break;
            }
        }
    }





Example 2 (saving multiple game positions):



#define MAXGAMESAVESIZE  100        /* 100 bytes */


void Game(void) {

 struct GameData *GameData;                                 /* Structure that application uses to organize game-save data */
 char            *DirList;                                  /* Pointer to list of existing game-save items                */

    FreeStorageWarning(MAXGAMESAVESIZE);                    /* Make sure there is enough space to save one game */

    do {

        if (LoadGame) {

            DirList = GetNVList("AppName");

            /* Display existing saves of this game */
            /* and allow user to pick one.         */

            if (GameData = GetCopyNV("AppName", Name) {

                /* Set up the game based on this information */

                FreeNVData(GameData);
                }

            else /* Could not load data (should not be possible if item exists) */

            FreeNVData(DirList);
            }


        /* Play the Game */


        if (SaveGame) {

            DirList = GetNVList("AppName");

            /* Display existing saves of this game */

            /* Pick one, or enter a new one        */

            switch (StoreNV("AppName", Name, GameData, Length)) {

                case NVERR_WRITEPROT: /* Device is write protected   */ break;
                case NVERR_BADNAME:   /* Improper name               */ break;
                case NVERR_FAIL:      /* Please insert correct disk  */ break;
                case NVERR_FATAL:     /* Fatal error, could not save */ break;
                }

            FreeStorageWarning(); /* Make sure there is still enough space to save one game */

            FreeNVData(DirList);
            }

        } while (!QuitGame);
    }




The following by: Jim Barkley

THESE FUNCTIONS ARE NOT FINALIZED.  SPECIFICATION SUBJECT TO CHANGE!


TABLE OF CONTENTS

nonvolatile.library/DeleteNV
nonvolatile.library/FreeNVData
nonvolatile.library/GetCopyNV
nonvolatile.library/GetNVInfo
nonvolatile.library/GetNVList
nonvolatile.library/StoreNV
nonvolatile.library/DeleteNV                     nonvolatile.library/DeleteNV

   NAME
    DeleteNV - Remove an entry from nonvoltatile storage

   SYNOPSIS
    Success = DeleteNV (AppName, ItemName)
    d0          a0       a1

    BOOL DeleteNV (char *, char *)

   FUNCTION
    Searches the nonvolatile storage for the indicated entry and removes
    it.

   INPUTS
    AppName - NULL terminated string identifing the application that
          created the data.
    ItemName - NULL terminated string uniquely identifing the data
           within the application.

   RESULT
    Success - TRUE will be returned if the entry is found and deleted.
          If the entry is not found FALSE will be returned

nonvolatile.library/FreeNVData                 nonvolatile.library/FreeNVData

   NAME
    FreeNVData - Release the memory allocated by a function of this
             library.

   SYNOPSIS
    FreeNVData (Data)
           a1

    VOID FreeNVData (APTR)

   FUNCTION
    Frees a block of memory that was allocated by any of the following:
    GetCopyNV(), GetNVInfo(), GetNVList().

   INPUTS
    Data - Pointer to the memory block to be freed.

   RESULT
    NONE

   SEE ALSO
    GetCopyNV(), GetNVInfo(), GetNVList()

nonvolatile.library/GetCopyNV                   nonvolatile.library/GetCopyNV

   NAME
    GetCopyNV - Returns an items stored in nonvolatile storage.

   SYNOPSIS
    Data = GetCopyNV (AppName, ItemName)
    d0        a0       a1

    APTR GetCopyNV (char *, char *)

   FUNCTION
    Seaches the nonvolatile storage for the indicated AppName and
    ItemName.  A pointer to a copy of this data will be returned.

   INPUTS
    AppName - NULL terminated string indicating the application name
          to be found.
    ItemName - NULL terminated string indicated the item within the
           application to be found.

   RESULT
    Data - Pointer to a copy of data found in the nonvolatile storage
           assocated with AppName and ItemName.  NULL will be returned
           if there is insufficient memory, or the AppName/ItemName does
           not exist.

   SEE ALSO
    FreeNVData()

nonvolatile.library/GetNVInfo                   nonvolatile.library/GetNVInfo

   NAME
    GetNVInfo - Reports information on the current nonvolatile storage

   SYNOPSIS
    Information = GetNVInfo ()
    d0

    struct NVInfo * GetNVList (VOID)

   FUNCTION
    Finds the user's preferred nonvolatile device and reports information
    about it.

    When a progrm first begins it should use this function to check if
    there is enough free space to store whatever may be stored (ie a
    saved game, or a high score table).  If there is not, the user
    should be notified so they are not left in a lurch when they want to
    save something.

    The check and possible user notification should be redone after
    every save.

   INPUTS
    NONE

   RESULT
    Information - Pointer to an NVInfo structure.

   SEE ALSO
    FreeNVData()

nonvolatile.library/GetNVList                   nonvolatile.library/GetNVList

   NAME
    GetNVList - Returns a list of the items stored in nonvolatile
            storage.

   SYNOPSIS
    List = GetNVList (AppName)
    d0        a0

    char * GetNVList (char *)

   FUNCTION
    Returns a list of NULL terminated strings.  This list is all of the
    ItemNames that have been stored in nonvolatile storage for the given
    AppName.  The end of the list is a NULL string (ie the list ends
    with two consecutive NULLs).

   INPUTS
    AppName - NULL terminated string indicating the application name
          to be matched.

   RESULT
    List - NULL terminated list of NULL terminated string.  A NULL will
           be returned if there is insufficient memory, or there are no
           entries in the nonvolatile storage for the AppName.

   SEE ALSO
    FreeNVData()

nonvolatile.library/StoreNV                       nonvolatile.library/StoreNV

   NAME
    StoreNV - Store data in nonvolatile storage.

   SYNOPSIS
    Error = StoreNV (AppName, ItemName, Data, Length)
    d0       a0   a1        a2    d0

    UWORD StoreNV (char *, char *, APTR, ULONG)

   FUNCTION
    Saves some data in nonvolatile storage.  The data is tagged with
    AppName and ItemName so it can be retrieved later.  No single
    item should be larger than one fourth of the maximum storage as
    returned by GetNVInfo().

    The string, AppName and ItemName, should be short, but descriptive.
    They need to be short since nonvolatile storage for a stand alone
    game system is limited.  The game system allows the user to
    selectively remove entries from storage, so the string should be
    desriptive.

   INPUTS
    AppName - NULL terminated string identifying the application
          creating the data.
    ItemName - NULL terminated string uniquely identifying the data
           within the application.
    Data - Pointer to the memory block to be stored.
    Length - Number of bytes to be stored.

   RESULT
    Error:
    0 - Data stored, no error.
    NVERR_BADNAME - Error in AppName, or ItemName.
    NVERR_FAIL - Failure in writing data (nonvolatile storage full, or
       write protected).
    NVERR_FATAL - Fatal error when accessing nonvolatile storage,
       possible loss of previously saved nonvolatile data.

   SEE ALSO
    GetCopyNV(), GetNVInfo()

