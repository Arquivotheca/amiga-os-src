/* global.c */
#include "ram_includes.h"
#include "ram.h"

struct Library *SysBase;
struct DosLibrary *DOSBase;
struct Library *UtilityBase;

BPTR lock_list;
struct node *root;
struct node *current_node;

LONG path_pos;

LONG spaceused;
LONG fileerr;
struct DeviceList *volumenode;

struct MsgPort *MyPort;
struct MsgPort *MyReplyPort;
struct MsgPort *timerport;

struct MinList orphaned;
struct MinList FreeMessages;
