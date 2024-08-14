/* Prototypes for functions defined in
idcmp.c
 */

extern ULONG IDCMPclasses[23];

ULONG translateIEtoIDCMP(UBYTE ieclass,
                         UWORD iecode);

int ModifyIDCMP(struct Window * window,
                ULONG flags);

int IModifyIDCMP(struct Window * window,
                 ULONG flags);

int reclaimWBMsg(void);

int reclaimMessages(struct Window * window);

int snoopVerifyReply(void);

static int clearPending(struct Window * , ULONG , int );

static struct IntuiMessage * findFreeMessage(struct Window * );

struct IntuiMessage * initIMsg(ULONG class,
                               ULONG code,
                               struct InputEvent * ie,
                               struct Window * window,
                               struct TabletData * tablet)
;

static int sendAllOne(ULONG , ULONG , ULONG , struct Screen * );

int sendAllAll(ULONG class,
               ULONG sendflag);

