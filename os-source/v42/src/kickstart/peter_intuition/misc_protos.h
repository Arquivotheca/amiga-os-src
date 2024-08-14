/* Prototypes for functions defined in
misc.c
 */

CPTR AllocRemember(struct Remember ** remember,
                   ULONG size,
                   ULONG flags);

int FreeRemember(struct Remember ** remember,
                 BOOL reallyforget);

struct Layer * getGimmeLayer(struct Window * w);

int cloneRP(struct RastPort * clonerp,
            struct RastPort * rp);

static int cloneRPFont(struct RastPort * , struct RastPort * );

int BlastPatternBox(struct RastPort * rp,
                    struct IBox * box,
                    USHORT * pat,
                    int size,
                    UBYTE pena,
                    UBYTE penb,
                    UBYTE mode);

int BlastPattern(struct RastPort * RP,
                 int left,
                 int top,
                 int right,
                 int bottom,
                 USHORT * pattern,
                 int size,
                 int pena,

int penb,
                 int mode);

int drawBox(struct RastPort * rp,
            int x,
            int y,
            int width,
            int height,
            int apen,
            int mode,
            int hthick,
            int vthick);

int xorbox(struct RastPort * rp,
           int left,
           int top,
           int width,
           int height);

int xorboxmask(struct RastPort * rp,
               int left,
               int top,
               int width,
               int height);

int outbox(struct RastPort * rp,
           int left,
           int top,
           int width,
           int height);

int drawBlock(struct RastPort * rp,
              int left,
              int up,
              int right,
              int down,
              int apen);

int drawImageGrunt(struct RastPort * rport,
                   struct Image * image,
                   int xoffset,
                   int yoffset,
                   int minterm);

int DrawBorder(struct RastPort * rp,
               struct Border * border,
               int left,
               int top);

int PrintIText(struct RastPort * rp,
               struct IntuiText * itext,
               int left,
               int top);

int printIText(struct RastPort * rp,
               struct IntuiText * itext,
               int left,
               int top,
               int use_it_pens_and_mode);

struct MenuItem * ItemAddress(struct Menu * menustrip,
                              USHORT menunum);

BOOL DoubleClick(ULONG startsec,
                 ULONG startmicro,
                 ULONG endsec,
                 ULONG endmicro);

int CurrentTime(ULONG * secs,
                ULONG * micros);

struct View * ViewAddress(void);

int IDisplayBeep(struct Screen * screen);

static ULONG beepComplement(ULONG );

static int beepGrunt(struct Screen * );

struct RastPort * obtainRP(struct RastPort * clonee,
                           struct Layer * layer);

int ReleaseGIRPort(struct RastPort * rp);

int freeRP(struct RastPort * rp);

struct RastPort * ObtainGIRPort(struct GadgetInfo * gi);

