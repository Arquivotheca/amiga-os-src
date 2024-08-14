/* Prototypes for functions defined in
menu.c
 */

static int menuImage(struct MenuItem * , struct Rectangle * , struct Rectangle * , BOOL );

static int itemExtent(struct Window * , struct MenuItem * , struct Rectangle * );

static int boxer(struct Window * , struct MenuItem * , struct Rectangle * , int , int );

struct Menu * grabMenu(register struct Menu * menu,
                       register USHORT menuNum);

struct MenuItem * grabItem(register struct Menu * menu,
                           register SHORT menuNum);

struct MenuItem * grabSub(register struct MenuItem * item,
                          register SHORT menuNum);

int eraseItem(void);

int eraseSub(void);

static SHORT hitMenu(void);

static USHORT hitItem(void);

static SHORT hitSub(void);

static SHORT hitGrunt(BOOL );

static int highMenu(void);

static int setMenuRPMask(struct RastPort * );

static int highGrunt(struct MenuItem * , int , int , struct Screen * );

int highItem(void);

int highSub(void);

static int highPrimeGrunt(BOOL );

int showMStrip(register struct Window * w);

int removeMStrip(struct Screen * s);

int resetMenu(struct Window * w,
              USHORT mflags,
              USHORT iflags,
              USHORT sflags);

static struct BitMap * allocMenuBitMap(struct Screen * , struct Rectangle * );

int drawItem(USHORT num);

int drawSub(USHORT num);

BOOL doSetMenuStrip(struct Window * window,
                    struct Menu * menu,
                    LONG recalc);

int realSetMenuStrip(struct Window * window,
                     struct Menu * menu,
                     int precalc_sizes);

int ClearMenuStrip(struct Window * window);

BOOL IOnOffMenu(struct Window * window,
                USHORT menuNum,
                BOOL onOrOff);

BOOL OnMenu(struct Window * window,
            UWORD menuNum);

BOOL OffMenu(struct Window * window,
             UWORD menuNum);

int getMenu(void);

static long swapMenuIn(struct Screen * , struct ClipRect * );

static int menuBlit(struct Screen * , struct BitMap * , struct Rectangle * , long );

