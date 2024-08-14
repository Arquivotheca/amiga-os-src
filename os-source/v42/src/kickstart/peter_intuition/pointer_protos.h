/* Prototypes for functions defined in
pointer.c
 */

extern UWORD RomPointer0[16];

extern UWORD RomPointer1[16];

extern UWORD RomBusyPointer0[16];

extern UWORD RomBusyPointer1[16];

int initMousePointer(void);

struct BitMap * setDefaultMousePointer(struct BitMap * bm,
                                       LONG xoffset,
                                       LONG yoffset,
                                       LONG xresn,

LONG yresn,
                                       LONG wordwidth,
                                       LONG type);

int ISetWindowPointer(struct Window * win,
                      struct MousePointer * newMousePointer);

LONG setMousePointer(void);

int updateMousePointer(struct ExtSprite * newSprite);

static int calcSpriteWidth(LONG );

int notePointerKill(struct IntuitionBase * passedIBase);

int reinstatePointer(void);

extern UWORD SpriteFactorTable[3];

int setSpriteSpeed(struct Screen * sc);

static int setSpriteHeight(void);

static struct ExtSprite * MakeSpriteData(struct MousePointer * , ULONG , LONG );

int LIB_SetWindowPointerA(struct Window * win,
                          struct TagItem * taglist);

int SetPointer(struct Window * win,
               USHORT * pointer,
               int height,
               int width,
               int xoffset,
               int yoffset);

extern ULONG pointerPackTable[8];

static ULONG dispatchPointerClass(Class * , Object * , Msg );

Class * initPointerClass(void);

