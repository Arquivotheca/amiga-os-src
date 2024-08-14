/* Prototypes for functions defined in
ezreq.c
 */

int easyRequestArgs(struct Window * rwin,
                    struct EasyStruct * ez,
                    ULONG * idcmp_ptr,
                    UWORD * args);

BOOL autoRequest(struct Window * rwin,
                 struct IntuiText * bodyt,
                 struct IntuiText * goodt,
                 struct IntuiText * badt,
                 ULONG goodif,
                 ULONG badif,
                 WORD width
,
                 WORD height);

int SysReqHandler(struct Window * w,
                  ULONG * idcmp_ptr,
                  int wait_first);

struct Window * buildEasyRequestArgs(struct Window * rwin,
                                     struct EasyStruct * ez,
                                     ULONG iflags,
                                     UWORD * args);

APTR buildSysRequest(struct Window * rwin,
                     struct IntuiText * bodyt,
                     struct IntuiText * retryt,
                     struct IntuiText * cancelt,
                     ULONG iflags,
                     int width,

                    int height);

static struct Window * commonBSR(struct Window * , struct Screen * , UBYTE * , struct IntuiText * , struct Gadget * , Object * , ULONG , struct Remember ** , BOOL );

static int reqTitleLength(struct RastPort * , UBYTE * );

static int addGadImage(struct Gadget * , struct Image * );

static int disposeGadgetList(struct Gadget * );

static int disposeImageList(struct Image * );

static struct Window * reqWindow(struct Window * , struct Screen * , UBYTE * , struct Gadget * , WORD , WORD , ULONG );

int FreeSysRequest(struct Window * window);

static int processFail(BOOL );

