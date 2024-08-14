/* Prototypes for functions defined in
pubscreen.c
 */

struct Screen * LockPubScreen(UBYTE * name);

int UnlockPubScreen(UBYTE * name,
                    struct Screen * screen);

struct List * LockPubScreenList(void);

int UnlockPubScreenList(void);

UBYTE * NextPubScreen(struct Screen * screen,
                      UBYTE * namebuf);

int SetDefaultPubScreen(UBYTE * name);

int GetDefaultPubScreen(UBYTE * Namebuff);

int SetPubScreenModes(UWORD modes);

int PubScreenStatus(struct Screen * screen,
                    UWORD status);

struct Screen * windowPubScreen(UBYTE * pubname,
                                BOOL fallback);

static struct PubScreenNode * findPubScreen(UBYTE * );

struct Screen * defaultPubScreen(void);

struct PubScreenNode * initPubScreen(unsigned char * pubname,
                                     struct Task * pubtask,
                                     UBYTE pubsig,
                                     LONG * errorptr);

int linkPubScreen(struct Screen * sc,
                  struct PubScreenNode * pubnode);

int freePubScreenNode(struct PubScreenNode * psn);

int bumpPSNVisitor(struct PubScreenNode * psn);

int decPSNVisitor(struct PubScreenNode * psn);

static struct Node * findNameNC(struct List * , STRPTR );

