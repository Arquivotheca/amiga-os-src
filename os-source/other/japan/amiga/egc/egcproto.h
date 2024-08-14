/*
 * $Id: egcproto.h,v 3.4.1.1 91/07/15 13:34:28 katogi GM Locker: katogi $
 */
#include "egcldpro.h"

  unsigned short
#ifdef  DOS_EGBRIDGE
_pascal
#endif
 cvtatoj(
#ifdef P_TYPE
unsigned char mode,unsigned char *ascs,unsigned short ascl,unsigned char *jiss
#endif /* P_TYPE */
);
  unsigned short
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  asctojis(
#ifdef P_TYPE
unsigned char emode,unsigned char scode
#endif /* P_TYPE */
);
  unsigned char
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  dhcheck(
#ifdef P_TYPE
unsigned char *p,unsigned char s
#endif /* P_TYPE */
);
  unsigned short
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  jistoasc(
#ifdef P_TYPE
unsigned char *jiss,unsigned char *jise,unsigned char *ascs
#endif /* P_TYPE */
);
  unsigned char
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  jacnvtr(
#ifdef P_TYPE
unsigned short scode,unsigned char *attrp
#endif /* P_TYPE */
);
  unsigned short
  sjcnvtr(
#ifdef P_TYPE
unsigned short scode
#endif /* P_TYPE */
);
  unsigned short
  jscnvtr(
#ifdef P_TYPE
unsigned short jcode
#endif /* P_TYPE */
);
  unsigned char
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  numtokansu(
#ifdef P_TYPE
unsigned char mode,unsigned char *src,unsigned short scl,unsigned char *dst,unsigned short *dsl
#endif /* P_TYPE */
);
  unsigned char
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  kansuji(
#ifdef P_TYPE
unsigned char cmode,unsigned char number,unsigned char *jisarea
#endif /* P_TYPE */
);
  unsigned char
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  findpwrd(
#ifdef P_TYPE
unsigned char *yomip,unsigned char yomil,struct jr *wordtbl,unsigned char opt
#endif /* P_TYPE */
);
  unsigned char
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  findfwrd(
#ifdef P_TYPE
unsigned char *yomip,unsigned char yomil,struct fr *fwrdtbl
#endif /* P_TYPE */
);
  unsigned char
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  findswrd(
#ifdef P_TYPE
unsigned char *yomip,unsigned char yomil,unsigned char prerow,struct jr *swordtbl
#endif /* P_TYPE */
);
  int
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  findkwrd(
#ifdef P_TYPE
unsigned char *yomip,unsigned char yomil,unsigned char *rowtbl
#endif /* P_TYPE */
);
  int
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  findgwrd(
#ifdef P_TYPE
unsigned char hinshi,unsigned char *yomip,unsigned char yomil,unsigned char *rowtbl,unsigned char *lentbl
#endif /* P_TYPE */
);
  int
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  clconnect(
#ifdef P_TYPE
unsigned char row,unsigned char col
#endif /* P_TYPE */
);
  int
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  terminate(
#ifdef P_TYPE
unsigned char row
#endif /* P_TYPE */
);
  unsigned char
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  readswrd(
#ifdef P_TYPE
unsigned short off,unsigned char *buf
#endif /* P_TYPE */
);
  int
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  lookslt(
#ifdef P_TYPE
short idno,unsigned short segno,unsigned short mrecn,unsigned short urecn
#endif /* P_TYPE */
);
  void
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  readslt(
#ifdef P_TYPE
short idno,unsigned short seg,unsigned short mrec,unsigned short urec
#endif /* P_TYPE */
);
  void
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  DimSet(
#ifdef P_TYPE
struct ct *cltype, char lefon, unsigned char qclfrm
#endif /* P_TYPE */
);
char
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  SelectSeg(
#ifdef P_TYPE
struct ct *cf,unsigned char clq
#endif /* P_TYPE */
);
  unsigned short
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  LearnSkip(
#ifdef P_TYPE
short idno,unsigned short *seqno
#endif /* P_TYPE */
);
  struct ct *
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  SetCLearn(
#ifdef P_TYPE
struct ct *cf1,struct ct *cf2,unsigned char *newcl
#endif /* P_TYPE */
);
  unsigned short
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  readdwrd(
#ifdef P_TYPE
short idno,unsigned char yomil,unsigned char *yomi,unsigned char *kanjil,unsigned char *kanji,unsigned char *hbit, unsigned char *selected
#endif /* P_TYPE */
);
int
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  ageformula(
#ifdef  P_TYPE
void
#endif
);
  void
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  agedict(
#ifdef P_TYPE
struct wi *wordid
#endif /* P_TYPE */
);
  void
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  agedict2(
#ifdef P_TYPE
struct wi *wordid,unsigned short allnbr,unsigned short setnbr
#endif /* P_TYPE */
);
  unsigned char
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  finddict(
#ifdef P_TYPE
unsigned char *yomi,unsigned char yomilen
#endif /* P_TYPE */
);
unsigned char*
#ifdef  DOS_EGBRIDGE
_pascal
#endif
readdict(
#ifdef P_TYPE
unsigned char seqno,unsigned char *yomip,unsigned char yomil,unsigned char *kanjip,unsigned char *kanjil
#endif /* P_TYPE */
);
unsigned char*
#ifdef  DOS_EGBRIDGE
_pascal
#endif
readdict2(
#ifdef P_TYPE
unsigned char seqno,unsigned char *yomip,unsigned char yomil,unsigned char *kanjip,unsigned char *kanjil,unsigned char *hinshi
#endif /* P_TYPE */
);
  short
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  deletedict(
#ifdef P_TYPE
unsigned char *yomi,unsigned char yomilen,unsigned char *kanji,unsigned char kanjilen
#endif /* P_TYPE */
);
  short
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  adddict(
#ifdef P_TYPE
unsigned char *yomi,unsigned char yomilen,unsigned char *kanji,unsigned char kanjilen,unsigned char *hbitstr
#endif /* P_TYPE */
);
  int
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  dellearn(
#ifdef P_TYPE
unsigned short delsegn
#endif /* P_TYPE */
);
  void
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  deluslt(
#ifdef P_TYPE
unsigned short dbsegn
#endif /* P_TYPE */
);
  void
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  putwoseg(
#ifdef P_TYPE
void 
#endif /* P_TYPE */
);
  void
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  makemidashi(
#ifdef P_TYPE
unsigned char *krecp,unsigned char *yomi,unsigned char yomilen
#endif /* P_TYPE */
);
  unsigned char *
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  findkanji(
#ifdef P_TYPE
unsigned char *krecp,unsigned char yomilen,unsigned char *yomi,unsigned char kanjilen,unsigned char *kanji
#endif /* P_TYPE */
);
  void
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  delkanji(
#ifdef P_TYPE
unsigned char *krecp,unsigned char *kelmp
#endif /* P_TYPE */
);
  unsigned char
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  makekrec(
#ifdef P_TYPE
unsigned char kanjilen,unsigned char *kanji,unsigned char *hbitstr,unsigned char *krec
#endif /* P_TYPE */
);
  unsigned char *
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  readpwrd(
#ifdef P_TYPE
unsigned char *yrec,int *yll,int *ylr
#endif /* P_TYPE */
);
  unsigned char *
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  readplh(
#ifdef P_TYPE
unsigned char *yrec
#endif /* P_TYPE */
);
  unsigned char
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  makelrec(
#ifdef P_TYPE
unsigned char lnyomil,unsigned char yomilen,unsigned char *yomi,unsigned char *hbitstr,unsigned char *lrec
#endif /* P_TYPE */
);
  unsigned char *
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  getdsrec(
#ifdef P_TYPE
short idno,unsigned short segno,unsigned short recno
#endif /* P_TYPE */
);
  unsigned char *
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  getdsorec(
#ifdef P_TYPE
unsigned short segno,unsigned short recno
#endif /* P_TYPE */
);
  unsigned char *
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  readjwrd(
#ifdef P_TYPE
unsigned char *krec,unsigned char yomil,unsigned char *yomi,unsigned char *kanjil,unsigned char *kanji,unsigned char *hbitstr
#endif /* P_TYPE */
);
  unsigned char *
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  skipkanji(
#ifdef P_TYPE
unsigned char kanjiq,unsigned char *kanjip
#endif /* P_TYPE */
);
  unsigned char 
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  ecdhinshi(
#ifdef P_TYPE
unsigned char *hbitstr
#endif /* P_TYPE */
);
  void 
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  cleanheap(
#ifdef P_TYPE
struct nt *treetbl
#endif /* P_TYPE */
);
  struct nt *
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  newtreept(
#ifdef P_TYPE
unsigned short memsize
#endif /* P_TYPE */
);
  int findpath(
#ifdef P_TYPE
unsigned char *srcpath,unsigned char *dstpath,int dictmode,unsigned char *dictdir,unsigned char *dictpath,unsigned char *dictenv
#endif /* P_TYPE */
);
  void closedict(
#ifdef P_TYPE
void 
#endif /* P_TYPE */
);
  int initdict(
#ifdef P_TYPE
void 
#endif /* P_TYPE */
);
  int opendict(
#ifdef P_TYPE
struct udict_info *pdarry
#endif /* P_TYPE */
);
  int openmseg(
#ifdef P_TYPE
short idno,struct udict_info *pdarry
#endif /* P_TYPE */
);
  int readmafp(
#ifdef P_TYPE
struct udict_info *pdarry
#endif /* P_TYPE */
);
  int readmads(
#ifdef P_TYPE
struct udict_info *pdarry
#endif /* P_TYPE */
);
  int readmaidx(
#ifdef P_TYPE
struct udict_info *pdarry
#endif /* P_TYPE */
);
  int readmapl(
#ifdef P_TYPE
struct udict_info *pdarry
#endif /* P_TYPE */
);
  int openuseg(
#ifdef P_TYPE
short idno,struct udict_info *pdarry
#endif /* P_TYPE */
);
  int readusfp(
#ifdef P_TYPE
struct udict_info *pdarry
#endif /* P_TYPE */
);
  int readusidx(
#ifdef P_TYPE
struct hd *hdrp,short idno
#endif /* P_TYPE */
);
  int readusdso(
#ifdef P_TYPE
struct hd *hdrp
#endif /* P_TYPE */
);
  int readusslt(
#ifdef P_TYPE
struct hd *hdrp,short idno
#endif /* P_TYPE */
);
  int readusplo(
#ifdef P_TYPE
struct hd *hdrp,short idno
#endif /* P_TYPE */
);
  int bfrinit(
#ifdef P_TYPE
struct udict_info *pdarry
#endif /* P_TYPE */
);
  int expopen(
#ifdef P_TYPE
short idno
#endif /* P_TYPE */
);
  void closemseg(
#ifdef P_TYPE
short idno,struct udict_info *pdarry
#endif /* P_TYPE */
);
  void closeuseg(
#ifdef P_TYPE
short idno,struct udict_info *pdarry
#endif /* P_TYPE */
);
  /*
  unsigned char *getgseg(
  */
  int   getgseg(
#ifdef P_TYPE
struct udict_info *pdarry
#endif /* P_TYPE */
);
  unsigned char *
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  getlseg(
#ifdef P_TYPE
short idno,unsigned short dsp,unsigned short len
#endif /* P_TYPE */
);
  unsigned char *
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  getwseg(
#ifdef P_TYPE
short idno,unsigned short segno
#endif /* P_TYPE */
);
  int
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  getuseg(
#ifdef P_TYPE
short idno,unsigned char *buf,unsigned short cnt,unsigned short blkno
#endif /* P_TYPE */
);
  int 
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  putuseg(
#ifdef P_TYPE
short idno,unsigned char *buf,unsigned short cnt,unsigned short blkno
#endif /* P_TYPE */
);
  int is_main_opened(
#ifdef P_TYPE
void 
#endif /* P_TYPE */
);
  unsigned short 
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  kanatokanji(
#ifdef P_TYPE
unsigned short kanalen,unsigned char *kana,unsigned short *kanjilen,unsigned char *kanji,unsigned char mod
#endif /* P_TYPE */
);
  int
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  findclause(
#ifdef P_TYPE
unsigned char yomilen,unsigned char *yomi
#endif /* P_TYPE */
);
  int
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  readclause(
#ifdef P_TYPE
unsigned char *clen,unsigned char *clause,struct wi *wordid,int *tankan
#endif /* P_TYPE */
);
  struct nt *
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  newjtree(
#ifdef P_TYPE
unsigned char stpt
#endif /* P_TYPE */
);
  void 
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  makeftree(
#ifdef P_TYPE
struct nt *tree,unsigned char trp,unsigned char level
#endif /* P_TYPE */
);
  struct nt *
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  wakachi(
#ifdef P_TYPE
void 
#endif /* P_TYPE */
);
  void
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  makenode(
#ifdef P_TYPE
struct nt *tree
#endif /* P_TYPE */
);
  void
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  setnode(
#ifdef P_TYPE
void 
#endif /* P_TYPE */
);
  int 
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  makeptbl(
#ifdef P_TYPE
struct nt *jtree
#endif /* P_TYPE */
);
  void 
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  makefclause(
#ifdef P_TYPE
struct nt *ftree,unsigned char level,unsigned char phlength
#endif /* P_TYPE */
);
  void 
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  setclause(
#ifdef P_TYPE
struct nd *node,unsigned char level,unsigned char length
#endif /* P_TYPE */
);
  void 
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  priority(
#ifdef P_TYPE
struct cl *cl
#endif /* P_TYPE */
);
  void 
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  getplength(
#ifdef P_TYPE
struct nt *tree,unsigned char blength
#endif /* P_TYPE */
);
  struct cl *
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  selectclause(
#ifdef P_TYPE
void 
#endif /* P_TYPE */
);
  int 
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  cvtclause(
#ifdef P_TYPE
struct cl *clause,struct nt *jtree
#endif /* P_TYPE */
);
  int 
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  nodecnt(
#ifdef P_TYPE
unsigned char row,struct nd *node
#endif /* P_TYPE */
);
  unsigned char 
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  clend(
#ifdef P_TYPE
unsigned char row,unsigned char *wordend
#endif /* P_TYPE */
);
unsigned char
#ifdef  DOS_EGBRIDGE
_pascal
#endif
codecheck(
#ifdef P_TYPE
unsigned char type,unsigned char *scode
#endif /* P_TYPE */
);
int
#ifdef  DOS_EGBRIDGE
_pascal
#endif
strcheck(
#ifdef P_TYPE
unsigned char *strp,unsigned short slen
#endif /* P_TYPE */
);
unsigned char
#ifdef  DOS_EGBRIDGE
_pascal
#endif
ctrlchk(
#ifdef P_TYPE
unsigned char *strp,unsigned short slen
#endif /* P_TYPE */
);
int
#ifdef  DOS_EGBRIDGE
_pascal
#endif
NtoKCheck(
#ifdef P_TYPE
unsigned char*,unsigned short,unsigned short*
#endif /* P_TYPE */
);
int
#ifdef  DOS_EGBRIDGE
_pascal
#endif
YStrChk(
#ifdef P_TYPE
unsigned char*,unsigned short
#endif /* P_TYPE */
);
unsigned short
#ifdef  DOS_EGBRIDGE
_pascal
#endif
YTypeChk(
#ifdef P_TYPE
unsigned char*
#endif /* P_TYPE */
);
int CompStr(
#ifdef P_TYPE
unsigned char *src,unsigned short srcl,unsigned char *dst,unsigned short dstl
#endif /* P_TYPE */
);
static  void creatfree(
#ifdef P_TYPE
struct tagDICM *dp
#endif /* P_TYPE */
);
  void MakeGIndex(
#ifdef P_TYPE
unsigned char *indexblk,unsigned short msegq
#endif /* P_TYPE */
);
  void MakeUIndex(
#ifdef P_TYPE
struct hd *puhead,unsigned char *indexblk,unsigned short msegq
#endif /* P_TYPE */
);
  void MakeHeader(
#ifdef P_TYPE
struct hd *puhead,unsigned char sltvalue,unsigned short plosize,unsigned short segval
#endif /* P_TYPE */
);
  void
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  initmem(
#ifdef P_TYPE
void 
#endif /* P_TYPE */
);
  short
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  memncmp(
#ifdef P_TYPE
unsigned char *s,unsigned char *t,unsigned short n
#endif /* P_TYPE */
);
  void
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  setint(
#ifdef P_TYPE
unsigned char *p,unsigned short n
#endif /* P_TYPE */
);
  unsigned short
