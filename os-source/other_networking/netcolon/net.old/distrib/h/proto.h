/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1987, 1988 The Software Distillery.  All Rights  */
/* |. o.| || Reserved.	This program may not be distributed without the    */
/* | .	| || permission of the authors: 			   BBS:    */
/* | o	| ||   John Toebes     Doug Walker    Dave Baker		   */
/* |  . |//								   */
/* ======								   */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#if PARANOID
extern int paranoid;
#define BUGP(x) if(paranoid) paranoid=request((GLOBAL)global, REQ_GENERAL, x);
#else
#define BUGP(x)
#endif

#define OBUG(x)
#define OBUGBSTR(x,y)

#if DEBUG
int __stdargs KGetChar U_ARGS((void));
void __stdargs KPutStr U_ARGS((char *));
void __stdargs kprintf U_ARGS((char *, ... ));
#define DBSERIAL ((BPTR)1)

/* Debugging routines */
void myputbstr	U_ARGS((char *, ...));
void myputlstr	U_ARGS((char *, ...));
void myprintf	U_ARGS((char *, ...));
void xwrite	U_ARGS((char *, int));
void xgetcr	U_ARGS((void));
int sprintf	U_ARGS((char *, char *, ...));
void mydump	U_ARGS((char *, int));
BPTR initdebug	U_ARGS((BPTR));
void termdebug	U_ARGS((void));
void maybedebug U_ARGS((void));

#define BUG(a) myprintf a;
#define BUGBSTR(a,b) myputbstr(a,b);
#define BUGLSTR(a,b,c) myputlstr(a,b,c);
#define BUGGETC xgetcr();
#define BUGTERM() termdebug();
#define BUGR(x) request((GLOBAL)global, REQ_GENERAL, x);

#if DEBUG >1
#define BUGDUMP(p,l) mydump(p,l);
#else
#define BUGDUMP(p,l)
#endif

#else

#define BUG(a)
#define BUGBSTR(a,b)
#define BUGLSTR(a,b,c)
#define BUGDUMP(p,l)
#define BUGGETC
#define BUGTERM()
#define BUGR(x)
#endif

extern struct DosLibrary *DOSBase;
#define alloc(a,b) DosAllocMem(a,b)
#define free(p) DosFreeMem(p)

/* membuf.c */
void ResetPacketBuffer U_ARGS((struct RPacket *));
void SetPacketBuffer U_ARGS((struct RPacket *, long));
void ClearPacketBuffer U_ARGS((struct RPacket *));

/* io#?.c */
int PutRPacket	U_ARGS((NGLOBAL, APTR, struct RPacket *));
int GetRPacket	 U_ARGS((NGLOBAL, APTR, struct RPacket *));

/* Subs.c */
struct HandlerPacket *taskwait     U_ARGS((GLOBAL));
void retpkt	  U_ARGS((GLOBAL, struct HandlerPacket *));
char *DosAllocMem U_ARGS((GLOBAL, long));
void DosFreeMem   U_ARGS((char *));
long sendpkt U_ARGS((struct MsgPort *, long, long*, long));
LONG checksum U_ARGS((char *, int));
void CheckRP U_ARGS((struct RPacket *));

/* pause.c */
int Pause U_ARGS((ULONG));

/* net#?.c */
int ReSync	   U_ARGS((GLOBAL, APTR *, char *));

/* timer.c */
int OpenTimer U_ARGS((GLOBAL, struct MsgPort *));
void CloseTimer U_ARGS((GLOBAL));
void PostTimerReq U_ARGS((GLOBAL, int));

/* request.c */
void initreq         U_ARGS((GLOBAL));
int  request	     U_ARGS((GLOBAL, int, char *));
#define REQ_MUST    0
#define REQ_ERROR   1
#define REQ_GENERAL 2

