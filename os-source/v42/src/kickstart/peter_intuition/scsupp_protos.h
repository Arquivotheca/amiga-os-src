/* Prototypes for functions defined in
scsupp.c
 */

int forceBarFront(register struct Screen * s);

int forceBarBack(register struct Screen * s);

int forceBarCenter(register struct Screen * s);

extern USHORT SGadgetTypes[2];

int screenGadgets(struct Screen * s);

int ScreenPosition(struct Screen * screen,
                   ULONG flags,
                   LONG x1,
                   LONG y1,
                   LONG x2,
                   LONG y2);

int IPreMoveScreen(struct Screen * screen,
                   struct Rectangle * rect,
                   ULONG flags);

int IMoveScreen(struct Screen * screen,
                struct Point * delta,
                WORD flags);

static int doMoveScreen(struct Screen * , WORD , WORD );

int LIB_ScreenDepth(struct Screen * screen,
                    ULONG flags,
                    struct Screen * reserved);

int IScreenDepth(struct Screen * screen,
                 int whichway);

struct DrawInfo * GetScreenDrawInfo(struct Screen * s);

int FreeScreenDrawInfo(struct Screen * s,
                       struct DrawInfo * dri);

int setScreen(struct Screen * s);

int drawScreenBar(struct RastPort * rp,
                  struct Screen * s,
                  ULONG blockpen);

int refreshScreenBar(struct Screen * sc);

int screenbar(register struct Screen * s);

int ShowTitle(struct Screen * screen,
              BOOL showit);

int IShowTitle(struct Screen * screen,
               BOOL showit);

struct ScreenBuffer * AllocScreenBuffer(struct Screen * sc,
                                        struct BitMap * bm,
                                        int flags);

int ICopyScreenBuffer(struct Screen * sc,
                      struct ScreenBuffer * sb);

void FreeScreenBuffer(struct Screen * sc,
                      struct ScreenBuffer * sb);

LONG ChangeScreenBuffer(struct Screen * sc,
                        struct ScreenBuffer * sb);

int IChangeScreenBuffer(struct Screen * screen,
                        struct ScreenBuffer * sb);

LONG parentOriginY(struct Screen * child,
                   struct Screen * parent);

static LONG parentOriginX(struct Screen * , struct Screen * );

struct Screen * screenFamily(struct Screen * s);

LONG attachScreen(struct Screen * child,
                  struct Screen * parent);

int relinkScreen(struct Screen * screen,
                 int flags);

int delinkScreen(struct Screen * OScreen);

LONG canSlide(struct Screen * s,
              int override);

BOOL IsHires(struct Screen * s);

