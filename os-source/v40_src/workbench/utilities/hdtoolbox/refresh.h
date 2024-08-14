/* refresh.h */

#define DONTDRAW	0x8000
#define BORDER		0x4000
#define DROPSHADOW	0x2000
#define FANCY		0x1000
#define REFRESH_MASK	0xF000


#define HANDLE_DONE	1
#define HANDLE_CANCEL	2
#define HANDLE_REFRESH	3

extern ULONG colors[4];	/* 0 is BG, 1 is GAD, 2 is BORDER, 3 is DS */
extern struct IntuiText *global_itext;
extern void   (*global_refreshrtn)(struct RastPort *);

extern
int HandleWindow(
		 struct Window *,
		 struct IntuiText *,	/* window itext */
		 struct Gadget *,
		 UBYTE  *,
		 int    (*)(struct Window *,struct IntuiMessage *),
							/* handles GadgetUp  */
		 void   (*)(struct RastPort *),
							/* draws into window */
		 int	(*)(struct Window *)		/* initization rtn   */
		);

extern
int HandleWindowNew(
		struct Window *,
		struct IntuiText *,	/* window itext */
		struct Gadget **,		// new gadget list
		UBYTE  *,
		ULONG,				// IDCMPFlags - v39.10
		int    (*)(struct Window *,struct IntuiMessage *),
						/* handles GadgetUp  */
		void   (*)(struct RastPort *),
						/* draws into window */
		int	(*)(struct Window *),		/* initization rtn   */
		struct	Gadget *(*)(void),	// create newgadget rtn
		struct Gadget **,		// backup gadget list
		struct	Gadget *(*)(void)	// create oldgadget rtn
		);


extern int HandleRequest(
		struct Window *,
		struct Requester *,
		int (*)(struct Window *,struct Requester *,
		struct IntuiMessage *)
		);

extern void Notify		(struct Window *,char *,LONG);

extern int  SureHandler 	(struct Window *,struct Requester *,
			 	struct IntuiMessage *);

extern int  NotifyHandler	 (struct Window *,struct Requester *,
			   	struct IntuiMessage *);

extern int  AskSure		(struct Window *,char **);

extern void LongNotify		(struct Window *,char **);

extern int  InternalAskSure	(struct Window *,char **,int);

extern void do_refresh		(struct Window *,struct IntuiText *,
				void (*)(struct RastPort *));

extern void refresh		(struct Window *,ULONG,ULONG,ULONG,ULONG);

// We are new gadgets stuff.
extern struct Gadget *CreateAllGadgets1	(void);
extern struct Gadget *CreateAllGadgets2	(void);
extern struct Gadget *CreateAllGadgets3	(void);
extern struct Gadget *CreateAllGadgets4	(void);
extern struct Gadget *CreateAllGadgets5	(void);
extern struct Gadget *CreateAllGadgets10(void);
extern struct Gadget *CreateAllGadgets12(void);

extern void do_reposition		(struct IntuiText *);
extern void do_repositionRequester	(struct Request *);

extern void MakePrettyRequestBorder	(UWORD,UWORD);
//
