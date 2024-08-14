/* Prototypes for functions defined in
ih.c
 */

struct InputEvent * IntuitionHandler(struct InputEvent * ie);

int Intuition(struct InputEvent * ie);

static int runIntuition(struct InputEvent * );

int activeEvent(int ieclass,
                int code);

int windowEvent(struct Window * sendwindow,
                ULONG ieclass,
                ULONG iecode);

int broadcastIEvent(ULONG ieclass);

int sendIDCMP(ULONG ieclass,
              ULONG code,
              struct InputToken * it,
              struct Window * window,
              APTR iaddress);

int PutMsgSafely(struct MsgPort * port,
                 struct IntuiMessage * imsg);

