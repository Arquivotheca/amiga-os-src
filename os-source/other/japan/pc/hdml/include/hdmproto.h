unsigned short  cvttext(
#ifdef COMPLETE_PROTOTYPE
unsigned char  mode,struct  tagCB *cb
#endif /* COMPLETE_PROTOTYPE */
);
unsigned short  movphr(
#ifdef COMPLETE_PROTOTYPE
unsigned char  dir,struct  tagCB *cb
#endif /* COMPLETE_PROTOTYPE */
);
unsigned short  crrphr(
#ifdef COMPLETE_PROTOTYPE
unsigned char  dir,struct  tagCB *cb
#endif /* COMPLETE_PROTOTYPE */
);
unsigned short  cllreq(
#ifdef COMPLETE_PROTOTYPE
struct  tagCB *cb
#endif /* COMPLETE_PROTOTYPE */
);
int  egcupdate(
#ifdef COMPLETE_PROTOTYPE
unsigned char  *scr,unsigned short  scrl,unsigned char  *org,unsigned short  orgl,unsigned short  oelen,unsigned short  nelen,unsigned short  *ele,struct  tagCB *cb
#endif /* COMPLETE_PROTOTYPE */
);
unsigned short  hdmkey(
#ifdef COMPLETE_PROTOTYPE
unsigned char  *chp,struct  tagCB *cb
#endif /* COMPLETE_PROTOTYPE */
);
unsigned short  rvcvt(
#ifdef COMPLETE_PROTOTYPE
unsigned char  dir,struct  tagCB *cb
#endif /* COMPLETE_PROTOTYPE */
);
unsigned short  hdmdel(
#ifdef COMPLETE_PROTOTYPE
unsigned char  dir,struct  tagCB *cb
#endif /* COMPLETE_PROTOTYPE */
);
unsigned short  movchar(
#ifdef COMPLETE_PROTOTYPE
unsigned char  dir,struct  tagCB *cb
#endif /* COMPLETE_PROTOTYPE */
);
unsigned short  ntojis(
#ifdef COMPLETE_PROTOTYPE
unsigned char  *asrc,unsigned short  spt,unsigned short  ept,unsigned char  *adst,struct  tagCB *cb
#endif /* COMPLETE_PROTOTYPE */
);
short  hdmerr(
#ifdef COMPLETE_PROTOTYPE
short  error
#endif /* COMPLETE_PROTOTYPE */
);
unsigned short
	hdmlclear(
#ifdef COMPLETE_PROTOTYPE

			  struct  tagCB *cb
#ifdef UNIX			  
			  ,struct  tagEGB_HDML *egb_hdml
#endif
			  
#endif /* COMPLETE_PROTOTYPE */
);
unsigned short  hdmpget(
#ifdef COMPLETE_PROTOTYPE
unsigned short  *chl,unsigned char  *chp,struct  tagCB *cb
#endif /* COMPLETE_PROTOTYPE */
);
unsigned short  hdmpput(
#ifdef COMPLETE_PROTOTYPE
unsigned short  chl,unsigned char  *chp,struct  tagCB *cb
#endif /* COMPLETE_PROTOTYPE */
);
unsigned short  cdcvt(
#ifdef COMPLETE_PROTOTYPE
unsigned char  mode,struct  tagCB *cb
#endif /* COMPLETE_PROTOTYPE */
);
unsigned short  geteleinf(
#ifdef COMPLETE_PROTOTYPE
unsigned char  mode,struct  tagCB *cb
#endif /* COMPLETE_PROTOTYPE */
);
unsigned short  zenncmp(
#ifdef COMPLETE_PROTOTYPE
unsigned char  *aptr,unsigned short  prest
#endif /* COMPLETE_PROTOTYPE */
);
unsigned short  hanncmp(
#ifdef COMPLETE_PROTOTYPE
unsigned short  code
#endif /* COMPLETE_PROTOTYPE */
);
unsigned short  readhom(
#ifdef COMPLETE_PROTOTYPE
struct  tagCB *cb
#endif /* COMPLETE_PROTOTYPE */
);
unsigned short  movhom(
#ifdef COMPLETE_PROTOTYPE
unsigned char  dir,struct  tagCB *cb
#endif /* COMPLETE_PROTOTYPE */
);
unsigned short  fixhom(
#ifdef COMPLETE_PROTOTYPE
unsigned char  hnbr,struct  tagCB *cb
#endif /* COMPLETE_PROTOTYPE */
);
int  agehom(
#ifdef COMPLETE_PROTOTYPE
struct  tagCB *cb
#endif /* COMPLETE_PROTOTYPE */
);
unsigned short  canhom(
#ifdef COMPLETE_PROTOTYPE
struct  tagCB *cb
#endif /* COMPLETE_PROTOTYPE */
);
unsigned short  curhomchk(
#ifdef COMPLETE_PROTOTYPE
struct  tagCB *cb
#endif /* COMPLETE_PROTOTYPE */
);
unsigned short
	hdml(
#ifdef COMPLETE_PROTOTYPE

		 struct  tagCB *cb
#ifdef UNIX			  
			  ,struct  tagEGB_HDML *egb_hdml
#endif
		 
#endif /* COMPLETE_PROTOTYPE */
);
int  egcinst(
#ifdef COMPLETE_PROTOTYPE
unsigned short  iop,unsigned char  *chp,unsigned short  cn,struct  tagCB *cb
#endif /* COMPLETE_PROTOTYPE */
);
int  egcdelt(
#ifdef COMPLETE_PROTOTYPE
unsigned short  iop,unsigned short  cn,struct  tagCB *cb
#endif /* COMPLETE_PROTOTYPE */
);
unsigned short  egcpos(
#ifdef COMPLETE_PROTOTYPE
unsigned char  dir,unsigned char  *str,unsigned short  iop
#endif /* COMPLETE_PROTOTYPE */
);
unsigned short  egcsel(
#ifdef COMPLETE_PROTOTYPE
struct  tagCB *cb
#endif /* COMPLETE_PROTOTYPE */
);
unsigned short  egcsbl(
#ifdef COMPLETE_PROTOTYPE
struct  tagCB *cb
#endif /* COMPLETE_PROTOTYPE */
);
unsigned short  egcoel(
#ifdef COMPLETE_PROTOTYPE
struct  tagCB *cb
#endif /* COMPLETE_PROTOTYPE */
);
unsigned short  egcobl(
#ifdef COMPLETE_PROTOTYPE
struct  tagCB *cb
#endif /* COMPLETE_PROTOTYPE */
);

unsigned short zenhanchk(
#ifdef COMPLETE_PROTOTYPE
unsigned char * code
#endif
);
unsigned short addkana(
#ifdef COMPLETE_PROTOTYPE
struct  tagCB *cb
#endif /* COMPLETE_PROTOTYPE */
);

