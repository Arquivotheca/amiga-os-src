/* Prototypes for functions defined in tmgr_misc.c */
int CXBRK(void);
int ckkabort(void);
BOOL TMgr_DoSigs(struct TMData *tmd);
BOOL handleInternalMsg(struct TMgr_Globals *t,
                       struct errReport *er,
                       struct TMData *tmd);
void parPrint(STRPTR fmt,
              APTR arg, ...);
void handleReport(struct TMgr_Globals *t,
                  struct errReport *er,
                  struct TMData *tmd);
void updateScreen(struct TMgr_Globals *t,
                  struct errReport *er,
                  struct TMData *TMData,
                  BOOL pf);
struct MsgPort *createClientPort(STRPTR portName);
BOOL TMgr_Init(struct TMData *tmd);
void TMgr_Free(struct TMData *tmd);
BOOL getArgs(struct TMData *TMData,
             int argc,
             UBYTE **argv);
void blueRefreshKludge(struct Window *win, 
                       struct Gadget *gad);

void blankString(STRPTR str, 
                 ULONG n);
