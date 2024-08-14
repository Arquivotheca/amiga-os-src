
/* Prototypes for functions defined libinit.c */
ULONG __saveds __stdargs __asm _LibBeginIO(register __a1 struct IOSana2Req *,
                                 register __a6 struct rs485Device *);
ULONG __saveds __stdargs __asm _LibAbortIO(register __a1 struct IOSana2Req *,
                                 register __a6 struct rs485Device *);
ULONG __saveds __stdargs _LibNull(void);

/* Prototypes for functions defined in libinit.c */
ULONG __saveds __stdargs __asm  _LibInit(register __a0 APTR,
                              register __d0 struct rs485Device *);
LONG __saveds __stdargs __asm _LibOpen(register __a6 struct rs485Device *,
                             register __a1 struct IOSana2Req *,
                             register __d0 long,
                             register __d1 long);
ULONG __saveds __stdargs __asm _LibClose(register __a6 struct rs485Device * libbase,
                               register __a1 struct IOSana2Req * iob);
ULONG __saveds __stdargs __asm _LibExpunge(register __a6 struct rs485Device * libbase);

/* Prototypes for functions defined in config.c */
int S2_configinterface(struct IOSana2Req *);

/* Prototypes for functions defined in devicequery.c */
int S2_devicequery(struct IOSana2Req *);

/* Prototypes for functions defined in board.c */
struct rs485Unit *newunit(void);
void freeunit(struct rs485Unit *);
int getboards(void);
int initboard(struct rs485Unit *);
void shutupboard(struct rs485Unit *);

/* Prototypes for functions defined in devinit.c */
void __saveds rs485Open(struct rs485Device *,
               register struct IOSana2Req *, long unit, long flags);
void __saveds rs485Close(struct rs485Device *,
                register struct IOSana2Req *);
void __saveds rs485Expunge(struct rs485Device *);

/* Prototypes for functions defined in event.c */
int S2_onevent(struct IOSana2Req *);
void wakeup(struct AmigaUnit *, ULONG);

/* Prototypes for functions defined in getstation.c */
int S2_getstationaddress(struct IOSana2Req *);
void daddr(struct rs485Unit *);

/* Prototypes for functions defined in interrupts.c */
int interrupts_on(struct rs485Unit *);
void interrupts_off(struct rs485Unit *);
VOID __saveds __stdargs __asm interruptC(register __a1 struct rs485Unit *);

/* Prototypes for functions defined in offline.c */
int S2_offline(struct IOSana2Req *);
int S2_online(struct IOSana2Req *);
void empty_q(struct List *, UBYTE, ULONG);
void resetstats(struct rs485Unit *);

/* Prototypes for functions defined in rdwr.c */
int CMD_rdwr(struct IOSana2Req *);

/* Prototypes for functions defined in checktype.c */
/*int checktype(struct IOSana2Req *);*/

/* Prototypes for functions defined in stat.c */
int S2_getspecialstats(struct IOSana2Req *);
int S2_getglobalstats(struct IOSana2Req *);

/* Prototypes for functions defined in task.c */
void sigtask(struct rs485Unit *);
int starttask(struct rs485Unit *);
void stoptask(struct rs485Unit *);
void __stdargs __saveds listener(void);

/* Prototypes for functions defined in type.c */
int S2_tracked(struct IOSana2Req *);
struct TrackedType *findtracked(struct rs485Unit *, ULONG);
void free_alltracked(struct rs485Unit *);

/* Prototypes for functions defined in startxmit.c */
int startxmit(struct rs485Unit *);

/* Prototypes for functions defined in recv.c */
void recv(struct rs485Unit *, int);

/* Prototypes for functions defined in getlibbase.c */
void __stdargs *getlibbase(void);

/* prototypes for functions defined in stub.a */
void taskent(void);
void interrupt(void);
void __asm copyin(register __a0 UBYTE *datareg, register __a1 UBYTE *buf,
		register __d0 unsigned int cnt);
void __asm copyout(register __a0 UBYTE *buf, register __a1 UBYTE *datareg,
		register __d0 unsigned int cnt);

