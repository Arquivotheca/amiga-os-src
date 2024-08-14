
extern struct Window *PrefWindow;	/* this programs window */
extern struct MsgPort *MPort;		/* this programs message port */
extern ULONG	MSigs;			/* signal bit from message port */
extern ULONG	FinishedFlag;		/* flag, time to exit the program */
extern UWORD	FinishCode;		/* what to do when finished */
extern BPTR	PrefsFile;		/* file handle for prefs file */
#if 0
extern UBYTE	SerialState;		/* state of serial enable/disable */
#endif
extern UBYTE	ShadowState;
extern UBYTE	MonoState;		/* state of mono enable/disable */
extern UBYTE	ColorState;		/* state of color enable/disable */
extern UBYTE	RamBank;		/* position of 64k RAM bank */
extern int	top_offs;		/* offset from top */

#define PREFSIZE	4	/* size of preferences data */
 
#define DEFSER		0	/* default serial to off */
#define DEFMON		1	/* default mono to on */
#define DEFCOL		0	/* default color to off */
#define DEFRAM		2	/* default RAM to $D0000-$DFFFF */
#define DEFSHAD		0	/* default shadow to on */
