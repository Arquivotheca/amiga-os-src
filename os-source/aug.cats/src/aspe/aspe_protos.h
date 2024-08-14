/* missing system protos and pragmas */
ULONG SetICR(struct Library*,ULONG);
ULONG AbleICR(struct Library*,ULONG);
struct Interrupt *AddICRVector(struct Library*,ULONG,struct Interrupt*);
void RemICRVector(struct Library*,ULONG,struct Interrupt*);

UBYTE *GetMiscResource(LONG,UBYTE *);
void FreeMiscResource(LONG);
#pragma libcall MiscBase GetMiscResource 6 9002
#pragma libcall MiscBase FreeMiscResource c 001

/* in aspe.c */
void set_baud(UWORD);
char* init_data(LONG);

/* in cia.c */
void End_cia(void);
BOOL Begin_cia(void);
void Set_cia(UWORD micros);
void Stop_cia(void);
void Start_cia(void);

/* in stop_watch.c */
struct timerequest  *Init_Timer  (VOID);
void Timer_Start (struct timerequest *);
void Timer_Stop  (struct timerequest *);
void Free_Timer  (struct timerequest *);

extern VOID RBFHand(VOID);            /* assembly interrupt code */
extern VOID trans(char*,LONG);        /* MIDI out assembly subroutine */
extern void argparse(char *);         /* command line argument parser */
extern void help(void);               /* aspe online help text */