#ifdef  DOS_EGBRIDGE
_pascal
#endif
  getint(
#ifdef P_TYPE
unsigned char *p
#endif /* P_TYPE */
);
  unsigned char romaji(
#ifdef P_TYPE
unsigned char cmode,unsigned char *src,unsigned short srcs,unsigned short srce,unsigned char *dst,unsigned short *dstl,unsigned short *totl
#endif /* P_TYPE */
);
  unsigned char romcvt(
#ifdef P_TYPE
unsigned char cmode,unsigned char *sbufptr,unsigned short rmjstart,unsigned short crsrend,unsigned char *rstr,unsigned short *cnvnbr,unsigned short *totalnbr,unsigned short *ncvtl
#endif /* P_TYPE */
);
void    CheckUI341(
#ifdef P_TYPE
struct udict_info *dic
#endif /* P_TYPE */
);
static  int CheckPLOSeg(
#ifdef P_TYPE
struct tagDICR *dic
#endif /* P_TYPE */
);
  int CmpIndex(
#ifdef P_TYPE
struct ie *a_gie,struct ie *b_gie,unsigned short msegq
#endif /* P_TYPE */
);
  void vers_ltoc(
#ifdef P_TYPE
long lv,unsigned char *cv
#endif /* P_TYPE */
);
#ifdef DOS_EGC
   short  bsfopen(
#ifdef P_TYPE
unsigned char *open_path,short type
#endif /* P_TYPE */
);
  int  bsfclose(
#ifdef P_TYPE
short close_fp
#endif /* P_TYPE */
);
  int  bsfseek(
#ifdef P_TYPE
short seek_fp,long seek_len,int seek_pos
#endif /* P_TYPE */
);
  int  bsfread(
#ifdef P_TYPE
unsigned char *read_buf,int read_size,int read_len,short read_fp
#endif /* P_TYPE */
);
  int  bsfwrite(
#ifdef P_TYPE
unsigned char *write_buf,int write_size,int write_len,short write_fp
#endif /* P_TYPE */
);
  long  bsftell(
#ifdef P_TYPE
short    write_fp
#endif
);
unsigned char*
#ifdef  DOS_EGBRIDGE
_pascal
#endif
GetSltp(
#ifdef P_TYPE
struct udict_info* pdarry, unsigned short segno
#endif
);
int
#ifdef  DOS_EGBRIDGE
_pascal
#endif
PutSltp(
#ifdef P_TYPE
struct udict_info* pdarry, unsigned short segno
#endif
);
unsigned char*
#ifdef  DOS_EGBRIDGE
_pascal
#endif
GetPlop(
#ifdef  P_TYPE
struct udict_info* pdarry, unsigned short segno
#endif
);
int
#ifdef  DOS_EGBRIDGE
_pascal
#endif
PutPlop(
#ifdef  P_TYPE
struct udict_info* pdarry, unsigned short segno, unsigned char* label, unsigned char llen
#endif
);

