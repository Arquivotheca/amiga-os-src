
struct V {

    struct  ExecBase        *ExecBase;
    struct  CDFSBase        *CDFSBase;
    struct  PlayerPrefsBase *PlayerPrefsBase;
    struct  ExpansionBase   *ExpansionBase;
    struct  GfxBase         *GfxBase;
    struct  DosBase         *DOSBase;
    struct  DeBoxBase       *DeBoxBase;
    struct  IntuitionBase   *IntuitionBase;
    struct  Library         *CardResource;
    struct  Library         *GameBase;
    struct  Library         *CDUIBase;
    struct  Library         *LowLevelBase;
    struct  Library         *NoFastMemBase;

    struct  Interrupt      IntServer;
    struct  MsgPort        IOPort;
    struct  IOStdReq       IOReq;
    struct  IOStdReq       CDReq;
    struct  IOExtTD        SReq;                /* for sniffing boot blocks */
    struct  CardHandle    *CardHandle;

    ULONG   CardPolled;
    ULONG   TimeCount;                          /* vertical blank counter   */
    LONG    CDChgCnt;                           /* CD dev change counter    */
    ULONG   CardChgCnt;                         /* CC dev change counter    */
    LONG    TDChgCnt[4];

    UWORD   CDReady;                            /* CE is ready              */  
    UWORD   TDReady;                            /* Track disk is ready      */
    UWORD   KeyPress;                           /* Some key has been pressed*/
    UWORD   Animating;                          /* Animation is running     */
    WORD    BootPri;                            /* BootList priority        */
    WORD    AudioBoot;                          /* Audio disk has priority  */

    char    vstorage[8];                        /* storage for version string */
    };


