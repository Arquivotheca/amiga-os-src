/* Prototypes for functions defined in
screens.c
 */

extern BYTE loresBorders[8];

extern ULONG screenFlagsPackTable[10];

struct Screen * openScreen(struct NewScreen * ns);

extern struct IScreenModePrefs DefaultScreenMode;

struct Screen * openScreenTagList(struct NewScreen * ns,
                                  struct TagItem * tags);

static struct Screen * realOpenScreenTagList(struct NewScreen * , struct IScreenModePrefs * , struct TagItem * );

static int closeScreenCommon(struct Screen * );

LONG IOpenScreen(struct Screen * sc,
                 struct TagItem * taglist);

int displayOScan(CPTR drecord,
                 ULONG displayID,
                 LONG ostype,
                 struct Rectangle * rect);

int CloseScreen(struct Screen * OScreen);

int AlohaWorkbench(struct MsgPort * wbport);

BOOL CloseWorkBench(void);

int initWBMessage(struct IntuiMessage * imsg,
                  struct MsgPort * replyport,
                  int code);

int ICloseScreen(struct Screen * OScreen);

static int detachScreen(int );

struct Screen * openSysScreen(USHORT type);

