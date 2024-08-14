/* Prototypes for functions defined in
windows.c
 */

extern USHORT WGadgetTypes[5];

extern USHORT WGadgets[5];

struct Window * hitUpfront(struct Layer ** layerhit);

struct Screen * hitScreen(void);

struct Window * OpenWindow(struct ExtNewWindow * nw);

int IOpenWindow(struct Window * w,
                struct BitMap * superbmap,
                struct Hook * backfill);

int CloseWindow(struct Window * window);

int closeWindowCommon(struct Window * window,
                      ULONG failure);

void ActivateWindow(struct Window * window);

int addSWGadgets(struct Window * w,
                 int addzoom);

int freeSysGadgets(struct Gadget * g);

int RefreshWindowFrame(struct Window * w);

int drawEmbossedBorder(struct Window * w,
                       int draw_flags);

int SafeRectFill(struct RastPort * rp,
                 WORD xmin,
                 WORD ymin,
                 WORD xmax,
                 WORD ymax);

int embossedFillPen(struct Window * w);

int setWindow(struct Window * w,
              LONG initial);

int LendMenus(struct Window * fromwindow,
              struct Window * towindow);

int SetWindowTitles(struct Window * window,
                    UBYTE * windowtitle,
                    UBYTE * screentitle);

BOOL WindowLimits(struct Window * window,
                  SHORT minwidth,
                  SHORT minheight,
                  USHORT maxwidth,
                  USHORT maxheight);

int MoveWindow(struct Window * window,
               int dx,
               int dy);

int SizeWindow(struct Window * window,
               SHORT dx,
               SHORT dy);

int ChangeWindowBox(struct Window * window,
                    int l,
                    int t,
                    int w,
                    int h);

int WindowToFront(struct Window * window);

int MoveWindowInFrontOf(struct Window * w,
                        struct Window * behindw);

int WindowToBack(struct Window * window);

void ScrollWindowRaster(struct Window * win,
                        LONG dx,
                        LONG dy,
                        LONG xmin,
                        LONG ymin,
                        LONG xmax,
                        LONG ymax);