int
#ifdef  DOS_EGBRIDGE
_pascal
#endif
MovePlop(
#ifdef  P_TYPE
struct udict_info*, unsigned short, unsigned short, unsigned short, unsigned short
#endif
);
int
#ifdef  DOS_EGBRIDGE
_pascal
#endif
ReadFromWid(
#ifdef P_TYPE
unsigned char*, unsigned short*, unsigned char*, unsigned short*,
unsigned char*, struct wi*
#endif
);
#endif
void    CashFuncInit(
#ifdef  P_TYPE
void
#endif
);
int
#ifdef  DOS_EGBRIDGE
_pascal
#endif
NumZenChk(
#ifdef  P_TYPE
unsigned char*, unsigned short
#endif
);
int
#ifdef  DOS_EGBRIDGE
_pascal
#endif
egcIOctrl(
#ifdef  P_TYPE
short, unsigned char*, long, int, unsigned char
#endif
);
int Canlearn(
#ifdef  P_TYPE
void
#endif
);
void  egcreconv(
#ifdef  P_TYPE
unsigned char
#endif
);
int
#ifdef  DOS_EGBRIDGE
_pascal
#endif
IsIncluding(
#ifdef  P_TYPE
unsigned char*, unsigned short, unsigned char
#endif
);
unsigned short
#ifdef  DOS_EGBRIDGE
_pascal
#endif
CallDhundle(
#ifdef  P_TYPE
unsigned char
#endif
);
unsigned short
#ifdef  DOS_EGBRIDGE
_pascal
#endif
IsStrAlpha(
#ifdef  P_TYPE
unsigned char*, unsigned short
#endif
);
unsigned short
#ifdef  DOS_EGBRIDGE
_pascal
#endif
IsStrKKana(
#ifdef  P_TYPE
unsigned char*, unsigned short
#endif
);
unsigned short
#ifdef  DOS_EGBRIDGE
_pascal
#endif
IsStrSep(
#ifdef  P_TYPE
unsigned char*, unsigned short
#endif
);
